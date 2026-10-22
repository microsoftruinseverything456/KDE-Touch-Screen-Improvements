/*
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "touchscreenimprovementseffect.h"
#include "physicalinputcore.h"
#include "taptoshowkeyboardcore.h"
#include "touchlongpresscore.h"
#include "touchscreenimprovementsconfig.h"
#include "virtualinputdevice.h"

#include "effect/effecthandler.h"
#include "input.h"
#include "inputmethod.h"
#include "main.h"
#include "wayland/seat.h"
#include "wayland_server.h"
#include "window.h"

#include <linux/input-event-codes.h>

using namespace std::chrono_literals;

static constexpr auto s_menuCloseTimeout = 500ms;

TouchScreenImprovementsEffect::TouchScreenImprovementsEffect()
    : m_keyDevice(std::make_unique<VirtualInputDevice>(QStringLiteral("Touch screen improvements keyboard"), true, false))
{
    TouchScreenImprovementsConfig::instance(KWin::effects->config());
    KWin::input()->addInputDevice(m_keyDevice.get());

    connect(&m_swipeUpAction, &QAction::triggered, this, &TouchScreenImprovementsEffect::handleSwipeUp);
    m_menuCloseTimeout.setSingleShot(true);
    m_menuCloseTimeout.setInterval(s_menuCloseTimeout);
    connect(&m_menuCloseTimeout, &QTimer::timeout, this, &TouchScreenImprovementsEffect::handleMenuClosed);
    reconfigure(ReconfigureAll);
}

// The parts restore KWin's handlers and release emulated buttons when destroyed
TouchScreenImprovementsEffect::~TouchScreenImprovementsEffect()
{
    if (m_swipeUpRegistered) {
        KWin::effects->unregisterTouchBorder(KWin::ElectricBottom, &m_swipeUpAction);
    }
    KWin::input()->removeInputDevice(m_keyDevice.get());
}

template<typename T>
static void enable(std::unique_ptr<T> &part, bool enabled)
{
    if (!enabled) {
        part.reset();
    } else if (!part) {
        part = std::make_unique<T>();
    }
}

void TouchScreenImprovementsEffect::reconfigure(ReconfigureFlags)
{
    TouchScreenImprovementsConfig::self()->read();
    const bool hideOnPhysicalInput = TouchScreenImprovementsConfig::hideKeyboardOnPhysicalInput();
    const bool rightClick = TouchScreenImprovementsConfig::longPressRightClick();
    const bool select = TouchScreenImprovementsConfig::longPressSelect();
    const bool swipeUp = TouchScreenImprovementsConfig::swipeUpForKeyboard();

    enable(m_tapToShowKeyboard, TouchScreenImprovementsConfig::tapToShowKeyboard());
    if (m_tapToShowKeyboard) {
        m_tapToShowKeyboard->setSuppressAfterPhysicalInput(hideOnPhysicalInput);
    }
    enable(m_physicalInput, hideOnPhysicalInput);
    enable(m_longPress, rightClick || select);
    if (m_longPress) {
        m_longPress->setRightClickEnabled(rightClick);
        m_longPress->setSelectEnabled(select);
    }

    if (swipeUp != m_swipeUpRegistered) {
        if (swipeUp) {
            KWin::effects->registerTouchBorder(KWin::ElectricBottom, &m_swipeUpAction);
        } else {
            KWin::effects->unregisterTouchBorder(KWin::ElectricBottom, &m_swipeUpAction);
        }
        m_swipeUpRegistered = swipeUp;
    }
}

void TouchScreenImprovementsEffect::handleSwipeUp()
{
    // An open context menu has the keyboard focus, so the virtual keyboard couldn't type
    // into the text field. Close it with Escape (keeps a selection) and show the keyboard
    // once it's gone.
    auto seat = KWin::waylandServer()->seat();
    KWin::Window *focused = seat->focusedKeyboardSurface() ? KWin::waylandServer()->findWindow(seat->focusedKeyboardSurface()) : nullptr;
    if (!focused || !focused->isPopupWindow()) {
        showKeyboard();
        return;
    }

    m_waitingForMenu = true;
    m_closingMenu = focused;
    connect(focused, &KWin::Window::closed, this, &TouchScreenImprovementsEffect::handleMenuClosed, Qt::SingleShotConnection);
    m_menuCloseTimeout.start();

    const auto now = VirtualInputDevice::now();
    Q_EMIT m_keyDevice->keyChanged(KEY_ESC, KWin::KeyboardKeyState::Pressed, now, m_keyDevice.get());
    Q_EMIT m_keyDevice->keyChanged(KEY_ESC, KWin::KeyboardKeyState::Released, now + 1ms, m_keyDevice.get());
}

// Menu closed or timed out
void TouchScreenImprovementsEffect::handleMenuClosed()
{
    if (!m_waitingForMenu) {
        return;
    }
    m_waitingForMenu = false;
    m_menuCloseTimeout.stop();
    if (m_closingMenu) {
        disconnect(m_closingMenu, &KWin::Window::closed, this, &TouchScreenImprovementsEffect::handleMenuClosed);
        m_closingMenu.clear();
    }
    showKeyboard();
}

void TouchScreenImprovementsEffect::showKeyboard()
{
    if (m_tapToShowKeyboard) {
        m_tapToShowKeyboard->forceShow();
    } else {
        KWin::kwinApp()->inputMethod()->forceActivate();
    }
}

bool TouchScreenImprovementsEffect::isActive() const
{
    return false;
}

#include "moc_touchscreenimprovementseffect.cpp"
