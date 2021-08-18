#include "Aurora.h"
#include "Physics.h"
#include "PhysicsUtilities.h"
#include "PhysicsDebugDraw.h"
#include "../Renderer/Renderer.h"
#include "../Scene/World.h"

namespace Aurora
{
    std::vector<btVector3> g_Collisions;

    Physics::Physics(EngineContext* engineContext) : ISubsystem(engineContext)
    {

    }

    Physics::~Physics()
    {

    }

    bool Physics::Initialize()
    {
        m_CollisionDispatcher = std::make_unique<btCollisionDispatcher>(&m_CollisionConfiguration);
        m_PhysicsWorld = std::make_unique<btDiscreteDynamicsWorld>(m_CollisionDispatcher.get(), &m_OverlappingPairCache, &m_Solver, &m_CollisionConfiguration);

        // Enabled debug drawing.
        m_DebugDrawSystem = std::make_unique<PhysicsDebugDraw>(m_EngineContext->GetSubsystem<Renderer>());
        if (m_PhysicsWorld)
        {
            m_PhysicsWorld->setDebugDrawer(m_DebugDrawSystem.get());
        }

        m_PhysicsWorld->setGravity(btVector3(0, -10, 0));

        AURORA_INFO(LogLayer::Physics, "Initialized Physics Engine.");
        return true;
    }

    void Physics::Tick(float deltaTime)
    {
        if (!GetPhysicsEnabled() || deltaTime <= 0 || !m_PhysicsWorld)
        {
            return;
        }

        Stopwatch physicsProfiler("Physics");

        /// Render option to enable or disable drawing of physics debugging systems.
        //if (true)
        //{
        m_PhysicsWorld->debugDrawWorld();
        //}

        // Perform internal simulation step.
        if (GetPhysicsSimulationEnabled())
        {
            // AURORA_INFO(LogLayer::Physics, "Running Physics!");
            m_PhysicsWorld->stepSimulation(deltaTime, m_Accuracy);
        }
    }

    void Physics::AddRigidBody(btRigidBody* rigidBodyInternal) const
    {
        if (!m_PhysicsWorld)
        {
            return;
        }

        m_PhysicsWorld->addRigidBody(rigidBodyInternal);
    }

    void Physics::RemoveRigidBody(btRigidBody*& rigidBodyInternal) const
    {
        if (!m_PhysicsWorld)
        {
            return;
        }

        m_PhysicsWorld->removeRigidBody(rigidBodyInternal);
        delete rigidBodyInternal->getMotionState();

        AURORA_POINTER_DELETE(rigidBodyInternal);
    }

    XMFLOAT3 Physics::GetGravity() const
    {
        btVector3 gravity = m_PhysicsWorld->getGravity();
        if (!gravity)
        {
            AURORA_ERROR(LogLayer::Physics, "Unable to get gravity from the physics engine. Do ensure that it was properly initialized.");
            return XMFLOAT3(0.0f, 0.0f, 0.0f);
        }

        return gravity ? ToVector3(gravity) : XMFLOAT3(0.0f, 0.0f, 0.0f);
    }
}