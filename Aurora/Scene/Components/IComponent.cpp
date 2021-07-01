#include "Aurora.h"
#include "IComponent.h"
#include "../Entity.h"
#include "Camera.h"

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

    // Explict template instantiation. Returns the appropriate enum based on the type passed in. We ought to turn this into a proper template...
    template<>
    ComponentType IComponent::TypeToEnum<Camera>()
    {
        ValidateComponentType<Camera>();
        return ComponentType::Camera;
    }
}