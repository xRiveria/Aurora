#pragma once
#include "EngineContext.h"
#include "ISubsystem.h"
#include "../Scene/Components/Transform.h"
#include "../Scene/Components/RigidBody.h"
#include "Bullet/btBulletDynamicsCommon.h"

namespace Aurora
{
    class PhysicsDebugDraw;

    class Physics : public ISubsystem
    {
    public:
        Physics(EngineContext* engineContext);
        ~Physics();

        bool Initialize() override;
        void Tick(float deltaTime) override;

        // Enable or disable the physics simulation.
        void SetPhysicsSimulationEnabled(bool value) { m_IsSimulationEnabled = value; }
        bool GetPhysicsSimulationEnabled() const { return m_IsSimulationEnabled; }

        // RigidBody
        void AddRigidBody(btRigidBody* rigidBodyInternal) const;
        void RemoveRigidBody(btRigidBody*& rigidBodyInternal) const;

        /// Soft Bodies
        /// Constraints

        // Properties
        XMFLOAT3 GetGravity() const;
        PhysicsDebugDraw* GetDebugDrawSystem() const { return m_DebugDrawSystem.get(); }

    private:
        btDefaultCollisionConfiguration m_CollisionConfiguration;
        btDbvtBroadphase m_OverlappingPairCache;
        btSequentialImpulseConstraintSolver m_Solver;

        std::unique_ptr<btDiscreteDynamicsWorld> m_PhysicsWorld = nullptr;
        std::unique_ptr<btCollisionDispatcher> m_CollisionDispatcher = nullptr;
        std::unique_ptr<PhysicsDebugDraw> m_DebugDrawSystem = nullptr;

    private:
        bool m_IsSimulationEnabled = true;
        

        // Properties
        int m_MaxSubsteps = 1;
        int m_MaxSolveIterations = 256;
        float m_InternalFPS = 60.0f;
    };
}

/*
    - Force Mode: Applies a gradual force on the object, taking mass into account. This is a literal pushing motion where the bigger the mass of an object, the slower it will speed up.
      : Force = Vector3.forward * 1.0f * Time.fixedDeltaTime / (rigidbody.mass);

    - Impulse Mode: Applies an instant force on the object, taking mass into account, This pushes the object using the entire force in a single frame, affected by the mass of the object. Perfect for jumps/recoil.
      : Force = Vector3.forward * 1.0f / rigidbody.mass;

    - Acceleration Mode: Same as Force Mode, expect no mass is taken into account. It will always accelerate at a constant rate.
      : Force = Vector3.forward *1.0f * Time.fixedDeltaTime;

    - VelocityChange Mode: Same as Impulse Mode and doesn't take mass into account. It will add the force to the object's velocity in a single frame.
      : Force = Vector3.forward * 1.0f;

    - applyCentralImpulse applies the impulse directly to the box's center of gravity, turning 100% of the impulse into linear velocity. applyImpulse allows you to specify a point in space where the 
    impulse is applied, such as an edge or corner, which will convert the impulse into both linear and angular velocity, so it will move through space and also rotate.
*/