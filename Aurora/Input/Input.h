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
    struct MouseButtonPressedEvent
    {
        double xPos;
        double yPos;

        MouseButtonPressedEvent(double xPoss, double yPoss) : xPos(xPoss), yPos(yPoss)
        {
            AURORA_INFO("Raw Window Coordinates: %f, %f", static_cast<float>(xPos), static_cast<float>(yPos));

            // Transform into 3D normalized device coordinates between the ranges x Y Z -1 : 1. We already have an X and Y, so we scale their range and reverse the Y direction.
            float x = (2.0f * xPos) / 1280.0f - 1.0f;
            float y = 1.0f - (2.0 * yPos) / 720.0f;
            float z = -1.0f;

            AURORA_INFO("Normalized Window Coordinates: %f, %f", x, y);

            // Now, we want our Z to point forward. With DX11, this is usuallt the positive Z direction. We can ads a W here to allow for a 4D vector.
            float w = 1.0;
            AURORA_INFO("Normalized Window Coordinates: %f, %f, %f, %f", x, y, z, w);

        }
    };

    class Input : public ISubsystem
    {
    public:
        Input(EngineContext* engineContext);
        ~Input() = default;

        bool Initialize() override;
        void Tick(float deltaTime) override;

        void PollMouse();

        bool SetQueryWindow(void* window); // The window we're querying inputs for.

        // Use the below functions to create shortcut keys/check for input.
        const std::pair<float, float> GetMousePositionPreviousFrame() const { return m_MousePositionPreviousFrame; }
        const std::pair<float, float> GetMousePositionDelta() const { return m_MousePositionDelta; }

        bool IsKeyPressed(int keyCode);
        bool IsMouseButtonPressed(int mouseCode);
        void SetupInputCallbacks();

        bool RegisterShortcut(); /// Should we register shortcuts in a mapping?

    public:
        bool m_IsDPressed;
        bool m_IsCtrlPressed;

    private:
        void* m_QueryWindow = nullptr;

        std::pair<float, float> m_MousePositionPreviousFrame;
        std::pair<float, float> m_MousePositionDelta;
    };
}