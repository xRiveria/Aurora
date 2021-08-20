#include "Aurora.h"
#include "Entity.h"
#include "Components/Camera.h"
#include "Components/Light.h"
#include "Components/Mesh.h"
#include "Components/RigidBody.h"
#include "Components/Collider.h"
#include <type_traits>
#include "../Scene/World.h"
#include <functional>

namespace Aurora
{
    Entity::Entity(EngineContext* engineContext)
    {
        m_EngineContext = engineContext;
        m_ObjectName = "Entity";
        m_IsActive = true;

        m_Transform = AddComponent<Transform>();
    }

    Entity::~Entity()
    {
        m_IsActive = false;
        m_IsVisibleInHierarchy = false;
        m_Transform = nullptr;
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
        // Call component OnStart() across all of the engine's components.
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

    void Entity::Clone()
    {
        auto world = m_EngineContext->GetSubsystem<World>();
        std::vector<Entity*> clonedEntities;

        // Creation of new entity and copying of its selected properties.
        auto CloneEntity = [&world, &clonedEntities](Entity* entity)
        {
            // Clone the name and ID.
            Entity* clonedEntity = world->EntityCreate().get();
            clonedEntity->SetObjectID(GenerateObjectID());
            clonedEntity->SetName(entity->GetObjectName());
            clonedEntity->SetActive(entity->IsActive());
            clonedEntity->SetHierarchyVisibility(entity->IsVisibleInHierarchy());

            // Clone all of its components.
            for (const auto& component : entity->GetAllComponents())
            {
                const auto& originalComponent = component;
                //auto clonedComponent = clonedEntity->AddComponent(component->GetType());
                //clonedComponent->SetAttributes(originalComponent->GetAttributes());
            }

            clonedEntities.emplace_back(clonedEntity);

            return clonedEntity;
        };

        // Cloning of an entity and its descendants.
        std::function<Entity*(Entity*)> CloneEntityAndDescendants = [&CloneEntityAndDescendants, &CloneEntity](Entity* original)
        {
            // Clone self.
            const auto clonedSelf = CloneEntity(original);

            // Clone children.
            for (const auto& childTransform : original->GetTransform()->GetChildren())
            {
                const auto clonedChild = CloneEntityAndDescendants(childTransform->GetEntity());
                clonedChild->GetTransform()->SetParentTransform(clonedSelf->GetTransform());
            }

            // Return self.
            return clonedSelf;
        };

        // Clone the entire hierarchy.
        CloneEntityAndDescendants(this);
    }

    IComponent* Entity::AddComponent(ComponentType componentType, uint32_t componentID)
    {
        switch (componentType)
        {
            case ComponentType::Camera: return AddComponent<Camera>(componentID);
            case ComponentType::Light: return AddComponent<Light>(componentID);
            case ComponentType::Transform: return AddComponent<Transform>(componentID);
            case ComponentType::Mesh: return AddComponent<Mesh>(componentID);
            case ComponentType::RigidBody: return AddComponent<RigidBody>(componentID);
            case ComponentType::Collider: return AddComponent<Collider>(componentID);
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