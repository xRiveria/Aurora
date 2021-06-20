#include "Aurora.h"
#include "../Window/WindowContext.h"
#include <iostream>

namespace Aurora
{
    Engine::Engine()
    {
        // Test
        WindowContext windowContext;
        windowContext.Initialize();
        windowContext.Create({ "Aurora", 1280, 720 });

        AURORA_INFO("Initialized Engine.");
    }
}