#include "Aurora.h"
#include "Physics.h"

namespace Aurora
{
    Physics::Physics(EngineContext* engineContext) : ISubsystem(engineContext)
    {

    }

    Physics::~Physics()
    {

    }

    bool Physics::Initialize()
    {
        AURORA_INFO(LogLayer::Physics, "Initialized Physics Engine.");
        return true;
    }
}