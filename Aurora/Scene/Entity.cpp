#include "Aurora.h"
#include "Entity.h"
#include "Components/Camera.h"
#include "Components/Light.h"
#include "Components/Renderable.h"
#include "Components/RigidBody.h"
#include "Components/Collider.h"
#include "Components/AudioSource.h"
#include <type_traits>
#include "../Scene/World.h"
#include <functional>

namespace Aurora
{
    Entity::Entity(EngineContext* engineContext)
    {
        m_EngineContext = engineContext;
        SetEntityName("Entity");
        m_IsActive = true;

        m_Transform = AddComponent<Transform>();
        m_WorldContext = m_EngineContext->GetSubsystem<World>();
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

    void Entity::Serialize(BinarySerializer* binarySerializer)
    {
        // Basic Data
        {
            binarySerializer->Write(m_IsActive);
            binarySerializer->Write(m_IsVisibleInHierarchy);
            binarySerializer->Write(GetObjectID());
            binarySerializer->Write(GetEntityName());
        }

        // Components
        {
            binarySerializer->Write(static_cast<uint32_t>(m_Components.size()));
            for (const std::shared_ptr<IComponent>& component : m_Components)
            {
                binarySerializer->Write(static_cast<uint32_t>(component->GetType()));
                binarySerializer->Write(component->GetObjectID());
            }

            for (const std::shared_ptr<IComponent>& component : m_Components)
            {
                component->Serialize(binarySerializer);
            }
        }

        // Children
        {
            auto children = GetTransform()->GetChildren();

            // Children count.
            binarySerializer->Write(static_cast<uint32_t>(children.size()));

            // Children IDs.
            for (const auto& child : children)
            {
                binarySerializer->Write(child->GetObjectID());
            }

            // Children
            for (const auto& child : children)
            {
                if (child->GetEntity())
                {
                    child->GetEntity()->Serialize(binarySerializer);
                }
                else
                {
                    AURORA_ERROR(LogLayer::Serialization, "Aborting... Child entity is a nullptr?");
                    break;
                }
            }
        }
    }

    void Entity::Deserialize(BinarySerializer* binaryDeserializer, Transform* parentTransform)
    {
        // Basic Info
        {
            binaryDeserializer->Read(&m_IsActive);
            binaryDeserializer->Read(&m_IsVisibleInHierarchy);
            binaryDeserializer->Read(&m_ObjectID);
            SetEntityName(binaryDeserializer->ReadAs<std::string>());
        }

        // Components
        {
            const uint32_t componentCount = binaryDeserializer->ReadAs<uint32_t>();
            for (uint32_t i = 0; i < componentCount; i++)
            {
                uint32_t componentType = static_cast<uint32_t>(ComponentType::Unknown);
                uint32_t componentID = 0;

                binaryDeserializer->Read(&componentType); // Load the component's type.
                binaryDeserializer->Read(&componentID);   // Load the component's ID.'

                auto addedComponent = AddComponent(static_cast<ComponentType>(componentType), componentID);
            }

            // Sometimes, there are component dependencies, such as a collider that needs to set its shape for a rigidbody. Hence, its important to create the components first, and then deserialize them.
            for (const auto& component : m_Components)
            {
                component->Deserialize(binaryDeserializer);
            }

            // Set the transform's parent.
            if (m_Transform)
            {
                m_Transform->SetParentTransform(parentTransform);
            }
        }

        World* scene = m_EngineContext->GetSubsystem<World>();

        // Children
        {
            // Children Count
            const uint32_t childrenCount = binaryDeserializer->ReadAs<uint32_t>();

            // Children IDs
            std::vector<std::weak_ptr<Entity>> childEntities;
            for (uint32_t i = 0; i < childrenCount; i++)
            {
                auto child = scene->EntityCreate();
                child->SetObjectID(binaryDeserializer->ReadAs<uint32_t>());
                childEntities.emplace_back(child);
            }

            // Children
            for (const auto& child : childEntities)
            {
                child.lock()->Deserialize(binaryDeserializer, GetTransform());
            }

            if (m_Transform)
            {
                m_Transform->AcquireChildren();
            }
        }

        scene->SetSceneDirty();
    }

    void Entity::Clone()
    {
        std::vector<Entity*> clonedEntities;

        // Creation of new entity and copying of its selected properties.
        auto CloneEntity = [this, &clonedEntities](Entity* entity)
        {
            // Clone the name and ID.
            Entity* clonedEntity = m_WorldContext->EntityCreate().get();
            clonedEntity->SetObjectID(GenerateObjectID()); // Generate new object ID.
            clonedEntity->SetEntityName(entity->GetEntityName());
            clonedEntity->SetActive(entity->IsActive());
            clonedEntity->SetHierarchyVisibility(entity->IsVisibleInHierarchy());

            // Clone all of its components.
            for (const std::shared_ptr<IComponent>& component : entity->GetAllComponents())
            {
                const std::shared_ptr<IComponent>& originalComponent = component;
                IComponent* clonedComponent = clonedEntity->AddComponent(component->GetType());
                clonedComponent->SetComponentAttributes(originalComponent->GetComponentAttributes());
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
            case ComponentType::Renderable: return AddComponent<Renderable>(componentID);
            case ComponentType::RigidBody: return AddComponent<RigidBody>(componentID);
            case ComponentType::Collider: return AddComponent<Collider>(componentID);
            case ComponentType::AudioSource: return AddComponent<AudioSource>(componentID);
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