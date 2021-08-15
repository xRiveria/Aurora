#include "Aurora.h"
#include "IComponent.h"
#include "../Entity.h"
#include "Camera.h"
#include "Light.h"
#include "Mesh.h"
#include "Material.h"
#include "RigidBody.h"

namespace Aurora
{
    IComponent::IComponent(EngineContext* engineContext, Entity* entity, uint32_t componentID)
    {
        m_EngineContext = engineContext;
        m_Entity = entity;
        m_IsEnabled = true;
    }

    std::string IComponent::GetEntityName() const
    {
        if (!m_Entity)
        {
            return "";
        }

        return m_Entity->GetObjectName();
    }

    template<typename T>
    inline constexpr ComponentType IComponent::TypeToEnum()
    {
        return ComponentType::Unknown;
    }

    template<typename T>
    inline constexpr void ValidateComponentType() { static_assert(std::is_base_of<IComponent, T>::value, "Provided type does not implement IComponent."); }

    // Explict template instantiation.
    #define REGISTER_COMPONENT(T, enumT) template<> ComponentType IComponent::TypeToEnum<T>() { ValidateComponentType<T>(); return enumT; }

    // Register components here.
    REGISTER_COMPONENT(Camera, ComponentType::Camera);
    REGISTER_COMPONENT(Light, ComponentType::Light);
    REGISTER_COMPONENT(Mesh, ComponentType::Mesh);
    REGISTER_COMPONENT(Transform, ComponentType::Transform);
    REGISTER_COMPONENT(Material, ComponentType::Material);
    REGISTER_COMPONENT(RigidBody, ComponentType::RigidBody);
}