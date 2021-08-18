#include "Aurora.h"
#include "Collider.h"
#include "../Physics/Physics.h"
#include "../Physics/PhysicsUtilities.h"
#include "../Math/MathUtilities.h"

using namespace Aurora::Math;

namespace Aurora
{
    Collider::Collider(EngineContext* engineContext, Entity* entity, uint32_t componentID) : IComponent(engineContext, entity, componentID)
    {
        m_ShapeType = ColliderShape::ColliderShape_Box;
        m_Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
        m_Size = XMFLOAT3(1.0f, 1.0f, 1.0f);
        m_ShapeInternal = nullptr;
    }

    void Collider::Initialize()
    {
        /// If there is a mesh, use its bounding box.

        Shape_Update();
    }

    void Collider::Remove()
    {
        Shape_Release();
    }

    void Collider::SetBoundingBox(const XMFLOAT3& boundingBox)
    {
        if (m_Size == boundingBox)
        {
            return;
        }

        m_Size = boundingBox;

        m_Size.x = Helper::Clamp(m_Size.x, Helper::Epsilon, Helper::Infinity);
        m_Size.y = Helper::Clamp(m_Size.y, Helper::Epsilon, Helper::Infinity);
        m_Size.z = Helper::Clamp(m_Size.z, Helper::Epsilon, Helper::Infinity);

        Shape_Update();
    }

    void Collider::SetCenter(const XMFLOAT3& center)
    {
        if (m_Center == center)
        {
            return;
        }

        m_Center = center;
        RigidBody_SetCenterOfMass(center);
    }

    void Collider::SetShapeType(ColliderShape colliderShape)
    {
        if (m_ShapeType == colliderShape)
        {
            return;
        }

        m_ShapeType = colliderShape;
        Shape_Update();
    }

    void Collider::SetOptimizationState(bool optimizationState)
    {
        if (m_IsOptimizationEnabled == optimizationState)
        {
            return;
        }

        m_IsOptimizationEnabled = optimizationState;
        Shape_Update();
    }

    void Collider::Shape_Update()
    {
        Shape_Release();
        const XMFLOAT3 worldScale = GetEntity()->GetTransform()->GetScale();

        switch (m_ShapeType)
        {
            case ColliderShape::ColliderShape_Box:
                m_ShapeInternal = new btBoxShape(ToBulletVector3(XMFLOAT3(m_Size.x * 0.5f, m_Size.y * 0.5f, m_Size.z * 0.5f)));
                m_ShapeInternal->setLocalScaling(ToBulletVector3(worldScale));
                break;

            case ColliderShape::ColliderShape_Sphere:
                m_ShapeInternal = new btSphereShape(m_Size.x * 0.5f);
                m_ShapeInternal->setLocalScaling(ToBulletVector3(worldScale));
                break;

            case ColliderShape::ColliderShape_StaticPlane:
                m_ShapeInternal = new btStaticPlaneShape(btVector3(0.0f, 1.0f, 0.0f), 0.0f);
                break;

            case ColliderShape::ColliderShape_Cylinder:
                AURORA_WARNING(LogLayer::Physics, "Cylinder colliders are not supported yet.");
                break;

            case ColliderShape::ColliderShape_Capsule:
                AURORA_WARNING(LogLayer::Physics, "Capsule colliders are not supported yet.");
                break;

            case ColliderShape::ColliderShape_Cone:
                AURORA_WARNING(LogLayer::Physics, "Cone colliders are not supported yet.");
                break;

            case ColliderShape::ColliderShape_Mesh:
                AURORA_WARNING(LogLayer::Physics, "Mesh colliders are not supported yet.");
                break;;
        }

        m_ShapeInternal->setUserPointer(this);

        RigidBody_SetShape(m_ShapeInternal);
        RigidBody_SetCenterOfMass(m_Center);
    }

    void Collider::Shape_Release()
    {
        RigidBody_SetShape(nullptr);
        AURORA_POINTER_DELETE(m_ShapeInternal);
    }

    void Collider::RigidBody_SetShape(btCollisionShape* shapeInternal) const
    {
        if (const auto& rigidBody = m_Entity->GetComponent<RigidBody>())
        {
            rigidBody->SetColliderShape(shapeInternal);
        }
    }

    void Collider::RigidBody_SetCenterOfMass(const XMFLOAT3& center) const
    {
        if (const auto& rigidBody = m_Entity->GetComponent<RigidBody>())
        {
            rigidBody->SetCenterOfMass(center);
        }
    }

    std::string Collider::GetColliderShapeToString() const
    {
        switch (m_ShapeType)
        {
        case ColliderShape::ColliderShape_Box:
            return "Box";

        case ColliderShape::ColliderShape_Capsule:
            return "Capsule";

        case ColliderShape::ColliderShape_Cone:
            return "Cone";

        case ColliderShape::ColliderShape_Cylinder:
            return "Cylinder";

        case ColliderShape::ColliderShape_Mesh:
            return "Mesh";

        case ColliderShape::ColliderShape_Sphere:
            return "Sphere";

        case ColliderShape::ColliderShape_StaticPlane:
            return "Static Plane";
        }

        return "Unidentified Shape";
    }
}