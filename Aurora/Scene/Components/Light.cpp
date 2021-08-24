#include "Aurora.h"
#include "Light.h"
#include "Light.h"

namespace Aurora
{
    Light::Light(EngineContext* engineContext, Entity* entity, uint32_t componentID) : IComponent(engineContext, entity, componentID)
    {
        m_Type = ComponentType::Light;

        AURORA_REGISTER_ATTRIBUTE_VALUE_VALUE(m_Color, XMFLOAT3);
        AURORA_REGISTER_ATTRIBUTE_VALUE_VALUE(m_Intensity, float);
        AURORA_REGISTER_ATTRIBUTE_GET_SET(IsCastingShadow, SetIsCastingShadow, bool);
    }

    Light::~Light()
    {

    }

    void Light::Serialize(BinarySerializer* binarySerializer)
    {
        binarySerializer->Write(m_Color);
        binarySerializer->Write(m_Intensity);
        binarySerializer->Write(IsCastingShadow());
    }

    void Light::Deserialize(BinarySerializer* binaryDeserializer)
    {
        binaryDeserializer->Read(&m_Color);
        binaryDeserializer->Read(&m_Intensity);
        SetIsCastingShadow(binaryDeserializer->ReadAs<bool>());
    }
}