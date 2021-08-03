#pragma once
#include <string>
#include <sstream>

// Example of an event based input system. This is how we register input data into our engine's data structures.
namespace Aurora
{
    enum class InputType
    {
        InputType_None = 0,
        WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
        KeyPressed, KeyReleased, KeyTyped,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
    };

    // A quick way to quickly filter incoming events without looping through all events individually. 
    enum InputCategory
    {
        InputCategory_None        = 1 << 0,
        InputCategory_Application = 1 << 1,
        InputCategory_Keyboard    = 1 << 2,
        InputCategory_Mouse       = 1 << 3,
        InputCategory_MouseButton = 1 << 4,
        InputCategory_Input       = 1 << 5
    };

#define INPUT_TYPE(type) static InputType GetStaticType() { return InputType::type; }               \
                                InputType GetInputType() const override { return GetStaticType(); } \
                                virtual const char* GetName() const override { return #type; }

#define INPUT_CATEGORY(category)  virtual int GetCategoryFlags() const override { return category; }

    class InputEvent
    {
    public:
        virtual ~InputEvent() = default;
        
        virtual InputType GetInputType() const = 0;
        virtual int GetCategoryFlags() const = 0;
        bool IsInCategory(InputCategory inputCategory) const
        {
            return GetCategoryFlags() & inputCategory;
        }

        virtual const char* GetName() const = 0;
        virtual std::string ToString() const { return GetName(); }

    public:
        bool IsEventHandled = false;
    };

    /*
        The dispatcher will dispatch the function if said function is polling for the specified input type. 
    */

    class InputEventDispatcher
    {
    public:
        InputEventDispatcher(InputEvent& inputEvent) : m_InputEvent(inputEvent) {}

        template<typename T, typename F>
        bool Dispatch(const F& function)
        {
            if (m_InputEvent.GetInputType() == T::GetStaticType())
            {
                m_InputEvent.IsEventHandled = function(static_cast<T&>(m_InputEvent)); 
                return true;
            }

            return false;
        }

    private:
        InputEvent& m_InputEvent;
    };
}