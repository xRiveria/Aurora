#pragma once
#include "IComponent.h"
#include "../Math/Vector3.h"

class btCollisionShape;

namespace Aurora
{
    enum ColliderShape
    {
        ColliderShape_Box,
        ColliderShape_Sphere,
        ColliderShape_StaticPlane,
        ColliderShape_Cylinder,
        ColliderShape_Capsule,
        ColliderShape_Cone,
        ColliderShape_Mesh
    };

    class Collider : public IComponent
    {
    public:
        Collider(EngineContext* engineContext, Entity* entity, uint32_t componentID = 0);
        ~Collider() = default;

        void Initialize() override;
        void Remove() override;

        // Bounding Box
        const XMFLOAT3& GetBoundingBox() const { return m_Size; }
        void SetBoundingBox(const XMFLOAT3& boundingBox);

        // Collider Center
        const XMFLOAT3& GetCenter() const { return m_Center; }
        void SetCenter(const XMFLOAT3& center);

        // Collision Shape Type
        ColliderShape GetShapeType() const { return m_ShapeType; }
        void SetShapeType(ColliderShape colliderShape);
        std::string GetColliderShapeToString() const; // For our editor.

        // Collision Shpae Internal
        const auto& GetShapeInternal() const { return m_ShapeInternal; }
        
        // Optimize
        bool GetOptimizationState() const { return m_IsOptimizationEnabled; }
        void SetOptimizationState(bool optimizationState);

    private:
        void Shape_Update();
        void Shape_Release();
        void RigidBody_SetShape(btCollisionShape* shapeInternal) const;
        void RigidBody_SetCenterOfMass(const XMFLOAT3& center) const;

    private:
        ColliderShape m_ShapeType = ColliderShape::ColliderShape_Box;
        btCollisionShape* m_ShapeInternal = nullptr;

        XMFLOAT3 m_Size = XMFLOAT3(1.0f, 1.0f, 1.0f);
        XMFLOAT3 m_Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
        uint32_t m_VertexLimit = 100000;
        bool m_IsOptimizationEnabled = true;
    };
}