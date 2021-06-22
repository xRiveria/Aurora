#pragma once
#include <string>
#include <sstream>

namespace Aurora
{
    // Example of an event based input system. This is how we register input data into our engine's data structures.

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
            ss << "Key Pressed Event: " << (char)GetKeyCode() << " (" << GetRepeatCount() << " Repeats" << ")";
            return ss.str();
        }

    private:
        int m_RepeatCount;
    };
}