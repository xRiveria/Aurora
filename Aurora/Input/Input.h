#pragma once
#include "EngineContext.h"
#include "../Core/ISubsystem.h"

/* Personal Notes:

    Our input system depends on GLFW to function. Once again, we will use abstract pointers to our windows to faciliate possible future API changes.
    For now, we will support Mouse and Keyboard input + custom shortcuts.

    The input system is currently polling based, meaning we check for inputs directly every frame. We could convert this into an event based input system, but at this 
    point there really is no need to do so. See: https://www.reddit.com/r/gamedev/comments/1qee41/using_event_driven_vs_polling_input/.
*/

namespace Aurora
{
    class Input : public ISubsystem
    {
    public:
        Input(EngineContext* engineContext);
        ~Input() = default;

        bool Initialize() override;
        void Tick(float deltaTime) override;

        bool SetQueryWindow(void* window); // The window we're querying inputs for.

        // Use the below functions to create shortcut keys/check for input.
        bool IsKeyPressed(int keyCode);
        bool IsMouseButtonPressed(int mouseCode);
        void SetupInputCallbacks() const;

        bool RegisterShortcut(); /// Should we register shortcuts in a mapping?

    private:
        void* m_QueryWindow = nullptr;
    };
}