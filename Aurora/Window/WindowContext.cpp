#include "Aurora.h"
#include "WindowContext.h"
#include <GLFW/glfw3.h>

namespace Aurora
{
    static int8_t g_WindowCount = -1;

    WindowContext::WindowContext(EngineContext* engineContext) : ISubsystem(engineContext)
    {
        
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

    bool WindowContext::Initialize()
    {
        if (!glfwInit())
        {
            AURORA_ERROR("Failed to initialize window context subsystem.");
            return false;
        }

        AURORA_INFO("Successfully initialized GLFW dependency.");
        return true;
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
        for (auto it = m_Windows.begin(); it != m_Windows.end(); it++)
        {
            if (it->second == window)
            {
                glfwMakeContextCurrent(static_cast<GLFWwindow*>(window));
                AURORA_INFO("Window context successfully set.");
                return;
            }
        }

        AURORA_WARNING("Requested window does not exist in the window context mapping. Did you somehow create the window through the API directly? This is not advisable but we will proceed anyway. Manually adding window to mapping.");
        m_Windows[++g_WindowCount] = window;
        glfwMakeContextCurrent(static_cast<GLFWwindow*>(window));
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
}