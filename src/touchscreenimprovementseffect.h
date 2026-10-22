/*
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "effect/effect.h"

#include <QAction>
#include <QPointer>
#include <QTimer>

#include <memory>

namespace KWin
{
class Window;
}

class PhysicalInputCore;
class TapToShowKeyboardCore;
class TouchLongPressCore;
class VirtualInputDevice;

/**
 * Touch screen improvements, each toggleable in the effect's settings: tap to show the
 * keyboard, long press to right-click and to select text, hiding the keyboard on
 * physical input, and swiping up from the bottom edge to show it. Doesn't paint; being
 * an effect makes it toggleable in System Settings.
 */
class TouchScreenImprovementsEffect : public KWin::Effect
{
    Q_OBJECT

public:
    TouchScreenImprovementsEffect();
    ~TouchScreenImprovementsEffect() override;

    void reconfigure(ReconfigureFlags flags) override;
    bool isActive() const override; // false: skips the paint and window event paths

private:
    void handleSwipeUp();
    void handleMenuClosed();
    void showKeyboard();

    std::unique_ptr<TapToShowKeyboardCore> m_tapToShowKeyboard;
    std::unique_ptr<TouchLongPressCore> m_longPress;
    std::unique_ptr<PhysicalInputCore> m_physicalInput;

    QAction m_swipeUpAction;
    bool m_swipeUpRegistered = false;

    // Closing a context menu with Escape before showing the keyboard
    std::unique_ptr<VirtualInputDevice> m_keyDevice;
    QPointer<KWin::Window> m_closingMenu;
    bool m_waitingForMenu = false;
    QTimer m_menuCloseTimeout;
};
