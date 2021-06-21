#pragma once
#include "EngineContext.h"
#include "../Core/ISubsystem.h"

/* Personal Notes:

    Our input system depends on GLFW to function. Once again, we will use abstract pointers to our windows to faciliate possible future API changes.
    For now, we will support Mouse and Keyboard input + custom shortcuts.
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

        void PollMouseInput();
        void PollKeyInput();

        bool SetQueryWindow(void* window); // The window we're querying.

        // Use the below functions to create shortcut keys.
        bool IsKeyPressed(int keyCode);
        bool IsMouseButtonPressed(int mouseCode);
        bool RegisterShortcut(); /// Should we register shortcuts in a mapping?

    private:
        void* m_QueryWindow = nullptr;
    };
}