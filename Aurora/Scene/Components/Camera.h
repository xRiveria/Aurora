#pragma once
#include "IComponent.h"
#include <DirectXMath.h>
#include "../Math/XM_Utilities/BoundingBox.h"

using namespace DirectX;

namespace Aurora
{
    class Transform;

    class Camera : public IComponent
    {
    public:
        Camera(EngineContext* engineContext, Entity* entity, uint32_t componentID = 0);
        ~Camera();

        // ================
        void Initialize() override;
        void Tick(float deltaTime) override;
        // ================

        void FPSControl(float deltaTime);

        void ComputePerspectiveMatrix(float fovInDegrees, float aspectRatio, float nearZ, float farZ);
        void ComputeViewMatrix();
        void ComputeLookAtPosition(XMFLOAT3 lookAtPosition);

        const XMMATRIX& GetViewMatrix() const { return m_ViewMatrix; }
        const XMMATRIX& GetProjectionMatrix() const { return m_ProjectionMatrix; }
        const XMMATRIX& GetViewProjectionMatrix() const { return m_ViewMatrix * m_ProjectionMatrix; }
        const XMMATRIX& GetInverseViewProjectMatrix() const {  }

        void SetPosition(float x, float y, float z);
        void SetRotation(float x, float y, float z);
        void AdjustRotation(float x, float y, float z);

        const XMVECTOR& GetCurrentForwardVector() { return m_CurrentForwardVector; }
        const XMVECTOR& GetCurrentBackwardVector() { return m_CurrentBackwardVector; }
        const XMVECTOR& GetCurrentLeftVector() { return m_CurrentLeftVector; }
        const XMVECTOR& GetCurrentRightVector() { return m_CurrentRightVector; }

        XMFLOAT2 Project(XMVECTOR& worldPosition);

    public:
        XMMATRIX m_ViewMatrix;
        XMMATRIX m_ProjectionMatrix;

        // Camera Properties
        float m_Speed = 3.0f;

        // Default Values
        const XMVECTOR m_UpVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        const XMVECTOR m_ForwardVector = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        const XMVECTOR m_BackwardVector = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
        const XMVECTOR m_DownVector = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
        const XMVECTOR m_LeftVector = XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f);
        const XMVECTOR m_RightVector = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

        bool m_FPS_Control = false;
        bool m_IsCameraMoveable = true;

        XMVECTOR m_CurrentForwardVector;
        XMVECTOR m_CurrentLeftVector;
        XMVECTOR m_CurrentRightVector;
        XMVECTOR m_CurrentBackwardVector;

        bool m_IsFirstRun = true;
    };
}