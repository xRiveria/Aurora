#pragma once
#include <vector>
#include "AuroraObject.h"
#include "Components/IComponent.h"
#include "../Graphics/RHI_Implementation.h"
#include "../Renderer/Renderer.h"
#include <DirectXPackedVector.h>

using namespace DirectX::PackedVector;

namespace Aurora
{
    // Normals. 
    struct Vertex_Position
    {
        XMFLOAT3 m_Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
        XMFLOAT2 m_TexCoord = XMFLOAT2(0.0f, 0.0f);
        XMFLOAT3 m_Normals = XMFLOAT3(0.0f, 0.0f, 0.0f);

        static const Format m_Format = Format::FORMAT_R32G32B32A32_FLOAT;  // XYZW

        void Populate(const XMFLOAT3& position, const XMFLOAT2& texCoords, const XMFLOAT3& normals)
        {
            m_Position.x = position.x;
            m_Position.y = position.y;
            m_Position.z = position.z;

            m_TexCoord.x = texCoords.x;
            m_TexCoord.y = texCoords.y;

            m_Normals.x = normals.x;
            m_Normals.y = normals.y;
            m_Normals.z = normals.z;
        }

        XMVECTOR LoadPosition() const
        {
            return XMLoadFloat3(&m_Position);
        }
    };

    struct Vertex_TexCoords
    {
        XMHALF2 m_TexCoords = XMHALF2(0.0f, 0.0f);

        void Populate(const XMFLOAT2& texCoords)
        {
            m_TexCoords = XMHALF2(texCoords.x, texCoords.y);
        }

        static const Format m_Format = Format::FORMAT_R16G16_FLOAT;
    };

    struct MeshComponent
    {
        std::vector<uint32_t> m_Indices; ///
        IndexBuffer_Format GetIndexFormat() const { return m_VertexPositions.size() > 65535 ? IndexBuffer_Format::Format_32Bit : IndexBuffer_Format::Format_16Bit; } ///

        std::vector<XMFLOAT3> m_VertexPositions;
        std::vector<XMFLOAT3> m_VertexNormals;
        std::vector<XMFLOAT2> m_UVSet_0;

        RHI_GPU_Buffer m_IndexBuffer;
        RHI_GPU_Buffer m_VertexBuffer_Position;

        std::shared_ptr<AuroraResource> m_BaseTexture = nullptr;

        EngineContext* m_EngineContext;
    };


    class Entity : public AuroraObject, public std::enable_shared_from_this<Entity>
    {
    public:

        Entity(EngineContext* engineContext);
        ~Entity();

        void Start();                   // Starts all owned Components.
        void Stop();                    // Stops all owned Components.
        void Tick(float deltaTime);     // Ticks all owned components.

        // Properties
        const std::string& GetObjectName() const { return m_ObjectName; }
        void SetName(const std::string& name) { m_ObjectName = name; }

        bool IsActive() const { return m_IsActive; }
        void SetActive(const bool isActive) { m_IsActive = isActive; }

        // Adds a component of type T.
        template<typename T>
        T* AddComponent(uint32_t componentID = 0)
        {
            const ComponentType type = IComponent::TypeToEnum<T>();

            // Return component in case it already exists. We will take note of this in the future for scripts as multiple of those can exist.
            if (HasComponent(type))
            {
                return GetComponent<T>();
            }

            // Create a new component.
            std::shared_ptr<T> newComponent = std::make_shared<T>(m_EngineContext, this, componentID);
            
            // Save new component.
            m_Components.emplace_back(std::static_pointer_cast<IComponent>(newComponent));
            m_ComponentMask |= GetComponentMask(type);

            // Initialize component.
            newComponent->SetType(type);
            newComponent->Initialize();

            /// Make the scene resolve.

            return newComponent.get();
        }

        IComponent* AddComponent(ComponentType componentType, uint32_t componentID = 0);

        // Returns a component of type T.
        template<typename T>
        T* GetComponent()
        {
            const ComponentType type = IComponent::TypeToEnum<T>();

            if (!HasComponent(type))
            {
                return nullptr;
            }

            for (const std::shared_ptr<IComponent>& component : m_Components)
            {
                if (component->GetType() == type)
                {
                    return static_cast<T*>(component.get());
                }
            }

            return nullptr;
        }

        // Returns components of type T if they exist.
        template<typename T>
        std::vector<T*> GetComponents()
        {
            std::vector<T*> components;
            const ComponentType type = IComponent::TypeToEnum<T>();

            if (!HasComponent(type))
            {
                return components;
            }

            for (const std::shared_ptr<IComponent>& component : m_Components)
            {
                if (component->GetType() != type)
                {
                    continue;
                }

                components.emplace_back(static_cast<T*>(component.get()));
            }

            return components;
        }

        // Checks if a component exists.
        constexpr bool HasComponent(const ComponentType componentType) { return m_ComponentMask & GetComponentMask(componentType); }

        template <typename T>
        bool HasComponent() { return HasComponent(IComponent::TypeToEnum<T>()); }

        // Removes a component if it exists.
        template <typename T>
        void RemoveComponent()
        {
            const ComponentType type = IComponent::TypeToEnum<T>();

            for (auto it = m_Components.begin(); it != m_Components.end();)
            {
                std::shared_ptr<IComponent> component = *it;
                if (component->GetType() == type)
                {
                    component->Remove();
                    it = m_Components.erase(it);
                    m_ComponentMask &= ~GetComponentMask(type);
                }
                else
                {
                    ++it;
                }
            }

            /// Resolve the world.
        }

        void RemoveComponentByID(uint32_t componentID);
        const std::vector<std::shared_ptr<IComponent>>& GetAllComponents() const { return m_Components; }

        void MarkForDestruction() { m_IsDestructionPending = true; }
        bool IsPendingDestruction() const { return m_IsDestructionPending; }
        
        // Direct access for performance critical usage, which is not safe.
        std::shared_ptr<Entity> GetPointerShared() { return shared_from_this(); }
        
        // Load
        void LoadModel(const std::string& filePath, const std::string& fileTexture);
        void ImportModel_OBJ(const std::string& filePath, const std::string& fileTexture);
        void CreateRenderData();

    public:
        // Temporary
        MeshComponent m_MeshComponent;

    private:
        constexpr uint32_t GetComponentMask(ComponentType componentType) { return static_cast<uint32_t>(1) << static_cast<uint32_t>(componentType); }
        
    private:
        std::string m_ObjectName = "Entity";
        bool m_IsActive = true;
        bool m_IsDestructionPending = false;

        // Components
        std::vector<std::shared_ptr<IComponent>> m_Components;
        uint32_t m_ComponentMask = 0;
    };
}