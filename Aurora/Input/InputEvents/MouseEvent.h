#pragma once
#include "InputEvent.h"

namespace Aurora
{
    class MouseMovedEvent : public InputEvent
    {
    public:
        MouseMovedEvent(float x, float y) : m_MouseX(x), m_MouseY(y) {}
        float GetX() const { return m_MouseX; }
        float GetY() const { return m_MouseY; }

        std::string ToString() const override
        {
            std::stringstream stringStream;
            stringStream << "Mouse Moved Event: " << GetX() << ", " << GetY();
            return stringStream.str();
        }

        INPUT_TYPE(MouseMoved)
        INPUT_CATEGORY(InputCategory::InputCategory_Mouse | InputCategory::InputCategory_Input)

    private:
        float m_MouseX;
        float m_MouseY;
    };

    class MouseScrolledEvent : public InputEvent
    {
    public:
        MouseScrolledEvent(float xOffset, float yOffset) : m_xOffset(xOffset), m_yOffset(yOffset) {}
        float GetXOffset() const { return m_xOffset; }
        float GetYOffset() const { return m_yOffset; }

        std::string ToString() const override
        {
            std::stringstream stringStream;
            stringStream << "Mouse Scrolled Event: " << GetXOffset() << ", " << GetYOffset();
            return stringStream.str();
        }

        INPUT_TYPE(MouseScrolled)
        INPUT_CATEGORY(InputCategory::InputCategory_Mouse | InputCategory::InputCategory_Input)

    private:
        float m_xOffset;
        float m_yOffset;
    };

    class MouseButtonEvent : public InputEvent
    {
    public:
        int GetMouseButton() const { return m_Button; }
        INPUT_CATEGORY(InputCategory::InputCategory_Mouse | InputCategory::InputCategory_Input)

    protected:
        MouseButtonEvent(int mouseButton) : m_Button(mouseButton) {}
        int m_Button;
    };

    class MouseButtonPressedEvent : public MouseButtonEvent
    {
    public:
        MouseButtonPressedEvent(int mouseButton) : MouseButtonEvent(mouseButton) {}

        std::string ToString() const override
        {
            std::stringstream stringStream;
            stringStream << "Mouse Button Pressed Event: " << GetMouseButton();
            return stringStream.str();
        }

        INPUT_TYPE(MouseButtonPressed)
    };

    class MouseButtonReleasedEvent : public MouseButtonEvent
    {
    public:
        MouseButtonReleasedEvent(int mouseButton) : MouseButtonEvent(mouseButton) {}

        std::string ToString() const override
        {
            std::stringstream stringStream;
            stringStream << "Mouse Button Released Event: " << GetMouseButton();
            return stringStream.str();
        }

        INPUT_TYPE(MouseButtonReleased)
    };
}