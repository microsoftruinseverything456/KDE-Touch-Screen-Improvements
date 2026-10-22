/*
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "touchlongpresscore.h"
#include "touchscreenimprovements_debug.h"
#include "virtualinputdevice.h"

#include "inputmethod.h"
#include "main.h"
#include "touch_input.h"
#include "wayland_server.h"
#include "window.h"

#include <QLineF>

#include <linux/input-event-codes.h>

using namespace std::chrono_literals;

// Shorter than typical toolkit long-press delays (~500 ms+), so apps don't react first
static constexpr auto s_holdTime = 450ms;
static constexpr qreal s_slop = 16.0; // logical px
// Lets the app finish the selection before right-clicking on it
static constexpr auto s_contextMenuDelay = 50ms;
// After hiding the virtual keyboard: wait until the window geometry is stable this long...
static constexpr auto s_settleQuiet = 150ms;
// ...but no longer than this
static constexpr auto s_settleMax = 600ms;
// Covers apps enabling text input shortly after an emulated click
static constexpr auto s_emulationGrace = 1500ms;

static std::chrono::microseconds s_emulatedUntil{};

bool TouchLongPressCore::recentlyEmulated()
{
    return VirtualInputDevice::now() < s_emulatedUntil;
}

TouchLongPressCore::TouchLongPressCore()
    // Early in the chain, before screen edges, effects etc. can swallow touches
    : KWin::InputEventFilter(KWin::InputFilterOrder::EisInput)
    , m_device(std::make_unique<VirtualInputDevice>(QStringLiteral("Touch long press pointer"), false, true))
{
    m_holdTimer.setSingleShot(true);
    m_holdTimer.setInterval(s_holdTime);
    connect(&m_holdTimer, &QTimer::timeout, this, &TouchLongPressCore::handleHoldTimeout);
    m_settleTimer.setSingleShot(true);
    m_settleTimer.setInterval(s_settleQuiet);
    connect(&m_settleTimer, &QTimer::timeout, this, &TouchLongPressCore::handleWindowSettled);

    KWin::input()->addInputDevice(m_device.get());
    KWin::input()->installInputEventFilter(this);

    const auto devices = KWin::input()->devices();
    for (auto device : devices) {
        watchDevice(device);
    }
    connect(KWin::input(), &KWin::InputRedirection::deviceAdded, this, &TouchLongPressCore::watchDevice);
}

TouchLongPressCore::~TouchLongPressCore()
{
    if (m_state == State::Selecting) {
        sendButton(BTN_LEFT, KWin::PointerButtonState::Released);
    }
    KWin::input()->removeInputDevice(m_device.get());
}

// KWin stops processing a finger once its touch is cancelled, so follow it here
void TouchLongPressCore::watchDevice(KWin::InputDevice *device)
{
    if (!device->isTouch()) {
        return;
    }
    connect(device, &KWin::InputDevice::touchMotion, this, [this](qint32 id, const QPointF &pos) {
        handleRawTouchMotion(id, pos);
    });
    connect(device, &KWin::InputDevice::touchUp, this, [this](qint32 id) {
        handleRawTouchUp(id);
    });
    connect(device, &KWin::InputDevice::touchCanceled, this, &TouchLongPressCore::handleRawTouchCanceled);
}

void TouchLongPressCore::setRightClickEnabled(bool enabled)
{
    m_rightClickEnabled = enabled;
}

void TouchLongPressCore::setSelectEnabled(bool enabled)
{
    m_selectEnabled = enabled;
}

void TouchLongPressCore::reset()
{
    m_holdTimer.stop();
    m_state = State::Idle;
    m_touchId = -1;
}

bool TouchLongPressCore::touchDown(KWin::TouchDownEvent *event)
{
    if (m_state == State::Pending || m_state == State::HeldPassive) {
        reset(); // multi-finger gesture
        return false;
    }
    if (m_state != State::Idle) {
        return false;
    }
    cancelPendingRightClick();
    if (KWin::waylandServer()->isScreenLocked()) {
        return false;
    }

    // No window: an effect like the overview has the touch. The virtual keyboard has
    // its own long press (accents, key repeat).
    KWin::Window *window = KWin::input()->touch()->focus();
    if (!window || window->isInputMethod()) {
        return false;
    }
    m_allowSelection = m_selectEnabled && window->clientGeometry().contains(event->pos);
    if (!m_rightClickEnabled && !m_allowSelection) {
        return false;
    }

    m_state = State::Pending;
    m_touchId = event->id;
    m_startPos = event->pos;
    m_window = window;
    m_holdTimer.start();
    return false;
}

bool TouchLongPressCore::touchMotion(KWin::TouchMotionEvent *event)
{
    if (event->id != m_touchId || QLineF(m_startPos, event->pos).length() <= s_slop) {
        return false;
    }
    if (m_state == State::Pending) {
        reset(); // scrolling or dragging
    } else if (m_state == State::HeldPassive) {
        // Take the touch over now to select
        m_state = State::Held;
        s_emulatedUntil = VirtualInputDevice::now() + s_emulationGrace;
        KWin::input()->touch()->cancel();
        handleRawTouchMotion(event->id, event->pos);
        return true;
    }
    return false;
}

bool TouchLongPressCore::touchUp(KWin::TouchUpEvent *event)
{
    if ((m_state == State::Pending || m_state == State::HeldPassive) && event->id == m_touchId) {
        reset();
    }
    return false;
}

bool TouchLongPressCore::touchCancel()
{
    // Our own cancels also land here, after the state changed
    if (m_state == State::Pending || m_state == State::HeldPassive) {
        reset();
    }
    return false;
}

void TouchLongPressCore::handleHoldTimeout()
{
    if (m_state != State::Pending) {
        return;
    }
    if (!m_rightClickEnabled) {
        // Nothing happens on lift, so leave the touch to the app unless the finger moves
        m_state = State::HeldPassive;
        return;
    }
    qCDebug(KWIN_TOUCHSCREENIMPROVEMENTS) << "Long press at" << m_startPos;
    m_state = State::Held;
    s_emulatedUntil = VirtualInputDevice::now() + s_emulationGrace;
    KWin::input()->touch()->cancel();
}

void TouchLongPressCore::handleRawTouchMotion(qint32 id, const QPointF &pos)
{
    if (id != m_touchId) {
        return;
    }
    if (m_state == State::Selecting) {
        m_lastPos = pos;
        movePointer(pos);
        return;
    }
    if (m_state != State::Held || QLineF(m_startPos, pos).length() <= s_slop) {
        return;
    }
    if (!m_allowSelection) {
        reset();
        return;
    }

    // Double-click and hold; distinct timestamps so the presses aren't dropped as duplicates
    m_state = State::Selecting;
    m_lastPos = pos;
    movePointer(m_startPos);
    sendButton(BTN_LEFT, KWin::PointerButtonState::Pressed, -2ms);
    sendButton(BTN_LEFT, KWin::PointerButtonState::Released, -1ms);
    sendButton(BTN_LEFT, KWin::PointerButtonState::Pressed);
    movePointer(pos);
}

void TouchLongPressCore::handleRawTouchUp(qint32 id)
{
    if (id != m_touchId) {
        return;
    }
    if (m_state == State::Held) {
        rightClick(m_startPos, 0ms);
    } else if (m_state == State::Selecting) {
        sendButton(BTN_LEFT, KWin::PointerButtonState::Released);
        // On the selection, so the app offers copy etc.
        rightClick(m_lastPos, s_contextMenuDelay);
    } else {
        return;
    }
    reset();
}

void TouchLongPressCore::handleRawTouchCanceled()
{
    if (m_state == State::Selecting) {
        sendButton(BTN_LEFT, KWin::PointerButtonState::Released);
    }
    if (m_state == State::Held || m_state == State::Selecting) {
        reset();
    }
}

void TouchLongPressCore::rightClick(const QPointF &pos, std::chrono::milliseconds delay)
{
    auto inputMethod = KWin::kwinApp()->inputMethod();
    if (!inputMethod->isVisible() || !m_window) {
        QTimer::singleShot(delay, this, [this, pos] {
            sendRightClick(pos);
        });
        return;
    }

    // The menu would take focus from the text field, hiding the keyboard, and KWin's
    // restoring of the window it shrank for the keyboard would close the menu again. So
    // hide the keyboard first and click the same spot once the window has settled.
    cancelPendingRightClick();
    m_pendingWindow = m_window;
    m_pendingOffset = pos - m_window->clientGeometry().topLeft();
    m_settleDeadline.start();
    m_settleTimer.start();
    m_pendingGeometryConnection = connect(m_window, &KWin::Window::frameGeometryChanged, this, [this] {
        if (m_settleDeadline.elapsed() < s_settleMax.count()) {
            m_settleTimer.start();
        }
    });
    inputMethod->hide();
}

void TouchLongPressCore::handleWindowSettled()
{
    disconnect(m_pendingGeometryConnection);
    if (m_pendingWindow) {
        sendRightClick(m_pendingWindow->clientGeometry().topLeft() + m_pendingOffset);
        m_pendingWindow.clear();
    }
}

void TouchLongPressCore::cancelPendingRightClick()
{
    m_settleTimer.stop();
    disconnect(m_pendingGeometryConnection);
    m_pendingWindow.clear();
}

void TouchLongPressCore::sendRightClick(const QPointF &pos)
{
    movePointer(pos);
    sendButton(BTN_RIGHT, KWin::PointerButtonState::Pressed, -1ms);
    sendButton(BTN_RIGHT, KWin::PointerButtonState::Released);
}

void TouchLongPressCore::movePointer(const QPointF &pos)
{
    s_emulatedUntil = VirtualInputDevice::now() + s_emulationGrace;
    Q_EMIT m_device->pointerMotionAbsolute(pos, VirtualInputDevice::now(), m_device.get());
    Q_EMIT m_device->pointerFrame(m_device.get());
}

void TouchLongPressCore::sendButton(quint32 button, KWin::PointerButtonState state, std::chrono::milliseconds timeOffset)
{
    s_emulatedUntil = VirtualInputDevice::now() + s_emulationGrace;
    Q_EMIT m_device->pointerButtonChanged(button, state, VirtualInputDevice::now() + timeOffset, m_device.get());
    Q_EMIT m_device->pointerFrame(m_device.get());
}

#include "moc_touchlongpresscore.cpp"
