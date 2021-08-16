#pragma once
#include "EngineContext.h"
#include "ISubsystem.h"
#include "../Scene/Components/Transform.h"
#include "../Scene/Components/Mesh.h"
#include "../Scene/Components/RigidBody.h"
#include "Bullet/btBulletDynamicsCommon.h"

namespace Aurora
{
    class Physics : public ISubsystem
    {
    public:
        Physics(EngineContext* engineContext);
        ~Physics();

        bool Initialize() override;
        void Tick(float deltaTime) override;

        // Enable or disable the physics engine altogether.
        void SetPhysicsEnabled(bool value) { m_IsEnabled = value; }
        bool GetPhysicsEnabled() const { return m_IsEnabled; }

        // Enable or disable the physics simulation.
        void SetPhysicsSimulationEnabled(bool value) { m_IsSimulationEnabled = value; }
        bool GetPhysicsSimulationEnabled() const { return m_IsSimulationEnabled; }

        void AddRigidBody(Entity* entity, RigidBody* rigidBodyComponent, const Transform* transformComponent, const Mesh* meshComponent);

        // Apply force at body center.
        void ApplyForce(const RigidBody* rigidBodyComponent, const XMFLOAT3& force);

        // Apply force at body local position.
        void ApplyForceAt(const RigidBody* rigidBodyComponent, const XMFLOAT3& force, const XMFLOAT3& atPosition);

        // Apply impulse at body center.
        void ApplyImpulse(const RigidBody* rigidBodyComponent, const XMFLOAT3& impulse);

        // Apply impulse at body local position.
        void ApplyImpulseAt(const RigidBody* rigidBodyComponent, const XMFLOAT3& impulse, const XMFLOAT3& atPosition);

    private:
        // Collision configuration contains default setup for memory, collision setup. 
        btDefaultCollisionConfiguration m_CollisionConfiguration;
        // btDbvtBroadphase is a good general purpose broadphase. We can also try btAxis3Sweep.
        btDbvtBroadphase m_OverlappingPairCache;
        // The default constraint solver. For parallel processing, we can use a different solver.
        btSequentialImpulseConstraintSolver m_Solver;

        std::unique_ptr<btDiscreteDynamicsWorld> m_PhysicsWorld;
        // Uses the default collision dispatcher. For parallel processing, you can use a different dispatcher.
        std::unique_ptr<btCollisionDispatcher> m_CollisionDispatcher;

    private:
        bool m_IsEnabled = true;
        bool m_IsSimulationEnabled = true;
        int m_Accuracy = 10;
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