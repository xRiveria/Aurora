#pragma once
#include "IComponent.h"

namespace Aurora
{
    class Model;
    class Material;

    class Renderable : public IComponent
    {
    public:
        Renderable(EngineContext* engineContext, Entity* entity, uint32_t componentID = 0);
        ~Renderable() = default;

        // Geometry
        void GeometrySet(const std::string& renderableName, Model* model);
        Model* GetGeometryModel() const { return m_Model; }

        // Material
        // Sets a material from memory (adds it to the resource cache by default).
        std::shared_ptr<Material> SetMaterial(const std::shared_ptr<Material>& material);
        // Loads a material and sets it.
        std::shared_ptr<Material> SetMaterial(const std::string& filePath);

        void UseDefaultMaterial();
        std::string GetMaterialName() const;
        Material* GetMaterial() const { return m_Material; }
        bool HasMaterial() const { return m_Material != nullptr; }

    private:
        std::string m_GeometryName;
        bool m_IsUsingDefaultMaterial = false;

        Material* m_Material = nullptr;
        Model* m_Model = nullptr;
    };
}