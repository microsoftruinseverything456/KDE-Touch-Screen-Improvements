/*
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "input.h"
#include "input_event.h"

#include <QElapsedTimer>
#include <QHash>
#include <QList>
#include <QObject>
#include <QPointF>
#include <QPointer>

#include <functional>

namespace KWin
{
class Window;
}

/**
 * Only lets touch bring up the virtual keyboard through a deliberate tap on the window
 * that owns the focused text field, not when scrolling over a field, raising a window
 * with a focused field, etc. Takes over KWin's handlers that show the keyboard; requests
 * it holds back are remembered so the next tap on that window shows the keyboard.
 */
class TapToShowKeyboardCore : public QObject, public KWin::InputEventFilter
{
    Q_OBJECT

public:
    TapToShowKeyboardCore();
    ~TapToShowKeyboardCore() override;

    bool touchDown(KWin::TouchDownEvent *event) override;
    bool touchMotion(KWin::TouchMotionEvent *event) override;
    bool touchUp(KWin::TouchUpEvent *event) override;
    bool touchCancel() override;

    // Shows the keyboard even without a text field and while a finger is down
    void forceShow();
    // Ignore keyboard requests that follow real mouse or keyboard input
    void setSuppressAfterPhysicalInput(bool suppress);

private:
    enum class Decision {
        Allow,
        Defer, // finger still down
        Block, // not a tap on the text input's window; remember for the next tap
        Suppress, // after real mouse/keyboard input; don't remember
    };
    Decision decide() const;
    bool lastTouchWasTapOn(const KWin::Window *window) const;
    KWin::Window *textInputWindow() const;

    void handleVisibleChanged();
    void handleTextInputV3EnableRequested();
    void handleTextInputV3EnabledChanged();
    void handleShowInputPanelRequested();
    void holdBack(Decision decision);
    void endSequence();
    template<typename Sender, typename Signal, typename Slot>
    void takeOver(Sender *sender, Signal signal, Slot slot, const char *kwinSlot);

    // Touch points of the current sequence and where they went down
    QHash<qint32, QPointF> m_touchStart;
    // The current or last sequence is a single stationary touch
    bool m_isTap = true;
    // Window that got the first touch of the sequence; null for effects like the overview
    QPointer<KWin::Window> m_touchWindow;
    // A request was deferred during the current sequence
    bool m_deferred = false;
    // A request was refused since the keyboard was last shown
    bool m_heldBack = false;
    bool m_suppressAfterPhysicalInput = false;
    QElapsedTimer m_forceShowTimer;
    QList<std::function<void()>> m_restoreHandlers;
};
