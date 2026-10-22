/*
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "core/inputdevice.h"
#include "input.h"
#include "input_event.h"

#include <QElapsedTimer>
#include <QObject>
#include <QPointF>
#include <QPointer>
#include <QTimer>

#include <chrono>
#include <memory>

class VirtualInputDevice;

namespace KWin
{
class Window;
}

/**
 * Turns touch-and-hold into mouse input through a virtual pointer:
 * hold and lift right-clicks; hold and move double-click-drags (word-wise selection in
 * most toolkits) and right-clicks on the selection when lifted.
 *
 * Once a hold is recognized the touch is cancelled for the app, so its own long-press
 * handling doesn't fire, and the finger is followed through the raw device events.
 * With right-click off, the touch is only taken over once the finger moves.
 */
class TouchLongPressCore : public QObject, public KWin::InputEventFilter
{
    Q_OBJECT

public:
    TouchLongPressCore();
    ~TouchLongPressCore() override;

    void setRightClickEnabled(bool enabled);
    void setSelectEnabled(bool enabled);

    // Whether pointer input was emulated from a long press very recently
    static bool recentlyEmulated();

    bool touchDown(KWin::TouchDownEvent *event) override;
    bool touchMotion(KWin::TouchMotionEvent *event) override;
    bool touchUp(KWin::TouchUpEvent *event) override;
    bool touchCancel() override;

private:
    enum class State {
        Idle,
        Pending, // waiting for the hold timeout
        Held, // touch taken over; lift right-clicks, moving selects
        HeldPassive, // right-click off; the app still has the touch
        Selecting,
    };

    void reset();
    void handleHoldTimeout();
    void watchDevice(KWin::InputDevice *device);
    void handleRawTouchMotion(qint32 id, const QPointF &pos);
    void handleRawTouchUp(qint32 id);
    void handleRawTouchCanceled();

    void rightClick(const QPointF &pos, std::chrono::milliseconds delay);
    void handleWindowSettled();
    void cancelPendingRightClick();
    void sendRightClick(const QPointF &pos);
    void movePointer(const QPointF &pos);
    void sendButton(quint32 button, KWin::PointerButtonState state, std::chrono::milliseconds timeOffset = {});

    bool m_rightClickEnabled = true;
    bool m_selectEnabled = true;

    State m_state = State::Idle;
    qint32 m_touchId = -1;
    QPointF m_startPos;
    QPointF m_lastPos;
    QPointer<KWin::Window> m_window;
    // False on window decorations, where a double-click would maximize
    bool m_allowSelection = false;
    QTimer m_holdTimer;

    // Right-click waiting for the window to settle after hiding the virtual keyboard
    QPointer<KWin::Window> m_pendingWindow;
    QPointF m_pendingOffset; // from the window's client area origin
    QTimer m_settleTimer;
    QElapsedTimer m_settleDeadline;
    QMetaObject::Connection m_pendingGeometryConnection;

    std::unique_ptr<VirtualInputDevice> m_device;
};
