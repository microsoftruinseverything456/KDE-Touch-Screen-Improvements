/*
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "taptoshowkeyboardcore.h"
#include "touchlongpresscore.h"
#include "touchscreenimprovements_debug.h"

#include "inputmethod.h"
#include "inputpanelv1window.h"
#include "keyboard_input.h"
#include "main.h"
#include "pointer_input.h"
#include "touch_input.h"
#include "wayland/inputmethod_v1.h"
#include "wayland/seat.h"
#include "wayland/textinput_v2.h"
#include "wayland/textinput_v3.h"
#include "wayland_server.h"

#include <QLineF>
#include <QMetaMethod>

static constexpr qreal s_tapSlop = 16.0; // logical px
static constexpr qint64 s_forceShowGraceMs = 1000;

TapToShowKeyboardCore::TapToShowKeyboardCore()
    // Early in the chain so no other filter swallows touches first; we never consume any
    : KWin::InputEventFilter(KWin::InputFilterOrder::EisInput)
{
    KWin::input()->installInputEventFilter(this);

    auto inputMethod = KWin::kwinApp()->inputMethod();
    connect(inputMethod, &KWin::InputMethod::visibleChanged, this, &TapToShowKeyboardCore::handleVisibleChanged);
    connect(inputMethod, &KWin::InputMethod::activeChanged, this, [this](bool active) {
        if (!active) {
            m_heldBack = false;
        }
    });

    auto seat = KWin::waylandServer()->seat();
    connect(seat, &KWin::SeatInterface::focusedTextInputSurfaceChanged, this, [this] {
        m_heldBack = false;
    });

    // Stop the keyboard before KWin shows it; hiding it afterwards still animates.
    // text-input-v3 clients re-enable text input on every commit, e.g. every frame while scrolling.
    auto t2 = seat->textInputV2();
    auto t3 = seat->textInputV3();
    takeOver(t3, &KWin::TextInputV3Interface::enableRequested, &TapToShowKeyboardCore::handleTextInputV3EnableRequested, "textInputInterfaceV3EnableRequested()");
    takeOver(t3, &KWin::TextInputV3Interface::enabledChanged, &TapToShowKeyboardCore::handleTextInputV3EnabledChanged, "textInputInterfaceV3EnabledChanged()");
    takeOver(t2, &KWin::TextInputV2Interface::requestShowInputPanel, &TapToShowKeyboardCore::handleShowInputPanelRequested, nullptr);
    takeOver(t3, &KWin::TextInputV3Interface::requestShowInputPanel, &TapToShowKeyboardCore::handleShowInputPanelRequested, nullptr);
}

TapToShowKeyboardCore::~TapToShowKeyboardCore()
{
    for (const auto &restore : std::as_const(m_restoreHandlers)) {
        restore();
    }
}

// Replaces InputMethod's handler for signal with slot. kwinSlot is the private slot to
// reconnect when we're destroyed, or null for InputMethod::show().
template<typename Sender, typename Signal, typename Slot>
void TapToShowKeyboardCore::takeOver(Sender *sender, Signal signal, Slot slot, const char *kwinSlot)
{
    auto inputMethod = KWin::kwinApp()->inputMethod();
    // KWin connects these with function pointers to private slots, so drop all InputMethod
    // receivers of the signal (there's only one)
    if (!disconnect(sender, signal, inputMethod, nullptr)) {
        qCWarning(KWIN_TOUCHSCREENIMPROVEMENTS) << "Could not take over" << (kwinSlot ? kwinSlot : "show()");
        return;
    }
    connect(sender, signal, this, slot);

    QPointer<Sender> guardedSender(sender);
    m_restoreHandlers.append([guardedSender, signal, kwinSlot, inputMethod] {
        if (!guardedSender) {
            return;
        }
        if (!kwinSlot) {
            connect(guardedSender, signal, inputMethod, &KWin::InputMethod::show);
        } else if (const int index = inputMethod->metaObject()->indexOfSlot(kwinSlot); index >= 0) {
            connect(guardedSender, QMetaMethod::fromSignal(signal), inputMethod, inputMethod->metaObject()->method(index));
        }
    });
}

KWin::Window *TapToShowKeyboardCore::textInputWindow() const
{
    auto surface = KWin::waylandServer()->seat()->focusedTextInputSurface();
    return surface ? KWin::waylandServer()->findWindow(surface) : nullptr;
}

bool TapToShowKeyboardCore::lastTouchWasTapOn(const KWin::Window *window) const
{
    return m_isTap && window && m_touchWindow == window;
}

TapToShowKeyboardCore::Decision TapToShowKeyboardCore::decide() const
{
    if (!m_touchStart.isEmpty()) {
        return Decision::Defer;
    }
    const QObject *lastInput = KWin::input()->lastInputHandler();
    // Clicks emulated from a long press aren't taps
    if (lastInput == KWin::input()->pointer() && TouchLongPressCore::recentlyEmulated()) {
        return Decision::Block;
    }
    // Apps keep re-enabling text input while typing on a real keyboard
    if (m_suppressAfterPhysicalInput && (lastInput == KWin::input()->keyboard() || lastInput == KWin::input()->pointer())) {
        return Decision::Suppress;
    }
    // Other non-touch input behaves as in KWin
    if (lastInput != KWin::input()->touch()) {
        return Decision::Allow;
    }
    return lastTouchWasTapOn(textInputWindow()) ? Decision::Allow : Decision::Block;
}

void TapToShowKeyboardCore::handleTextInputV3EnableRequested()
{
    if (!KWin::waylandServer()->seat()->textInputV3()->implicitShowPanelRequired()) {
        return;
    }
    if (const Decision decision = decide(); decision == Decision::Allow) {
        KWin::kwinApp()->inputMethod()->show();
    } else {
        holdBack(decision);
    }
}

void TapToShowKeyboardCore::handleTextInputV3EnabledChanged()
{
    auto inputMethod = KWin::kwinApp()->inputMethod();
    auto t3 = KWin::waylandServer()->seat()->textInputV3();
    const Decision decision = t3->isEnabled() && t3->implicitShowPanelRequired() ? decide() : Decision::Allow;
    if (decision == Decision::Allow || inputMethod->isVisible()) {
        QMetaObject::invokeMethod(inputMethod, "textInputInterfaceV3EnabledChanged", Qt::DirectConnection);
        return;
    }

    holdBack(decision);
    // KWin's handler minus show(): activate, reset and adopt the context (setActive(true) adopts)
    inputMethod->setActive(true);
    if (auto context = KWin::waylandServer()->inputMethod()->context()) {
        context->sendReset();
        inputMethod->setActive(true);
    }
}

void TapToShowKeyboardCore::handleShowInputPanelRequested()
{
    if (const Decision decision = decide(); decision == Decision::Allow) {
        KWin::kwinApp()->inputMethod()->show();
    } else {
        holdBack(decision);
    }
}

void TapToShowKeyboardCore::holdBack(Decision decision)
{
    auto inputMethod = KWin::kwinApp()->inputMethod();
    if (inputMethod->isVisible()) {
        return;
    }
    if (decision == Decision::Defer) {
        m_deferred = true;
    } else if (decision == Decision::Block && inputMethod->panel() != m_touchWindow) {
        // Not after taps on the keyboard itself, e.g. its minimize button
        m_heldBack = true;
    }
    // Clear KWin's pending show request, or it shows the keyboard as soon as the input
    // method maps its window
    inputMethod->hide();
}

bool TapToShowKeyboardCore::touchDown(KWin::TouchDownEvent *event)
{
    if (m_touchStart.isEmpty()) {
        m_isTap = true;
        m_deferred = false;
        // KWin updates the touch focus before running filters
        m_touchWindow = KWin::input()->touch()->focus();
    } else {
        m_isTap = false;
    }
    m_touchStart.insert(event->id, event->pos);
    return false;
}

bool TapToShowKeyboardCore::touchMotion(KWin::TouchMotionEvent *event)
{
    const auto it = m_touchStart.constFind(event->id);
    if (m_isTap && it != m_touchStart.constEnd() && QLineF(*it, event->pos).length() > s_tapSlop) {
        m_isTap = false;
    }
    return false;
}

bool TapToShowKeyboardCore::touchUp(KWin::TouchUpEvent *event)
{
    if (m_touchStart.remove(event->id) && m_touchStart.isEmpty()) {
        endSequence();
    }
    return false;
}

bool TapToShowKeyboardCore::touchCancel()
{
    if (!m_touchStart.isEmpty()) {
        m_touchStart.clear();
        m_isTap = false;
        endSequence();
    }
    return false;
}

void TapToShowKeyboardCore::forceShow()
{
    m_forceShowTimer.start();
    m_heldBack = false;
    KWin::kwinApp()->inputMethod()->forceActivate();
}

void TapToShowKeyboardCore::setSuppressAfterPhysicalInput(bool suppress)
{
    m_suppressAfterPhysicalInput = suppress;
}

// Catches the keyboard showing through paths not taken over (initial focus, input method
// mapping its window, ...)
void TapToShowKeyboardCore::handleVisibleChanged()
{
    auto inputMethod = KWin::kwinApp()->inputMethod();
    if (!inputMethod->isVisible()) {
        return;
    }
    if (m_forceShowTimer.isValid() && m_forceShowTimer.elapsed() < s_forceShowGraceMs) {
        m_forceShowTimer.invalidate();
        m_heldBack = false;
        return;
    }

    const Decision decision = decide();
    // Interacting with the keyboard itself, e.g. it remapped after a layout switch
    if (decision == Decision::Allow || (decision != Decision::Suppress && m_touchWindow && m_touchWindow == inputMethod->panel())) {
        m_heldBack = false;
        return;
    }

    qCDebug(KWIN_TOUCHSCREENIMPROVEMENTS) << "Hiding keyboard shown without a tap";
    m_deferred = m_deferred || decision == Decision::Defer;
    m_heldBack = m_heldBack || decision == Decision::Block;
    inputMethod->hide();
}

void TapToShowKeyboardCore::endSequence()
{
    const bool wanted = m_deferred || m_heldBack;
    m_deferred = false;
    if (!wanted) {
        return;
    }
    // Show it ourselves: an app whose field already has focus may not ask again for this tap
    auto inputMethod = KWin::kwinApp()->inputMethod();
    if (lastTouchWasTapOn(textInputWindow()) && inputMethod->isActive() && !inputMethod->isVisible()) {
        m_heldBack = false;
        inputMethod->show();
    }
}

#include "moc_taptoshowkeyboardcore.cpp"
