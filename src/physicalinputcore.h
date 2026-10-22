/*
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "input_event_spy.h"

/**
 * Hides the virtual keyboard when a real mouse, touchpad or keyboard is used. Virtual
 * devices (emulated input, keys sent by the virtual keyboard) and system keys such as
 * volume, brightness or power are ignored.
 */
class PhysicalInputCore : public KWin::InputEventSpy
{
public:
    PhysicalInputCore();

    void pointerMotion(KWin::PointerMotionEvent *event) override;
    void pointerButton(KWin::PointerButtonEvent *event) override;
    void pointerAxis(KWin::PointerAxisEvent *event) override;
    void keyboardKey(KWin::KeyboardKeyEvent *event) override;
};
