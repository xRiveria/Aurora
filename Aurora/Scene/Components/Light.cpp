#include "Aurora.h"
#include "Light.h"
#include "Light.h"

namespace Aurora
{
    Light::Light(EngineContext* engineContext, Entity* entity, uint32_t componentID) : IComponent(engineContext, entity, componentID)
    {
        m_Type = ComponentType::Light;
    }

    Light::~Light()
    {

    }

    void Light::Serialize(SerializationStream& outputStream)
    {
        outputStream << YAML::Key << "LightComponent";
        outputStream << YAML::BeginMap;

        outputStream << YAML::Key << "LightColor" << YAML::Value << m_Color;
        outputStream << YAML::Key << "LightIntensity" << YAML::Value << m_Intensity;
        outputStream << YAML::Key << "CastShadows" << YAML::Value << IsCastingShadow();

        outputStream << YAML::EndMap;
    }

    void Light::Deserialize(SerializationNode& inputNode)
    {
        m_Color = inputNode["LightColor"].as<XMFLOAT3>();
        m_Intensity = inputNode["LightIntensity"].as<float>();
        SetIsCastingShadow(inputNode["CastShadows"].as<bool>());
    }
}