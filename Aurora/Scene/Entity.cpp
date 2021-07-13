#include "Aurora.h"
#include "Entity.h"
#include "Components/Camera.h"
#include "../Core/FileSystem.h"
#include <type_traits>

namespace Aurora
{
    Entity::Entity(EngineContext* engineContext)
    {
        m_EngineContext = engineContext;
        m_ObjectName = "Entity";
        m_IsActive = true;
    }

    Entity::~Entity()
    {
        m_IsActive = false;
        m_EngineContext = nullptr;

        m_ObjectName.clear();
        m_ComponentMask = 0;
        
        for (auto it = m_Components.begin(); it != m_Components.end();)
        {
            (*it)->Remove();
            (*it).reset();
            it = m_Components.erase(it);
        }

        m_Components.clear();
    }

    void Entity::Start()
    {
        // Call component Start() across all of the engine's components.
        for (const std::shared_ptr<IComponent>& component : m_Components)
        {
            component->Start();
        }
    }

    void Entity::Stop()
    {
        // Call component Stop() across all of the engine's components.
        for (const std::shared_ptr<IComponent>& component : m_Components)
        {
            component->Stop();
        }
    }

    void Entity::Tick(float deltaTime)
    {
        if (!m_IsActive)
        {
            return;
        }

        // Call component Update() across all of the engine's components.
        for (const std::shared_ptr<IComponent>& component : m_Components)
        {
            component->Tick(deltaTime);
        }
    }

    IComponent* Entity::AddComponent(ComponentType componentType, uint32_t componentID)
    {
        switch (componentType)
        {
            case ComponentType::Camera: return AddComponent<Camera>(componentID);
            default: nullptr;
        }

        return nullptr;
    }

    void Entity::RemoveComponentByID(uint32_t componentID)
    {
        ComponentType componentType = ComponentType::Unknown;

        for (auto it = m_Components.begin(); it != m_Components.end();)
        {
            std::shared_ptr<IComponent> component = *it;
            
            if (componentID == component->GetObjectID())
            {
                componentType = component->GetType();
                component->Remove();
                it = m_Components.erase(it);
                break;
            }
            else
            {
                ++it;
            }
        }

        bool othersOfSameTypeExist = false;
        /// Script component stuff in the future.
        if (!othersOfSameTypeExist)
        {
            m_ComponentMask &= ~GetComponentMask(componentType);
        }
        /// Resolve scene.
    }  
}