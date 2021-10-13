#include "Aurora.h"
#include "Light.h"
#include "../Renderer/Renderer.h"
#include "../Physics/PhysicsUtilities.h"

namespace Aurora
{
    Light::Light(EngineContext* engineContext, Entity* entity, uint32_t componentID) : IComponent(engineContext, entity, componentID)
    {
        m_Type = ComponentType::Light;

        // AURORA_REGISTER_ATTRIBUTE_VALUE_VALUE(m_Color, XMFLOAT3);
        AURORA_REGISTER_ATTRIBUTE_VALUE_VALUE(m_Intensity, float);
        AURORA_REGISTER_ATTRIBUTE_GET_SET(IsCastingShadow, SetIsCastingShadow, bool);

        m_Renderer = m_EngineContext->GetSubsystem<Renderer>();
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

    void Light::Tick(float deltaTime)
    {
        AURORA_ASSERT(m_Renderer != nullptr);

        if (!m_IsInitialized)
        {
            CreateShadowMap();
            m_IsInitialized = true;
        }

        /// Dirty checks. For now, we will recompute them every frame.
    }


    void Light::ComputeViewMatrix()
    {

    }

    void Light::CreateShadowMap()
    {

    }

    void Light::ComputeCascadeSplits()
    {
        if (m_ShadowMap.m_Slices.empty())
        {
            return;
        }

        if (!m_Renderer->GetCamera())
        {
            return;
        }

        Camera* camera = m_Renderer->GetCamera();
    }
}