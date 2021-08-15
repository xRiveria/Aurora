#pragma once
#include "EngineContext.h"
#include "ISubsystem.h"
#include "Bullet/btBulletDynamicsCommon.h"

namespace Aurora
{
    class Physics : public ISubsystem
    {
    public:
        Physics(EngineContext* engineContext);
        ~Physics();

        bool Initialize() override;

    private:
        std::unique_ptr<btCollisionDispatcher> m_CollisionDispatcher;
        std::unique_ptr<btDynamicsWorld> m_DynamicsWorld;
    };
}