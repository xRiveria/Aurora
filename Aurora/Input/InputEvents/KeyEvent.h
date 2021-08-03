#pragma once
#include "InputEvent.h"

namespace Aurora
{
    class KeyEvent : public InputEvent
    {
    public:
        int GetKeyCode() const { return m_KeyCode; }
        INPUT_CATEGORY(InputCategory::InputCategory_Keyboard | InputCategory::InputCategory_Input)

    protected:
        KeyEvent(int keyCode) : m_KeyCode(keyCode) {}
        int m_KeyCode;
    };

    class KeyPressedEvent : public KeyEvent
    {
    public:
        KeyPressedEvent(int keyCode, int repeatCount) : KeyEvent(keyCode), m_RepeatCount(repeatCount) {}
        int GetRepeatCount() const { return m_RepeatCount; }

        std::string ToString() const override
        {
            std::stringstream stringStream;
            stringStream << "Key Pressed Event: " << GetKeyCode() << " (" << GetRepeatCount() << ") Repeats";
            return stringStream.str();
        }

        INPUT_TYPE(KeyPressed)

    private:
        int m_RepeatCount;
    };

    class KeyTypedEvent : public KeyEvent
    {
    public:
        KeyTypedEvent(int keyCode) : KeyEvent(keyCode) {}

        std::string ToString() const override
        {
            std::stringstream stringStream;
            stringStream << "Key Typed Event: " << GetKeyCode();
            return stringStream.str();
        }
        
        INPUT_TYPE(KeyTyped)
    };

    class KeyReleasedEvent : public KeyEvent
    {
    public:
        KeyReleasedEvent(int keyCode) : KeyEvent(keyCode) {}

        std::string ToString() const override
        {
            std::stringstream stringStream;
            stringStream << "Key Released Event: " << GetKeyCode();
            return stringStream.str();
        }

        INPUT_TYPE(KeyReleased)
    };
}