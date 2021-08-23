#pragma once
#include "../Resource/AuroraObject.h"
#include "EngineContext.h"
#include <any>
#include <functional>
#include "../Serializer/BinarySerializer.h"

namespace Aurora
{
    enum class ComponentType : uint32_t
    {
        Camera,
        Renderable,
        Light,
        Transform,
        RigidBody,
        Collider,
        Unknown
    };

    class Entity;

    class IComponent : public AuroraObject, public std::enable_shared_from_this<IComponent>
    {
    public:
        IComponent(EngineContext* engineContext, Entity* entity, uint32_t componentID = 0);
        virtual ~IComponent() = default;

        // Runs when the component is added to the entity.
        virtual void Initialize() {}

        // Runs every time the simulation begins.
        virtual void Start() {}

        // Runs every time the simulation stops.
        virtual void Stop() {}

        // Runs when the component is removed.
        virtual void Remove() {}

        // Runs every frame.
        virtual void Tick(float deltaTime) {}

        virtual void Serialize(BinarySerializer* binarySerializer) {}
        virtual void Deserialize(BinarySerializer* binaryDeserializer) {}

        // Type
        template <typename T>
        static constexpr ComponentType TypeToEnum();

        // Properties
        EngineContext* GetContext() const { return m_EngineContext; }
        Entity* GetEntity() const { return m_Entity; }
        std::string GetEntityName() const;

        ComponentType GetType() const { return m_Type; }
        void SetType(ComponentType componentType) { m_Type = componentType; }

    protected:    
        // The type of our component.
        ComponentType m_Type = ComponentType::Unknown;

        // The state of our component.
        bool m_IsEnabled = false;

        // The owner of the component.
        Entity* m_Entity = nullptr;
    };
}