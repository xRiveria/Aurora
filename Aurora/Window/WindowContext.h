#pragma once
#include <unordered_map>
#include "WindowUtilities.h"

/* Personal Notes:

    - If we are to expand our engine to include a project selector (Unity Hub), perhaps an enum based map will make more sense?
    - If so, for the window mapping, perhaps mapping each window to the above-mentioned enum based map would make more sense.
*/

struct GLFWwindow;

namespace Aurora
{
    class WindowContext
    {
    public:
        // Subsystem?
        void Initialize();
        GLFWwindow* Create(const WindowDescription& description);

    private:
        std::unordered_map<int8_t, GLFWwindow*> m_Windows;
    };
}