#pragma once
#include "RenderPath.h"
#include <string>
#include <vector>

namespace Aurora
{
    struct RenderItem2D
    {
        
    };

    struct RenderLayer2D
    {

    };

    class RenderPath2D : public RenderPath
    {
    public:
        virtual void ResizeBuffers();         // Create resolution dependant resources, such as render targets.
        virtual void ResizeLayout();          // Update DPI dependent elements, such as GUI elements and sprites.

        void FixedTick() override;
        void Tick(float deltaTime) override;
        void Render() const override;
        void Compose() const override;

        // Layers
        void AddLayer(const std::string layerName);
        void SetLayerOrder(const std::string& layerName, int order);
        void SortLayers();
        void CleanLayers();

    private:
        std::vector<RenderLayer2D> m_Layers{ 1 };
    };
}