#include "Aurora.h"
#include "WindowContext.h"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32 // For the inclusion of native access handles.
#include <GLFW/glfw3native.h>
#include "../Input/InputEvents/MouseEvent.h"
#include "../Input/InputEvents/KeyEvent.h"

namespace Aurora
{
    static int8_t g_WindowCount = -1;

    WindowContext::WindowContext(EngineContext* engineContext) : ISubsystem(engineContext)
    {
        
    }

    bool WindowContext::Initialize()
    {
        if (!glfwInit())
        {
            AURORA_ERROR("Failed to initialize window context subsystem.");
            return false;
        }

        m_IsInitialized = true;
        AURORA_INFO("Successfully initialized GLFW dependency.");

        // Creates a window by default. This is only for additional windows other than our main render Window.
        Create({ ENGINE_ARCHITECTURE, 1280, 720 });
        SetCurrentContext(0); // Index 0 is guarenteed to be our render window. Alternatively, use GetRenderWindow().

        // Allows us to associate an arbitrary piece of data relevant to our program with a GLFW window. We will use this to associate event callbacks within the GLFW library with our engine.
        glfwSetWindowUserPointer(static_cast<GLFWwindow*>(m_Windows[0]), &m_EventCallback);
        SetWindowCallbacks();

        return true;
    }

    void WindowContext::SetEventCallback(const EventCallbackFunction& inputCallback)
    {
        // Sets the callback to be, well, called.
        m_EventCallback = inputCallback;
    }

    void WindowContext::SetWindowCallbacks()
    {
        // Window Size

        // Window Close

        // Window Key
        glfwSetKeyCallback(static_cast<GLFWwindow*>(m_Windows[0]), [](GLFWwindow* window, int keyPressed, int scanCode, int action, int mods)
        {
            EventCallbackFunction& inputEventData = *(EventCallbackFunction*)glfwGetWindowUserPointer(window);
            switch (action)
            {
                case GLFW_PRESS:
                {
                    KeyPressedEvent keyPressedEvent(keyPressed, 0);
                    inputEventData(keyPressedEvent);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent keyReleasedEvent(keyPressed);
                    inputEventData(keyReleasedEvent);
                    break;
                }

                case GLFW_REPEAT:
                {
                    KeyPressedEvent keyRepeatedEvent(keyPressed, 1);
                    inputEventData(keyRepeatedEvent);
                    break;
                }
            }
        });

        // Window Char
        glfwSetCharCallback(static_cast<GLFWwindow*>(m_Windows[0]), [](GLFWwindow* window, unsigned int character)
        {
            EventCallbackFunction& inputEventData = *(EventCallbackFunction*)glfwGetWindowUserPointer(window);
            KeyTypedEvent keyTypedEvent(character);
            inputEventData(keyTypedEvent);
        });

        // Window Mouse Button
        glfwSetMouseButtonCallback(static_cast<GLFWwindow*>(m_Windows[0]), [](GLFWwindow* window, int buttonPressed, int action, int mods)
        {
            EventCallbackFunction& inputEventData = *(EventCallbackFunction*)glfwGetWindowUserPointer(window);
            switch (action)
            {
                case GLFW_PRESS:
                {
                    MouseButtonPressedEvent mousePressedEvent(buttonPressed);
                    inputEventData(mousePressedEvent);
                    break;
                }

                case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent mouseReleasedEvent(buttonPressed);
                    inputEventData(mouseReleasedEvent);
                    break;
                }
            }
        });

        // Window Scroll
        glfwSetScrollCallback(static_cast<GLFWwindow*>(m_Windows[0]), [](GLFWwindow* window, double xPosition, double yPosition)
        {
            EventCallbackFunction& inputEventData = *(EventCallbackFunction*)glfwGetWindowUserPointer(window);
            MouseScrolledEvent mouseScrolledEvent(static_cast<float>(xPosition), static_cast<float>(yPosition));
            inputEventData(mouseScrolledEvent);
        });

        // Window Cursor Position
        glfwSetCursorPosCallback(static_cast<GLFWwindow*>(m_Windows[0]), [](GLFWwindow* window, double xPosition, double yPosition)
        {
            EventCallbackFunction& inputEventData = *(EventCallbackFunction*)glfwGetWindowUserPointer(window);
            MouseMovedEvent mouseMovedEvent(static_cast<float>(xPosition), static_cast<float>(yPosition));
            inputEventData(mouseMovedEvent);
        });
    }

    void WindowContext::Shutdown()
    {
        for (uint8_t i = 0; i < m_Windows.size(); i++)
        {
            glfwDestroyWindow(static_cast<GLFWwindow*>(m_Windows[i])); // Automatically detaches said window is the current context on the main thread.
            AURORA_INFO("Successfully destroyed window.");
        }

        m_Windows.clear();
        glfwTerminate();
        AURORA_INFO("Successfully shutdown GLFW dependency.");
    }

    void WindowContext::Tick(float deltaTime)
    {
        if (!glfwWindowShouldClose(static_cast<GLFWwindow*>(GetRenderWindow())))
        {
            glfwPollEvents();
            // Window is still opening and ticking.
        }
        else
        {
            m_IsRunning = false;
        }
    }

    void* WindowContext::Create(const WindowDescription& description)
    {
        GLFWwindow* newWindow = glfwCreateWindow(description.m_Width, description.m_Height, description.m_Title.c_str(), NULL, NULL);

        if (!newWindow)
        {
            AURORA_ERROR("Failed to create window.");
            return nullptr;
        }

        AURORA_INFO("Successfully created window.");
        m_Windows[++g_WindowCount] = static_cast<void*>(newWindow);

        return m_Windows[g_WindowCount];
    }

    void WindowContext::SetCurrentContext(int windowID)
    {
        if (m_Windows.find(windowID) != m_Windows.end())
        {
            glfwMakeContextCurrent(static_cast<GLFWwindow*>(m_Windows[windowID]));
            AURORA_INFO("Window context successfully set.");
            return;
        }

        AURORA_WARNING("Window context cannot be set. The requested window ID does not exist.");
    }

    void WindowContext::SetCurrentContext(void* window)
    {
        if (WindowExistsInMapping(window))
        {
            glfwMakeContextCurrent(static_cast<GLFWwindow*>(window));
            AURORA_INFO("Window context successfully set.");
            return;
        }
        
        AURORA_WARNING("Requested window does not exist in the window context mapping. Did you somehow create the window through the API directly? This is not advisable but we will proceed anyway. Manually adding window to mapping.");
        m_Windows[++g_WindowCount] = window;
        glfwMakeContextCurrent(static_cast<GLFWwindow*>(window));
    }

    void WindowContext::SetCurrentContextTitle_Scene(const std::string& inputSceneName)
    {
        std::string engineArchitecture = inputSceneName + " - " + ENGINE_ARCHITECTURE;
        glfwSetWindowTitle(static_cast<GLFWwindow*>(GetRenderWindow()), engineArchitecture.c_str());
    }

    float WindowContext::GetWindowWidth(int windowID)
    {
        if (m_Windows.find(windowID) != m_Windows.end())
        {
            int width;
            glfwGetWindowSize(static_cast<GLFWwindow*>(m_Windows[windowID]), &width, nullptr);
            return static_cast<float>(width);
        }

        AURORA_ERROR("Requested window width could not be found. Does the window not exist in the mapping?");
        return 0.0f;
    }

    float WindowContext::GetWindowWidth(void* window)
    {
        int width;
        glfwGetWindowSize(static_cast<GLFWwindow*>(window), &width, nullptr);
        return static_cast<float>(width);
    }

    float WindowContext::GetWindowHeight(int windowID)
    {
        if (m_Windows.find(windowID) != m_Windows.end())
        {
            int height;
            glfwGetWindowSize(static_cast<GLFWwindow*>(m_Windows[windowID]), nullptr, &height);
            return static_cast<float>(height);
        }

        AURORA_ERROR("Requested window height could not be found. Does the window not exist in the mapping?");
        return 0.0f;
    }

    float WindowContext::GetWindowHeight(void* window)
    {
        int height;
        glfwGetWindowSize(static_cast<GLFWwindow*>(window), nullptr, &height);
        return static_cast<float>(height);
    }

    float WindowContext::GetWindowDPI(int windowID)
    {
        if (m_Windows.find(windowID) != m_Windows.end())
        {
            float dpi = (float)GetDpiForWindow(GetWindowHWND(windowID));
            return dpi;
        }

        AURORA_ERROR("Requested window DPI could not be found. Does the window not exist in the mapping?");
        return 0.0f;
    }

    float WindowContext::GetWindowDPI(void* window)
    {
        float dpi = (float)GetDpiForWindow(GetWindowHWND(window));
        return dpi;
    }

    HWND WindowContext::GetWindowHWND(int windowID)
    {
        if (m_Windows.find(windowID) != m_Windows.end())
        {
            return glfwGetWin32Window(static_cast<GLFWwindow*>(m_Windows[windowID]));
        }

        AURORA_ERROR("Requested window HWND could not be found. Does the window not exist in the mapping?");
        return nullptr;
    }

    HWND WindowContext::GetWindowHWND(void* window)
    {
        return glfwGetWin32Window(static_cast<GLFWwindow*>(window));
    }

    void* WindowContext::GetRenderWindow()
    {
        if (m_Windows[0] != nullptr)
        {
            return m_Windows[0];
        }

        AURORA_ERROR("Render window does not exist. This shouldn't be happening. Did you forget to create one?");
        return nullptr;
    }

    bool WindowContext::WindowExistsInMapping(void* window) const
    {
        for (auto it = m_Windows.begin(); it != m_Windows.end(); it++)
        {
            if (it->second == window)
            {
                return true;
            }
        }

        return false;
    }
}