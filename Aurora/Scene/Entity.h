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

    public:
        // Transform
        XMMATRIX m_Transform = XMMatrixIdentity(); /// This will be refactored into its own component. For now, all entities will have an identity matrix to serve as the world matrix.

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