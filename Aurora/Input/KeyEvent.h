#pragma once
#include <string>
#include <sstream>
#include <GLFW/glfw3.h>

/// Temporary.

namespace Aurora
{
    class KeyEvent
    {
    public:
        int GetKeyCode() const { return m_KeyCode; }

    protected:
        KeyEvent(int keyCode) : m_KeyCode(keyCode) {}

    protected:
        int m_KeyCode;
    };

    class KeyPressedEvent : public KeyEvent
    {
    public:
        KeyPressedEvent(int keyCode, int repeatCount) : KeyEvent(keyCode), m_RepeatCount(repeatCount) {}
        int GetRepeatCount() const { return m_RepeatCount; }

        std::string ToString() const
        {
            std::stringstream ss;
            ss << "Key Pressed Event: " << GetKeyCode() << " (" << GetRepeatCount() << " Repeats" << ")";
            return ss.str();
        }

    private:
        int m_RepeatCount;
    };
}