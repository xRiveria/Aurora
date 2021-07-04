#include "Aurora.h"
#include "Light.h"

namespace Aurora
{
    Light::Light(EngineContext* engineContext, Entity* entity, uint32_t componentID) : IComponent(engineContext, entity, componentID)
    {

    }

    Light::~Light()
    {

    }
}