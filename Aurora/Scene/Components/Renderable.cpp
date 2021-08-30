#include "Aurora.h"
#include "Renderable.h"
#include "../Renderer/Material.h"
#include "../Renderer/Model.h"
#include "../Resource/ResourceCache.h"
#include "../Graphics/DX11_Refactored/DX11_Texture.h"

namespace Aurora
{
    Renderable::Renderable(EngineContext* engineContext, Entity* entity, uint32_t componentID) : IComponent(engineContext, entity, componentID)
    {
        m_IsUsingDefaultMaterial = false;
        m_GeometryIndexOffset = 0;
        m_GeometryIndexSize = 0;
        m_GeometryVertexOffset = 0;
        m_GeometryVertexSize = 0;

        AURORA_REGISTER_ATTRIBUTE_VALUE_VALUE(m_IsUsingDefaultMaterial, bool);
        AURORA_REGISTER_ATTRIBUTE_VALUE_VALUE(m_Material, Material*);
        AURORA_REGISTER_ATTRIBUTE_VALUE_VALUE(m_Model, Model*);
        AURORA_REGISTER_ATTRIBUTE_VALUE_VALUE(m_GeometryName, std::string);
        AURORA_REGISTER_ATTRIBUTE_VALUE_VALUE(m_GeometryIndexOffset, uint32_t);
        AURORA_REGISTER_ATTRIBUTE_VALUE_VALUE(m_GeometryIndexSize, uint32_t);
        AURORA_REGISTER_ATTRIBUTE_VALUE_VALUE(m_GeometryVertexOffset, uint32_t);
        AURORA_REGISTER_ATTRIBUTE_VALUE_VALUE(m_GeometryVertexSize, uint32_t);
    }

    void Renderable::Serialize(BinarySerializer* binarySerializer)
    {
        // Mesh
        binarySerializer->Write(m_Model ? m_Model->GetResourceName() : "");
        binarySerializer->Write(m_GeometryIndexOffset);
        binarySerializer->Write(m_GeometryIndexSize);
        binarySerializer->Write(m_GeometryVertexOffset);
        binarySerializer->Write(m_GeometryVertexSize);

        // Material
        binarySerializer->Write(m_IsUsingDefaultMaterial);
        if (!m_IsUsingDefaultMaterial)
        {
            binarySerializer->Write(m_Material ? m_Material->GetResourceName() : "");
        }
    }

    void Renderable::Deserialize(BinarySerializer* binaryDeserializer)
    {
        // Mesh
        std::string modelName;
        binaryDeserializer->Read(&modelName);
        m_Model = m_EngineContext->GetSubsystem<ResourceCache>()->GetResourceByName<Model>(modelName).get();
        m_GeometryIndexOffset = binaryDeserializer->ReadAs<uint32_t>();
        m_GeometryIndexSize = binaryDeserializer->ReadAs<uint32_t>();
        m_GeometryVertexOffset = binaryDeserializer->ReadAs<uint32_t>();
        m_GeometryVertexSize = binaryDeserializer->ReadAs<uint32_t>();

        // Material
        binaryDeserializer->Read(&m_IsUsingDefaultMaterial);
        if (m_IsUsingDefaultMaterial)
        {
            UseDefaultMaterial();
        }
        else
        {
            std::string materialName;
            binaryDeserializer->Read(&materialName);
            m_Material = m_EngineContext->GetSubsystem<ResourceCache>()->GetResourceByName<Material>(materialName).get();
        }
    }

    void Renderable::GeometrySet(const std::string& renderableName, uint32_t indexOffset, uint32_t indexSize, uint32_t vertexOffset, uint32_t vertexSize, Model* model)
    {
        m_GeometryName = renderableName;
        m_GeometryVertexSize = vertexSize;
        m_GeometryVertexOffset = vertexOffset;
        m_GeometryIndexSize = indexSize;
        m_GeometryIndexOffset = indexOffset;
        m_Model = model;
    }

    std::shared_ptr<Material> Renderable::SetMaterial(const std::shared_ptr<Material>& material)
    {
        AURORA_ASSERT(material != nullptr);

        // In order for the component to guarentee serializxation/deserialization, we cache the material.
        std::shared_ptr<Material> cachedMaterial = m_EngineContext->GetSubsystem<ResourceCache>()->CacheResource(material);

        m_Material = cachedMaterial.get();

        // Set to false. Otherwise, the material won't serialize/deserialize.
        m_IsUsingDefaultMaterial = false;

        return cachedMaterial;
    }

    std::shared_ptr<Material> Renderable::SetMaterial(const std::string& filePath)
    {
        // Load the material.
        std::shared_ptr<Material> material = std::make_shared<Material>(m_EngineContext);

        if (!material->LoadFromFile(filePath))
        {
            AURORA_WARNING(LogLayer::Engine, "Failed to load material from path \"%s\".", filePath.c_str());
            return nullptr;
        }

        // Set it as the current material.
        return SetMaterial(material);
    }

    void Renderable::UseDefaultMaterial()
    {
        m_IsUsingDefaultMaterial = true;
        // Create Material
        std::shared_ptr<Material> material = std::make_shared<Material>(m_EngineContext);
        const std::string filePath = m_EngineContext->GetSubsystem<Settings>()->GetProjectDirectory() + "Standard" + EXTENSION_MATERIAL; 
        material->SetResourceFilePath(filePath);
        
        // Set default texture.
        const std::shared_ptr<DX11_Texture> defaultTexture = m_EngineContext->GetSubsystem<ResourceCache>()->Load<DX11_Texture>("../Resources/Textures/Default/Default.png");
        material->SetTextureSlot(MaterialSlot::MaterialSlot_Albedo, defaultTexture);

        // Set material.
        SetMaterial(material);
    }

    std::string Renderable::GetMaterialName() const
    {
        return m_Material ? m_Material->GetResourceName() : "";
    }
}