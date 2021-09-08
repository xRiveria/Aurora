#pragma once
#include "RenderCanvas.h"

namespace Aurora
{
    class RenderPath : public RenderCanvas
    {
        public:
            virtual void Load()  {}                 // Load resources in the background (for example, behind a loading screen).
            virtual void Start() {}                 // Called when the render path is activated.
            virtual void Stop()  {}                 // Called when the render path is deactivated (for example, when switching to another render path).
            virtual void PreTick() {}               // Executed before Tick().
            virtual void FixedTick() {}             // Updated with fixed frequency.
            virtual void Tick(float deltaTime) {}   // Update once per frame. deltaTime is the elapsed time since the last call in seconds.
            virtual void PostTick() {}              // Executed after Tick().
            virtual void Render() const {}          // Render to layers, render targets, etc. This will be rendered offscreen (?).
            virtual void Compose() const {}         // Compose the rendered layers (for example, blend the layers together as Images).

            uint32_t GetLayerMask() const { return m_LayerMask; }
            void SetLayerMask(uint32_t value) { m_LayerMask = value; }

        private:
            uint32_t m_LayerMask = 0xFFFFFFFF;
    };
}