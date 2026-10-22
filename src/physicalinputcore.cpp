/*
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "physicalinputcore.h"

#include "core/inputdevice.h"
#include "input.h"
#include "input_event.h"
#include "inputmethod.h"
#include "main.h"

// Only hardware (libinput) devices have a sysfs path
static bool isPhysical(const KWin::InputDevice *device)
{
    return device && !device->sysPath().isEmpty();
}

static void hideKeyboard()
{
    if (auto inputMethod = KWin::kwinApp()->inputMethod(); inputMethod->isVisible()) {
        inputMethod->hide();
    }
}

PhysicalInputCore::PhysicalInputCore()
{
    KWin::input()->installInputEventSpy(this);
}

void PhysicalInputCore::pointerMotion(KWin::PointerMotionEvent *event)
{
    if (isPhysical(event->device) && !event->warp && !event->delta.isNull()) {
        hideKeyboard();
    }
}

void PhysicalInputCore::pointerButton(KWin::PointerButtonEvent *event)
{
    if (isPhysical(event->device) && event->state == KWin::PointerButtonState::Pressed) {
        hideKeyboard();
    }
}

void PhysicalInputCore::pointerAxis(KWin::PointerAxisEvent *event)
{
    if (isPhysical(event->device)) {
        hideKeyboard();
    }
}

void PhysicalInputCore::keyboardKey(KWin::KeyboardKeyEvent *event)
{
    // Keys from Qt::Key_Back on are media, volume, brightness, power etc.
    if (isPhysical(event->device) && event->state == KWin::KeyboardKeyState::Pressed && event->key != 0 && event->key < Qt::Key_Back) {
        hideKeyboard();
    }
}
