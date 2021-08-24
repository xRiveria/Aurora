#pragma once
#include "IComponent.h"
#include <DirectXMath.h>
#include "../Math/Vector3.h"
#include "../Math/Quaternion.h"

using namespace DirectX;
class btRigidBody;
class btCollisionShape;

namespace Aurora
{
    class Physics;

    enum RigidBodyFlags
    {
        RigidBodyFlag_Empty                = 0,
        RigidBodyFlag_Kinematic            = 1,
        RigidBodyFlag_GravityAffected      = 2
    };

    enum ForceMode
    {
        ForceMode_Force,
        ForceMode_Impulse
    };

    class RigidBody : public IComponent
    {
    public:
        RigidBody(EngineContext* engineContext, Entity* entity, uint32_t componentID = 0);
        ~RigidBody();

        void Initialize() override;
        void Remove() override;
        void Start() override;
        void Tick(float deltaTime) override;

        // Serialization
        void Serialize(BinarySerializer* binarySerializer) override;
        void Deserialize(BinarySerializer* binaryDeserializer) override;
            
        // Mass
        float GetMass() const { return m_Mass; }
        void SetMass(float mass);

        // Friction (Drag)
        float GetFriction() const { return m_Friction; }
        void SetFriction(float friction);

        // Restitution
        float GetRestitution() const { return m_Restitution; }
        void SetRestitution(float restitution);

        // Angular Drag
        float GetFrictionRolling() const { return m_FrictionRolling; }
        void SetFrictionRolling(float frictionRolling);

        // Gravity
        void SetGravityState(bool gravityState);
        bool GetGravityState() const { return m_RigidBodyFlags & RigidBodyFlags::RigidBodyFlag_GravityAffected; }

        XMFLOAT3 GetGravity() const { return m_Gravity; }
        void SetGravity(const XMFLOAT3& acceleration);

        // Kinematic
        void SetKinematicState(bool kinematicState);
        bool GetKinematicState() const { return m_RigidBodyFlags & RigidBodyFlags::RigidBodyFlag_Kinematic; }

        // Velocity/Force/Torque
        void SetLinearVelocity(const XMFLOAT3& velocity, const bool activate = true) const;
        void SetAngularVelocity(const XMFLOAT3& velocity, const bool activate = true) const;

        void ApplyForce(const XMFLOAT3& forceAmount, ForceMode forceMode);
        void ApplyForceAtPosition(const XMFLOAT3& forceAmount, const XMFLOAT3& atPosition, ForceMode forceMode) const;
        void ApplyTorque(const XMFLOAT3& torque, ForceMode forceMode) const;

        // Position / Lock
        XMFLOAT3 GetPosition() const;
        void SetPosition(const XMFLOAT3& position, const bool activate = true) const;

        // void SetPositionLock(bool lockState);
        // void SetPositionLock(const XMFLOAT3& lockVector);
        // XMFLOAT3 GetPositionLock() const { return m_PositionLock; }

        // Rotation / Lock
        // XMFLOAT4 GetRotation() const;
        // void SetRotation(const XMFLOAT4& rotation, const bool activate = true) const;

        // void SetRotationLock(bool lockState);
        // void SetRotationLock(const XMFLOAT3& lockVector);
        // XMFLOAT3 GetRotationLock() const { return m_RotationLock; }
        
        // Center of Mass
        void SetCenterOfMass(const XMFLOAT3& centerOfMass);
        const XMFLOAT3& GetCenterOfMass() const { return m_CenterOfMass; }

        // Communication with other Physics components.
        void SetColliderShape(btCollisionShape* shapeInternal);

        // Misc
        void ClearForces() const;
        void Activate() const;
        void Deactivate() const;

    private:
        void RigidBody_AddToWorld();
        void RigidBody_Release();
        void RigidBody_RemoveFromWorld();
        void RigidBody_AcquireShape();
        void RigidBody_UpdateKinematic();
        void RigidBody_UpdateGravity();
        bool GetActivationState() const;

    public:
        btRigidBody* m_RigidBodyInternal = nullptr;
        btCollisionShape* m_CollisionShapeInternal = nullptr;

    private:
        float m_Mass = 1.0f;
        float m_Friction = 0.5f;       // Essentially damping, but only applies when two bodies are touching instead of all the time like linear and angular damping.
        float m_FrictionRolling = 0.0f;

        float m_Restitution = 0.0f;    // Bounciness. Higher restitution makes a body bounce more. No restitution gives no added bouncing.
        // float m_DampingLinear = 0.0f;  // Affects how the body moves througfh the world in any given direction. Having this at zero means the object keeps moving until friction slowers them down. At higher values, they would slow down even if they don't touch anything.
        // float m_DampingAngular = 0.0f; // Angular damping is similar, but applies to angular motion (ie. rotation).
        uint32_t m_RigidBodyFlags = 0;

        XMFLOAT3 m_Gravity;
        XMFLOAT3 m_CenterOfMass;
        XMFLOAT3 m_PositionLock;
        XMFLOAT3 m_RotationLock;

        bool m_IsInPhysicsWorld = false;

        Physics* m_PhysicsSystem = nullptr;
    };
}