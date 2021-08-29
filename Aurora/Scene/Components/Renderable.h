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

        // Serialization
        void Serialize(BinarySerializer* binarySerializer) override;
        void Deserialize(BinarySerializer* binaryDeserializer) override;

        // Geometry
        void GeometrySet(const std::string& renderableName, uint32_t indexOffset, uint32_t indexSize, uint32_t vertexOffset, uint32_t vertexSize, Model* model);
        Model* GetGeometryModel() const { return m_Model; }
        uint32_t GetGeometryIndexOffset() const { return m_GeometryIndexOffset; }
        uint32_t GetGeometryVertexOffset() const { return m_GeometryVertexOffset; }
        uint32_t GetGeometryVerticesSize() const { return m_GeometryVertexSize; }
        uint32_t GetGeometryIndicesSize() const { return m_GeometryIndexSize; }


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
        uint32_t m_GeometryIndexOffset;
        uint32_t m_GeometryVertexOffset;
        uint32_t m_GeometryVertexSize;
        uint32_t m_GeometryIndexSize;

        bool m_IsUsingDefaultMaterial = false;

        Material* m_Material = nullptr;
        Model* m_Model = nullptr;
    };
}