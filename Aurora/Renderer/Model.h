#pragma once
#include "../Resource/AuroraResource.h"
#include "Material.h"

/* === Model ===
    - A model represents engine created data that attaches to a mesh (which contains raw data such as indices/vertices) loaded from some arbitrary format, such as FBX, GLTF etc.
    - As such, all forms of work relating to models will take in said model class, which is created whilst loading in its associated Mesh.
    - All forms of mesh manipulation will hence go through the model.
    - Similarly, serializations take in and release the Model itself.

    - Within the game world and editor, a model and its material it represented by a Renderable.
*/

namespace Aurora
{
    class DX11_VertexBuffer;
    class DX11_IndexBuffer;
    class ResourceCache;
    class Entity;
    class Mesh;

    class Model : public AuroraResource
    {

    public:
        Model(EngineContext* engineContext);
        ~Model() { Clear(); }

        void Clear();

        bool LoadFromFile(const std::string& filePath);
        bool SaveToFile(const std::string& filePath);

        // Geometry
        void AppendGeometry(
            const std::vector<uint32_t>& indices,
            const std::vector<XMFLOAT3>& vertexPositions,
            const std::vector<XMFLOAT3>& vertexNormals,
            const std::vector<XMFLOAT2>& vertexUVs);

        // Add resources to the model.
        void SetRootEntity(const std::shared_ptr<Entity>& entity) { m_RootEntity = entity; }
        void AddMaterial(std::shared_ptr<Material>& material, const std::shared_ptr<Entity>& entity);

        // Animations
        bool IsAnimated() const { return m_IsAnimated; }
        void SetAnimated(const bool isAnimated) { m_IsAnimated = isAnimated; }

        // Buffers
        DX11_VertexBuffer* GetVertexBuffer() const { return m_VertexBuffer.get(); }
        DX11_IndexBuffer* GetIndexBuffer() const { return m_IndexBuffer.get(); }

    public:
        bool CreateBuffers();

    private:
        // Misc
        std::weak_ptr<Entity> m_RootEntity; // The root entity of the model.
        std::shared_ptr<DX11_IndexBuffer> m_IndexBuffer;
        std::shared_ptr<DX11_VertexBuffer> m_VertexBuffer;
        std::shared_ptr<Mesh> m_Mesh;
        /// Axis-Aligned Bounding Box
        bool m_IsAnimated = false;

        // Dependencies
        ResourceCache* m_ResourceCache;
    };
}