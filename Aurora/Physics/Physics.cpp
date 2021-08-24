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
        m_PhysicsWorld->getSolverInfo().m_splitImpulse = true;
        m_PhysicsWorld->getDispatchInfo().m_useContinuous = true;
        m_PhysicsWorld->getSolverInfo().m_numIterations = m_MaxSolveIterations;

        AURORA_INFO(LogLayer::Physics, "Initialized Physics Engine.");
        return true;
    }

    void Physics::Tick(float deltaTime)
    {
        if (!m_PhysicsWorld)
        {
            return;
        }

        Stopwatch physicsProfiler("Physics", true);

        /// Render option to enable or disable drawing of physics debugging systems.
        //if (true)
        //{
        m_PhysicsWorld->debugDrawWorld();
        //}

        // Don't simulate physics if they are turned off or if we are in the editor mode.
        if (!m_EngineContext->GetEngine()->EngineFlag_IsSet(EngineFlag::EngineFlag_TickGame) || !m_EngineContext->GetEngine()->EngineFlag_IsSet(EngineFlag::EngineFlag_TickPhysics))
        {
            return;
        }
       
        // This equation must be met: timeStep < maxSubSteps * fixedTimeStep
        double internalTimeStep = 1.0 / m_InternalFPS;
        auto maxSubsteps = static_cast<int>(deltaTime * m_InternalFPS) + 1;
        if (m_MaxSubsteps < 0)
        {
            internalTimeStep = deltaTime;
            maxSubsteps = 1;
        }
        else if (m_MaxSubsteps > 0)
        {
            maxSubsteps = XMMin(maxSubsteps, m_MaxSubsteps);
        }
        
        // Step the physics world.
        m_IsSimulationEnabled = true;
        m_PhysicsWorld->stepSimulation(deltaTime, maxSubsteps, internalTimeStep);    
        m_IsSimulationEnabled = false;
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