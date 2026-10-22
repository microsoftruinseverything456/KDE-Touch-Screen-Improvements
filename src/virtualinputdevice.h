/*
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "core/inputdevice.h"

#include <chrono>

// Input device for emulated pointer or keyboard events
class VirtualInputDevice : public KWin::InputDevice
{
public:
    VirtualInputDevice(const QString &name, bool keyboard, bool pointer)
        : m_name(name)
        , m_keyboard(keyboard)
        , m_pointer(pointer)
    {
    }

    QString name() const override
    {
        return m_name;
    }
    bool isEnabled() const override
    {
        return true;
    }
    void setEnabled(bool) override
    {
    }
    bool isKeyboard() const override
    {
        return m_keyboard;
    }
    bool isPointer() const override
    {
        return m_pointer;
    }
    bool isTouchpad() const override
    {
        return false;
    }
    bool isTouch() const override
    {
        return false;
    }
    bool isTabletTool() const override
    {
        return false;
    }
    bool isTabletPad() const override
    {
        return false;
    }
    bool isTabletModeSwitch() const override
    {
        return false;
    }
    bool isLidSwitch() const override
    {
        return false;
    }

    static std::chrono::microseconds now()
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch());
    }

private:
    QString m_name;
    bool m_keyboard;
    bool m_pointer;
};
