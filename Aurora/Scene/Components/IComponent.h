#pragma once
#include "../Resource/AuroraObject.h"
#include "../Serializer/BinarySerializer.h"
#include "EngineContext.h"
#include <any>
#include <functional>
#include <typeinfo>

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
        AudioSource,
        Script,
        Unknown,
    };

    struct ComponentAttribute
    {
        std::function<std::any()> Getter;
        std::function<void(std::any)> Setter;
        std::function<bool(std::any)> Comparison;
    };

    class Entity;

    class IComponent : public AuroraObject, public std::enable_shared_from_this<IComponent>
    {
    public:
        /// Test
        std::vector<std::variant<bool, uint32_t, float>> m_TransactionAttributes;
        virtual const std::vector<std::variant<bool, uint32_t, float>> GetTransactionAttributes() { return std::vector<std::variant<bool, uint32_t, float>>(); }
        virtual const std::vector<std::function<void(std::variant<bool, uint32_t, float>)>> GetTransactionFunctions() 
        { 
            std::vector<std::function<void(std::variant<bool, uint32_t, float> value)>> stuff;
            
            return stuff;
        };

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
        ComponentType GetType() const { return m_Type; }
        void SetType(ComponentType componentType) { m_Type = componentType; }

        const std::vector<Aurora::ComponentAttribute>& GetComponentAttributes() const { return m_ComponentAttributes; }
        const std::vector<Aurora::ComponentAttribute> GetComponentAttributesCopy() const { return m_ComponentAttributes; }

        void SetComponentAttributes(const std::vector<ComponentAttribute>& attributes)
        {
            for (uint32_t i = 0; i < static_cast<uint32_t>(m_ComponentAttributes.size()); i++)
            {
                m_ComponentAttributes[i].Setter(attributes[i].Getter());
            }
        }
       
        // Entity
        Entity* GetEntity() const { return m_Entity; }
        std::string GetEntityName() const;

        EngineContext* GetContext() const { return m_EngineContext; }

    protected:
        #define AURORA_REGISTER_ATTRIBUTE_GET_SET(Getter, Setter, Type) RegisterAttribute(                                                 \
        [this]() { return Getter(); },                                                                                                     \
        [this](const std::any& valueIn) { Setter(std::any_cast<Type>(valueIn)); },                                                         \
        [this](const std::any& comparisonValue) { return (std::any_cast<Type>(comparisonValue) == std::any_cast<Type>(Getter())); });         \

        #define AURORA_REGISTER_ATTRIBUTE_VALUE_SET(Value, Setter, Type) RegisterAttribute(                                                \
        [this]() { return Value; },                                                                                                        \
        [this](const std::any& valueIn) { Setter(std::any_cast<Type>(valueIn)); },                                                         \
        [this](const std::any& comparisonValue) { return (std::any_cast<Type>(comparisonValue) == std::any_cast<Type>(Value)); });         \

        #define AURORA_REGISTER_ATTRIBUTE_VALUE_VALUE(Value, Type) RegisterAttribute(                                               \
        [this]() { return Value; },                                                                                                 \
        [this](const std::any& valueIn) { Value = std::any_cast<Type>(valueIn); },                                                  \
        [this](const std::any& comparisonValue) { return (std::any_cast<Type>(comparisonValue) == std::any_cast<Type>(Value)); });  \
        
        // Registers an attribute.
        void RegisterAttribute(std::function<std::any()>&& Getter, std::function<void(std::any)>&& Setter, std::function<bool(std::any)>&& Comparison)
        {
            ComponentAttribute attribute;
            attribute.Getter = std::move(Getter);
            attribute.Setter = std::move(Setter);
            attribute.Comparison = std::move(Comparison);
            m_ComponentAttributes.emplace_back(attribute);
        }

        // The type of our component.
        ComponentType m_Type = ComponentType::Unknown;

        // The state of our component.
        bool m_IsEnabled = false;

        // The owner of the component.
        Entity* m_Entity = nullptr;

    private:
        std::vector<ComponentAttribute> m_ComponentAttributes;
    };
}