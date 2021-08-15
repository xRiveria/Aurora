#include "Aurora.h"
#include "Physics.h"
#include "../Scene/World.h"

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
        m_CollisionDispatcher = std::make_unique<btCollisionDispatcher>(&m_CollisionConfiguration);
        m_PhysicsWorld = std::make_unique<btDiscreteDynamicsWorld>(m_CollisionDispatcher.get(), m_OverlappingPairCache, &m_Solver, &m_CollisionConfiguration);

        m_PhysicsWorld->setGravity(btVector3(0, -10, 0));

        AURORA_INFO(LogLayer::Physics, "Initialized Physics Engine.");
        return true;
    }

    void Physics::Tick(float deltaTime)
    {
        if (!GetPhysicsEnabled() || deltaTime <= 0)
        {
            return;
        }

        Stopwatch physicsProfiler("Physics");

        // Our system will register rigidbodies to objects and update the physics engine state for kinematics.
        auto& rigidbodyEntities = m_EngineContext->GetSubsystem<World>()->GetEntitiesByComponent(ComponentType::RigidBody);

        for (uint32_t i = 0; i < static_cast<uint32_t>(rigidbodyEntities.size()); i++)
        {
            RigidBody* rigidBodyComponent = rigidbodyEntities[i]->GetComponent<RigidBody>();
            Entity* entity = rigidbodyEntities[i].get();

            if (rigidBodyComponent->m_PhysicsInternal == nullptr)
            {

            }
        }
    }

    void Physics::ApplyForce(const RigidBody* rigidBodyComponent, const XMFLOAT3& force)
    {
    }

    void Physics::ApplyForceAt(const RigidBody* rigidBodyComponent, const XMFLOAT3& force, const XMFLOAT3& atPosition)
    {
    }

    void Physics::ApplyImpulse(const RigidBody* rigidBodyComponent, const XMFLOAT3& impulse)
    {
    }

    void Physics::ApplyImpulseAt(const RigidBody* rigidBodyComponent, const XMFLOAT3& impulse, const XMFLOAT3& atPosition)
    {
    }
}