#pragma once
#include <vector>
#include "AuroraObject.h"
#include "Components/IComponent.h"

namespace Aurora
{
    class Entity : public AuroraObject, public std::enable_shared_from_this<Entity>
    {
    public:
        Entity(EngineContext* engineContext, uint32_t m_TransformID = 0);
        ~Entity();

        void Start();                   // Starts all owned Components.
        void Stop();                    // Stops all owned Components.
        void Tick(float deltaTime);     // Ticks all owned components.

        // Properties
        const std::string& GetObjectName() const { return m_ObjectName; }
        void SetName(const std::string& name) { m_ObjectName = name; }

        bool IsActive() const { return m_IsActive; }
        bool SetActive(const bool isActive) { m_IsActive = isActive; }

        // Component Adding
        // Add Component
        // Remove Component
        // GetComponent
        // GetComponents
        // HasComponent

        void MarkForDestruction() { m_IsDestructionPending = true; }
        bool IsPendingDestruction() const { return m_IsDestructionPending; }
        
        // Direct access for performance critical usage, which is not safe.
        std::shared_ptr<Entity> GetPointerShared() { return shared_from_this(); }
        
    private:
        std::string m_ObjectName = "Entity";
        bool m_IsActive = true;
        bool m_IsDestructionPending = false;

        // Components
        std::vector<std::shared_ptr<IComponent>> m_Components;
        uint32_t m_ComponentMask = 0;
    };
}