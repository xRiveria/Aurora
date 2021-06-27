#include "Aurora.h"
#include "Input.h"
#include "Future/KeyEvent.h"
#include "../Window/WindowContext.h"
#include <GLFW/glfw3.h>

namespace Aurora
{
    Input::Input(EngineContext* engineContext) : ISubsystem(engineContext)
    {
        
    }

    bool Input::Initialize()
    {
        WindowContext* windowContext = m_EngineContext->GetSubsystem<WindowContext>();
        if (!windowContext->IsInitialized())
        {
            AURORA_ERROR("GLFW has not been initialized. Please include the input system only after the window context has been created.");
            return false;
        }

        // We will set our initial query window to our rendering window created with the window context on initialization.
        SetQueryWindow(windowContext->GetRenderWindow());

        return true;
    }

    void Input::Tick(float deltaTime)
    {
        glfwPollEvents();

        PollMouse();  // To ensure that our mouse position and delta is always computed.
    }

    void Input::PollMouse()
    {
        // Get current mouse position.
        double xPosition, yPosition;
        glfwGetCursorPos(static_cast<GLFWwindow*>(m_QueryWindow), &xPosition, &yPosition);

        // Get delta.
        m_MousePositionDelta = { xPosition - m_MousePositionPreviousFrame.first, yPosition - m_MousePositionPreviousFrame.second };

        // Save current mouse position.
        m_MousePositionPreviousFrame = { xPosition, yPosition };
    }

    bool Input::SetQueryWindow(void* window)
    {
        if (m_EngineContext->GetSubsystem<WindowContext>()->WindowExistsInMapping(window))
        {
            m_QueryWindow = window;
            // SetupInputCallbacks();
         
            AURORA_INFO("Successfully directed input querying to window.");
            return true;
        }

        AURORA_ERROR("Requested query window does not exist in mapping. Did you create your window with the engine's window context?");
        return false;
    }

    bool Input::IsKeyPressed(int keyCode)
    {
        int keyState = glfwGetKey(static_cast<GLFWwindow*>(m_QueryWindow), keyCode);
        return keyState == GLFW_PRESS || keyState == GLFW_REPEAT;
    }

    bool Input::IsMouseButtonPressed(int mouseCode)
    {
        int mouseState = glfwGetMouseButton(static_cast<GLFWwindow*>(m_QueryWindow), mouseCode);
        return mouseState == GLFW_PRESS;
    }

    // Example of an event based input system. This is where we setup all our callbacks that will ultimately throw input data into our custom data structures.
    void Input::SetupInputCallbacks() const
    {
        glfwSetKeyCallback(static_cast<GLFWwindow*>(m_QueryWindow), [](GLFWwindow* window, int keyPressed, int scanCode, int action, int mods)
        {
            switch (action)
            {
                case GLFW_PRESS:
                {
                    KeyPressedEvent keyPressedEvent(keyPressed, 0);
                    AURORA_INFO(keyPressedEvent.ToString());
                    break;
                }

                case GLFW_REPEAT:
                {
                    KeyPressedEvent keyPressedEvent(keyPressed, 1);
                    AURORA_INFO(keyPressedEvent.ToString());
                    break;
                }
            }
        });
    }
}