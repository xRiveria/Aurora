#pragma once
#include "IComponent.h"
#include <DirectXMath.h>

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

        void Tick(float deltaTime) override;
        void UpdateTransform();

        void SetDirty(bool value = true) { if (value) { m_Flags |= Transform_Flags::Transform_Flag_Dirty; } else { m_Flags &= ~Transform_Flags::Transform_Flag_Dirty; } }
        bool IsDirty() const { return m_Flags & Transform_Flags::Transform_Flag_Dirty; }

        // Setters
        void Translate(const XMFLOAT3& value);
        void RotateRollPitchYaw(const XMFLOAT3& value);
        void Rotate(const XMFLOAT4& value);
        void Scale(const XMFLOAT3& value);

        // Getters
        XMFLOAT3 GetPosition() const;
        XMFLOAT4 GetRotation() const;
        XMFLOAT3 GetScale() const;
        XMVECTOR GetPositionVector() const;
        XMVECTOR GetRotationVector() const;
        XMVECTOR GetScaleVector() const;
        XMMATRIX GetLocalMatrix() const;    // Computes the local space matrix from scale, rotation and translation.
 
    public:
        XMFLOAT3 m_TranslationLocal = XMFLOAT3(0, 0, 0);
        XMFLOAT3 m_ScaleLocal = XMFLOAT3(1, 1, 1);
        XMFLOAT4 m_RotationLocal = XMFLOAT4(0.0, 0.0, 0.0, 1.0);    // Quaternion

        // The world matrix can be computed from the local scale, rotation and translation. This can be done through UpdateTransform() or by calling SetDirty() and letting the Tick system handle the updating.
        XMFLOAT4X4 m_WorldMatrix = IdentityMatrix;

        uint32_t m_Flags = Transform_Flags::Transform_Flag_Dirty;
    };
}