#include "Aurora.h"
#include "Light.h"
#include "Light.h"

namespace Aurora
{
    Light::Light(EngineContext* engineContext, Entity* entity, uint32_t componentID) : IComponent(engineContext, entity, componentID)
    {

    }

    Light::~Light()
    {

    }

    void Light::Serialize(SerializationStream& outputStream)
    {
        outputStream << YAML::Key << "Light Component";
        outputStream << YAML::BeginMap;

        outputStream << YAML::Key << "Light Color" << YAML::Value << m_Color;
        outputStream << YAML::Key << "Light Intensity" << YAML::Value << m_Intensity;
        outputStream << YAML::Key << "Cast Shadows" << YAML::Value << IsCastingShadow();

        outputStream << YAML::EndMap;
    }
}