#pragma once
#include "IComponent.h"
#include <DirectXMath.h>
#include <vector>

using namespace DirectX;
static const XMFLOAT4X4 IdentityMatrix = XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

namespace Aurora
{
    enum Transform_Flags
    {
        Transform_Flag_Empty = 0,
        Transform_Flag_Dirty = 1 << 0
    };

    class Transform : public IComponent
    {
    public:
        Transform(EngineContext* engineContext, Entity* entity, uint32_t componentID = 0);
        ~Transform() = default;

        void Tick(float deltaTime) override;
        void Serialize(SerializationStream& outputStream) override;

        void UpdateTransform();

        void SetDirty(bool value = true) { if (value) { m_Flags |= Transform_Flags::Transform_Flag_Dirty; } else { m_Flags &= ~Transform_Flags::Transform_Flag_Dirty; } }
        bool IsDirty() const { return m_Flags & Transform_Flags::Transform_Flag_Dirty; }

        // Setters
        void Translate(const XMFLOAT3& value);
        void RotateRollPitchYaw(const XMFLOAT3& value);
        void Rotate(const XMFLOAT4& value);
        void RotateAngles(const XMFLOAT3& value);
        void Scale(const XMFLOAT3& value);

        // Getters
        XMFLOAT3 GetPosition() const;
        XMFLOAT4 GetRotation() const;
        XMFLOAT3 GetScale() const;
        XMVECTOR GetPositionVector() const;
        XMVECTOR GetRotationVector() const;
        XMVECTOR GetScaleVector() const;
        XMMATRIX GetLocalMatrix() const;    // Computes the local space matrix from scale, rotation and translation.

        // ==== Hierarchy ====
        void BecomeOrphan();

        Transform* GetRootTransform() { return HasParentTransform() ? GetParentTransform()->GetRootTransform() : this; }
        bool IsRootTransform() const { return !HasParentTransform(); }
        void SetParentTransform(Transform* newParent);
        bool HasParentTransform() const { return m_ParentTransform != nullptr; }        
        Transform* GetParentTransform() const { return m_ParentTransform; }

        void AddChildTransform(Transform* childTransform);
        bool HasChildren() const { return GetChildrenCount() > 0 ? true : false; }
        uint32_t GetChildrenCount() const { return static_cast<uint32_t>(m_Children.size()); }

        Transform* GetChildByIndex(uint32_t childIndex);
        Transform* GetChildByName(const std::string& childName);
        const std::vector<Transform*>& GetChildren() const { return m_Children; }

        void AcquireChildren();
        bool IsDescendantOf(const Transform* transform) const;
        void GetDescendants(std::vector<Transform*>* descendants);
        
        const XMMATRIX& GetWorldMatrix() const { return XMLoadFloat4x4(&m_WorldMatrix); }

    private:
        XMMATRIX GetParentTransformMatrix() const;

    public:
        //Hierarchy
        Transform* m_ParentTransform; // The parent of this transform.
        std::vector<Transform*> m_Children; // The children of this transform.

        XMFLOAT3 m_TranslationLocal = XMFLOAT3(0, 0, 0);
        XMFLOAT3 m_ScaleLocal = XMFLOAT3(1, 1, 1);
        XMFLOAT4 m_RotationLocal = XMFLOAT4(0.0, 0.0, 0.0, 1.0);    // Quaternion

        XMFLOAT3 m_RotationAngles = XMFLOAT3(0.0f, 0.0f, 0.0f);

        // The world matrix can be computed from the local scale, rotation and translation. This can be done through UpdateTransform() or by calling SetDirty() and letting the Tick system handle the updating.
        XMFLOAT4X4 m_WorldMatrix = IdentityMatrix; // World, relative to its parents.
        XMFLOAT4X4 m_LocalMatrix = IdentityMatrix; // Local.

        uint32_t m_Flags = Transform_Flags::Transform_Flag_Dirty;
    };
}