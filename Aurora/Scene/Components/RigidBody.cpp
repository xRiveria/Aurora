#include "Aurora.h"
#include "RigidBody.h"
#include "Collider.h"
#include "../Physics/Physics.h"
#include "../Physics/PhysicsUtilities.h"

namespace Aurora
{
    static const float g_Default_Mass = 1.0f;
    static const float g_Default_Friction = 0.5f;
    static const float g_Default_FrictionRolling = 0.0f;
    static const float g_Default_Restitution = 0.0f;
    static const float g_Default_DeactivationTime = 2000;

    // btMotionState allows the dynamics world to synchronize and interpolate the updated world transform with graphics. For optimizations, potentially only moving objects get synchronized using setWorldPosition/setWorldOrientation.
    class MotionState : public btMotionState
    {
    public:
        MotionState(RigidBody* rigidBody) { m_RigidBody = rigidBody; }

        // Update from Engine to Bullet.
        void getWorldTransform(btTransform& worldTransform) const override
        {
            const XMFLOAT3 lastPosition = m_RigidBody->GetEntity()->GetTransform()->GetPosition();
            const XMFLOAT4 lastRotation = m_RigidBody->GetEntity()->GetTransform()->GetRotation();

            worldTransform.setOrigin(ToBulletVector3(lastPosition));
            worldTransform.setRotation({ lastRotation.x, lastRotation.y, lastRotation.z, lastRotation.w });
        }

        // Update from Bullet to Engine.
        void setWorldTransform(const btTransform& worldTransform) override
        {
            const XMFLOAT3 newWorldPosition = ToVector3(worldTransform.getOrigin());
            const XMFLOAT4 newWorldRotation = ToVector4(worldTransform.getOrigin());

            m_RigidBody->GetEntity()->GetTransform()->m_TranslationLocal = newWorldPosition;
            m_RigidBody->GetEntity()->GetTransform()->m_RotationLocal = newWorldRotation;
        }

    private:
        RigidBody* m_RigidBody;
    };

    RigidBody::RigidBody(EngineContext* engineContext, Entity* entity, uint32_t componentID) : IComponent(engineContext, entity, componentID)
    {
        m_PhysicsSystem = m_EngineContext->GetSubsystem<Physics>();

        m_IsInPhysicsWorld = false;
        m_Mass = g_Default_Mass;
        m_Restitution = g_Default_Restitution;
        m_Friction = g_Default_Friction;
        m_FrictionRolling = g_Default_FrictionRolling;
        m_RigidBodyFlags |= RigidBodyFlags::RigidBodyFlag_GravityAffected;
        m_Gravity = m_PhysicsSystem->GetGravity();
        m_RigidBodyFlags &= ~RigidBodyFlags::RigidBodyFlag_Kinematic;
        m_PositionLock = XMFLOAT3(0.0f, 0.0f, 0.0f);
        // m_RotationLock = XMFLOAT3(0.0f, 0.0f, 0.0f);
        m_CollisionShapeInternal = nullptr;
        m_RigidBodyInternal = nullptr;
    }

    RigidBody::~RigidBody()
    {
        RigidBody_Release();
    }

    void RigidBody::Initialize()
    {
        RigidBody_AcquireShape();
        RigidBody_AddToWorld();
    }

    void RigidBody::Start()
    {
        Activate();
    }

    void RigidBody::Remove()
    {
        RigidBody_Release();
    }

    void RigidBody::Tick(float deltaTime)
    {
        // When the rigidbody is inactive or we are in editor mode, allow the user to move/rotate it.
        if (!GetActivationState() || !m_EngineContext->GetEngine()->EngineFlag_IsSet(EngineFlag::EngineFlag_TickGame))
        {
            if (GetPosition() != GetEntity()->GetTransform()->GetPosition())
            {
                SetPosition(GetEntity()->GetTransform()->GetPosition());
                SetLinearVelocity(XMFLOAT3(0.0f, 0.0f, 0.0f), false);
                SetAngularVelocity(XMFLOAT3(0.0f, 0.0f, 0.0f), false);
            }

            //if (GetRotation() != GetEntity()->GetTransform()->GetRotation())
            //{
            //    SetRotation(GetEntity()->GetTransform()->GetRotation(), false);
            //    SetLinearVelocity(XMFLOAT3(0.0f, 0.0f, 0.0f), false);
            //    SetAngularVelocity(XMFLOAT3(0.0f, 0.0f, 0.0f), false);
            // }
        }     
    }

    void RigidBody::SetMass(float mass)
    {
        mass = XMMax(mass, 0.0f);

        if (mass != m_Mass)
        {
            m_Mass = mass;
            RigidBody_AddToWorld();
        }
    }

    void RigidBody::SetFriction(float friction)
    {
        if (!m_RigidBodyInternal || m_Friction == friction)
        {
            return;
        }

        m_Friction = friction;
        m_RigidBodyInternal->setFriction(friction);
    }

    void RigidBody::SetRestitution(float restitution)
    {
        if (!m_RigidBodyInternal || m_Restitution == restitution)
        {
            return;
        }

        m_Restitution = restitution;
        m_RigidBodyInternal->setRestitution(restitution);
    }

    void RigidBody::SetFrictionRolling(float frictionRolling)
    {
        if (!m_RigidBodyInternal || m_FrictionRolling == frictionRolling)
        {
            return;
        }

        m_FrictionRolling = frictionRolling;
        m_RigidBodyInternal->setRollingFriction(frictionRolling);
    }

    void RigidBody::SetUseGravity(bool gravityState)
    {
        if (gravityState)
        {
            m_RigidBodyFlags |= RigidBodyFlags::RigidBodyFlag_GravityAffected;
        }
        else
        {
            m_RigidBodyFlags &= ~RigidBodyFlags::RigidBodyFlag_GravityAffected;
        }
         
        RigidBody_AddToWorld();
    }

    void RigidBody::SetGravity(const XMFLOAT3& acceleration)
    {
        if (m_Gravity == acceleration)
        {
            return;
        }

        m_Gravity = acceleration;
        RigidBody_AddToWorld();
    }

    void RigidBody::SetKinematicState(bool kinematicState)
    {
        if (kinematicState)
        {
            m_RigidBodyFlags |= RigidBodyFlags::RigidBodyFlag_Kinematic;
        }
        else
        {
            m_RigidBodyFlags &= ~RigidBodyFlags::RigidBodyFlag_Kinematic;
        }

        RigidBody_AddToWorld();
    }

    void RigidBody::SetLinearVelocity(const XMFLOAT3& velocity, const bool activate) const
    {
        if (!m_RigidBodyInternal)
        {
            return;
        }

        m_RigidBodyInternal->setLinearVelocity(ToBulletVector3(velocity));
        if (velocity != XMFLOAT3(0.0f, 0.0f, 0.0f) && activate)
        {
            Activate();
        }
    }

    void RigidBody::SetAngularVelocity(const XMFLOAT3& velocity, const bool activate) const
    {
        if (!m_RigidBodyInternal)
        {
            return;
        }

        m_RigidBodyInternal->setAngularVelocity(ToBulletVector3(velocity));
        if (velocity != XMFLOAT3(0.0f, 0.0f, 0.0f) && activate)
        {
            Activate();
        }
    }

    void RigidBody::ApplyForce(const XMFLOAT3& forceAmount, ForceMode forceMode)
    {
        if (!m_RigidBodyInternal)
        {
            return;
        }

        Activate();

        if (forceMode == ForceMode::ForceMode_Force)
        {
            m_RigidBodyInternal->applyCentralForce(ToBulletVector3(forceAmount));
        }
        else if (forceMode == ForceMode::ForceMode_Impulse)
        {
            m_RigidBodyInternal->applyCentralImpulse(ToBulletVector3(forceAmount));
        }
    }

    void RigidBody::ApplyForceAtPosition(const XMFLOAT3& forceAmount, const XMFLOAT3& atPosition, ForceMode forceMode) const
    {
        if (!m_RigidBodyInternal)
        {
            return;
        }

        Activate();

        if (forceMode == ForceMode::ForceMode_Force)
        {
            m_RigidBodyInternal->applyForce(ToBulletVector3(forceAmount), ToBulletVector3(atPosition));
        }
        else if (forceMode == ForceMode::ForceMode_Impulse)
        {
            m_RigidBodyInternal->applyImpulse(ToBulletVector3(forceAmount), ToBulletVector3(atPosition));
        }
    }

    void RigidBody::ApplyTorque(const XMFLOAT3& torque, ForceMode forceMode) const
    {
        if (!m_RigidBodyInternal)
        {
            return;
        }

        Activate();

        if (forceMode == ForceMode::ForceMode_Force)
        {
            m_RigidBodyInternal->applyTorque(ToBulletVector3(torque));
        }
        else if (forceMode == ForceMode::ForceMode_Impulse)
        {
            m_RigidBodyInternal->applyTorqueImpulse(ToBulletVector3(torque));
        }
    }

    void RigidBody::SetCenterOfMass(const XMFLOAT3& centerOfMass)
    {
        m_CenterOfMass = centerOfMass;
        SetPosition(GetPosition());
    }

    XMFLOAT3 RigidBody::GetPosition() const
    {
        if (m_RigidBodyInternal)
        {
            const btTransform& transform = m_RigidBodyInternal->getWorldTransform();
            return ToVector3(transform.getOrigin());
        }

        return XMFLOAT3(0.0f, 0.0f, 0.0f);
    }

    void RigidBody::SetPosition(const XMFLOAT3& position, const bool activate) const
    {
        if (!m_RigidBodyInternal)
        {
            return;
        }

        // Set position to world transform.
        btTransform& worldTransform = m_RigidBodyInternal->getWorldTransform();
        worldTransform.setOrigin(ToBulletVector3(position));

        // Set position to interpolated world transform.
        btTransform transformWorldInterpolated = m_RigidBodyInternal->getInterpolationWorldTransform();
        transformWorldInterpolated.setOrigin(worldTransform.getOrigin());
        m_RigidBodyInternal->setInterpolationWorldTransform(transformWorldInterpolated);

        if (activate)
        {
            Activate();
        }
    }

    void RigidBody::ClearForces() const
    {
        if (!m_RigidBodyInternal)
        {
            return;
        }

        m_RigidBodyInternal->clearForces();
    }

    void RigidBody::Activate() const
    {
        if (!m_RigidBodyInternal)
        {
            return;
        }

        if (m_Mass > 0.0f)
        {
            m_RigidBodyInternal->activate(true);
        }
    }

    void RigidBody::Deactivate() const
    {
        if (!m_RigidBodyInternal)
        {
            return;
        }

        m_RigidBodyInternal->setActivationState(false);
    }

    void RigidBody::SetColliderShape(btCollisionShape* shapeInternal)
    {
        m_CollisionShapeInternal = shapeInternal;

        if (m_CollisionShapeInternal)
        {
            RigidBody_AddToWorld();
        }
        else
        {
            RigidBody_RemoveFromWorld();
        }
    }

    void RigidBody::RigidBody_AddToWorld()
    {
        if (m_Mass < 0.0f)
        {
            m_Mass = 0.0f;
        }

        // Transfer inertia to new collision shape.
        btVector3 localInertia = btVector3(0.0f, 0.0f, 0.0f);
        if (m_CollisionShapeInternal && m_RigidBodyInternal)
        {
            localInertia = m_RigidBodyInternal ? m_RigidBodyInternal->getLocalInertia() : localInertia;
            m_CollisionShapeInternal->calculateLocalInertia(m_Mass, localInertia);
        }

        RigidBody_Release();

        // Construction
        {
            // Create a motion state (memory will be freed by the RigidBody).
            const auto& motionState = new MotionState(this);

            // Information
            btRigidBody::btRigidBodyConstructionInfo constructionInfo(m_Mass, motionState, m_CollisionShapeInternal, localInertia);
            constructionInfo.m_mass = m_Mass;
            constructionInfo.m_friction = m_Friction;
            constructionInfo.m_rollingFriction = m_FrictionRolling;
            constructionInfo.m_restitution = m_Restitution;
            constructionInfo.m_collisionShape = m_CollisionShapeInternal;
            constructionInfo.m_localInertia = localInertia;
            constructionInfo.m_motionState = motionState;

            m_RigidBodyInternal = new btRigidBody(constructionInfo);
            m_RigidBodyInternal->setUserPointer(this);
        }

        /// Reapply contraint positions for new center of mass shift.

        RigidBody_UpdateKinematic();
        RigidBody_UpdateGravity();

        // Transform
        SetPosition(GetEntity()->GetTransform()->GetPosition());
        // SetRotation(GetEntity()->GetTransform()->GetRotation());

        /// Constraints

        /// Position and Rotation Locks

        // Add to World
        m_PhysicsSystem->AddRigidBody(m_RigidBodyInternal);
        AURORA_INFO(LogLayer::Physics, "Adding RigidBody to World.");

        if (m_Mass > 0.0f)
        {
            Activate();
        }
        else
        {
            SetLinearVelocity({ 0.0f, 0.0f, 0.0f });
            SetAngularVelocity({ 0.0f, 0.0f, 0.0f });
        }

        m_IsInPhysicsWorld = true;
    }

    void RigidBody::RigidBody_Release()
    {
        if (!m_RigidBodyInternal)
        {
            return;
        }

        /// Release constraints.

        // Remove it from the world.
        RigidBody_RemoveFromWorld();

        // Reset it.
        m_RigidBodyInternal = nullptr;
    }

    void RigidBody::RigidBody_RemoveFromWorld()
    {
        if (!m_RigidBodyInternal)
        {
            return;
        }

        if (m_IsInPhysicsWorld)
        {
            m_PhysicsSystem->RemoveRigidBody(m_RigidBodyInternal);
            m_IsInPhysicsWorld = false;
        }
    }

    void RigidBody::RigidBody_AcquireShape()
    {
        if (const auto& collider = m_Entity->GetComponent<Collider>())
        {
            m_CollisionShapeInternal = collider->GetShapeInternal();
            m_CenterOfMass = collider->GetCenter();
            AURORA_INFO(LogLayer::Physics, "Collider Found!");
        }
    }

    // If an object is Kinematic, it will not be driven by the physics engine, and can only be manipulated by its Transform.
    void RigidBody::RigidBody_UpdateKinematic()
    {
        int rigidBodyFlagsInternal = m_RigidBodyInternal->getCollisionFlags();

        if (GetKinematicState())
        {
            rigidBodyFlagsInternal |= btCollisionObject::CF_KINEMATIC_OBJECT;
        }
        else
        {
            rigidBodyFlagsInternal &= ~btCollisionObject::CF_KINEMATIC_OBJECT;
        }

        m_RigidBodyInternal->setCollisionFlags(rigidBodyFlagsInternal);
        m_RigidBodyInternal->forceActivationState(GetKinematicState() ? DISABLE_DEACTIVATION : ISLAND_SLEEPING);
        m_RigidBodyInternal->setDeactivationTime(g_Default_DeactivationTime);
    }

    void RigidBody::RigidBody_UpdateGravity()
    {
        int rigidBodyFlagsInternal = m_RigidBodyInternal->getFlags();

        if (GetGravityState())
        {
            rigidBodyFlagsInternal &= ~btRigidBodyFlags::BT_DISABLE_WORLD_GRAVITY;
        }
        else
        {
            rigidBodyFlagsInternal |= btRigidBodyFlags::BT_DISABLE_WORLD_GRAVITY;
        }

        m_RigidBodyInternal->setFlags(rigidBodyFlagsInternal);

        if (GetGravityState())
        {
            m_RigidBodyInternal->setGravity(ToBulletVector3(m_Gravity));
        }
        else
        {
            m_RigidBodyInternal->setGravity(btVector3(0.0f, 0.0f, 0.0f));
        }
    }

    bool RigidBody::GetActivationState() const
    {
        return m_RigidBodyInternal->isActive();
    }
}