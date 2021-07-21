#pragma once
#include "IComponent.h"
#include <DirectXMath.h>
#include "Transform.h"

using namespace DirectX;

namespace Aurora
{
    class Camera : public IComponent
    {
    public:
        Camera(EngineContext* engineContext, Entity* entity, uint32_t componentID = 0);
        ~Camera();

        // ================
        void Initialize() override;
        void Tick(float deltaTime) override;
        void TransformCamera(Transform* transform);
        void CreatePerspective(float newWidth, float newHeight, float newNear, float newFar, float newFOV);
        // ================

        void FPSControl(float deltaTime);
        void AdjustRotation(float x, float y, float z);

        XMVECTOR GetEye() const { return XMLoadFloat3(&m_Eye); }
        XMVECTOR GetAt() const { return XMLoadFloat3(&m_At); }
        XMVECTOR GetUp() const { return XMLoadFloat3(&m_Up); }
        XMMATRIX GetView() const { return XMLoadFloat4x4(&m_View); }
        XMMATRIX GetProjection() const { return XMLoadFloat4x4(&m_Projection); }
        XMMATRIX GetViewProjection() const { return XMLoadFloat4x4(&m_VP); }
       
        // Default Values
        const XMVECTOR m_UpVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        const XMVECTOR m_ForwardVector = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        const XMVECTOR m_BackwardVector = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
        const XMVECTOR m_DownVector = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
        const XMVECTOR m_LeftVector = XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f);
        const XMVECTOR m_RightVector = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

    private:
        //=======================================

        XMFLOAT3 m_Eye = XMFLOAT3(0, 0, 0);
        XMFLOAT3 m_At = XMFLOAT3(0, 0, 1);
        XMFLOAT3 m_Up = XMFLOAT3(0, 1, 0);
        XMFLOAT4X4 m_View, m_Projection, m_VP;
        float m_Width = 0.0f;
        float m_Height = 0.0f;
        float m_zNearPlane = 0.1f;
        float m_zFarPlane = 800.0f;
        float m_FOV = XM_PI / 3.0f;

        //=======================================

        // Camera Properties
        float m_Speed = 3.0f;

        bool m_FPS_Control = false;
    };
}