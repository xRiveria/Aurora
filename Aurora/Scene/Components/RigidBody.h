#pragma once
#include "IComponent.h"
#include <DirectXMath.h>

using namespace DirectX;
namespace Aurora
{
    enum RigidBodyFlags
    {
        RigidBodyFlag_Empty                = 0,
        RigidBodyFlag_Disactivated         = 1,
        RigidBodyFlag_Kinematic            = 2
    };

    enum CollisionShape
    {
        CollisionShape_Empty,
        CollisionShape_Box,
        CollisionShape_Sphere,
        CollisionShape_Capsule,
        CollisionShape_ConvexHull,
        CollisionShape_TriangleMesh
    };

    struct BoxParameters
    {
        XMFLOAT3 m_HalfExtents = XMFLOAT3(1.0f, 1.0f, 1.0f);
    };

    struct SphereParameters
    {
        float m_Radius = 1.0f;
    };

    struct CapsuleParameters
    {
        float m_Radius = 1.0f;
        float m_Height = 1.0f;
    };

    class RigidBody : public IComponent
    {
    public:
        RigidBody(EngineContext* engineContext, Entity* entity, uint32_t componentID = 0);
        ~RigidBody();

        void SetKinematic(bool value)
        {
            if (value)
            {
                m_RigidBodyFlags |= RigidBodyFlags::RigidBodyFlag_Kinematic;
            }
            else
            {
                m_RigidBodyFlags &= ~RigidBodyFlags::RigidBodyFlag_Kinematic;
            }
        }

        void SetDeactivationState(bool value)
        {
            if (value)
            {
                m_RigidBodyFlags |= RigidBodyFlags::RigidBodyFlag_Disactivated;
            }
            else
            {
                m_RigidBodyFlags &= ~RigidBodyFlags::RigidBodyFlag_Disactivated;
            }
        }

        void SetMass(float mass) { m_Mass = mass; }
        void SetFriction(float friction) { m_Friction = friction; }
        void SetRestitution(float restitution) { m_Restitution = restitution; }
        void SetDamplingLinear(float linearDamping) { m_DampingLinear = linearDamping; }
        void SetDamplingAngular(float angularDamping) { m_DampingAngular = angularDamping; }
        void SetCollisionShape(CollisionShape collisionShape) { m_CollisionShape = collisionShape; }

        void* GetPhysicsInternal() const { return m_PhysicsInternal; }
        float GetMass() const { return m_Mass; }
        float GetFriction() const { return m_Friction; }
        float GetRestitution() const { return m_Restitution; }
        float GetDamplingLinear() const { return m_DampingLinear; }
        float GetDamplingAngular() const { return m_DampingAngular; }
        CollisionShape GetCollisionShape() const { return m_CollisionShape; }
        std::string GetCollisionShapeToString() const; // For our editor.
        bool GetDeactivationState() const { return m_RigidBodyFlags & RigidBodyFlags::RigidBodyFlag_Disactivated; }
        bool GetKinematicState() const { return m_RigidBodyFlags & RigidBodyFlags::RigidBodyFlag_Kinematic; }

    public:
        void* m_PhysicsInternal = nullptr;

    private:
        float m_Mass = 1.0f;
        float m_Friction = 0.5f;       // Essentially damping, but only applies when two bodies are touching instead of all the time like linear and angular damping.
        float m_Restitution = 0.0f;    // Bounciness. Higher restitution makes a body bounce more. No restitution gives no added bouncing.
        float m_DampingLinear = 0.0f;  // Affects how the body moves througfh the world in any given direction. Having this at zero means the object keeps moving until friction slowers them down. At higher values, they would slow down even if they don't touch anything.
        float m_DampingAngular = 0.0f; // Angular damping is similar, but applies to angular motion (ie. rotation).

        uint32_t m_RigidBodyFlags = 0;

        CapsuleParameters m_CapsuleParameters;
        SphereParameters m_SphereParameters;
        BoxParameters m_BoxParameters;
        CollisionShape m_CollisionShape = CollisionShape::CollisionShape_Empty;
    };
}