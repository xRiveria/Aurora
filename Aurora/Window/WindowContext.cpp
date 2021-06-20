#include "Aurora.h"
#include "WindowContext.h"
#include <GLFW/glfw3.h>

namespace Aurora
{
    static int8_t g_WindowCount = -1;

    void WindowContext::Initialize()
    {
        if (!glfwInit())
        {
            AURORA_ERROR("Failed to initialize Window Context subsystem.");
        }
    }

    GLFWwindow* WindowContext::Create(const WindowDescription& description)
    {
        GLFWwindow* newWindow = glfwCreateWindow(description.m_Width, description.m_Height, description.m_Name.c_str(), NULL, NULL);

        if (!newWindow)
        {
            AURORA_ERROR("Failed to create window.");
        }

        m_Windows[++g_WindowCount] = newWindow;

        return m_Windows[g_WindowCount];
    }
}