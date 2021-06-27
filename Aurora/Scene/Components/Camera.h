#pragma once
#include <DirectXMath.h>
#include "EngineContext.h"

using namespace DirectX;

namespace Aurora
{
    class Camera
    {
    public:
        Camera(EngineContext* engineContext);
        ~Camera();

        void Tick(float deltaTime);
        void FPSControl(float deltaTime);

        void ComputePerspectiveMatrix(float fovInDegrees, float aspectRatio, float nearZ, float farZ);
        void ComputeViewMatrix();
        void ComputeLookAtPosition(XMFLOAT3 lookAtPosition);

        const XMMATRIX& GetViewMatrix() const { return m_ViewMatrix; }
        const XMMATRIX& GetProjectionMatrix() const { return m_ProjectionMatrix; }

        void SetPosition(float x, float y, float z);
        void SetRotation(float x, float y, float z);
        void AdjustRotation(float x, float y, float z);

        const XMVECTOR& GetPosition() const { return m_Position; }
        const XMVECTOR& GetRotation() const { return m_Rotation; }

        const XMVECTOR& GetCurrentForwardVector() { return m_CurrentForwardVector; }
        const XMVECTOR& GetCurrentBackwardVector() { return m_CurrentBackwardVector; }
        const XMVECTOR& GetCurrentLeftVector() { return m_CurrentLeftVector; }
        const XMVECTOR& GetCurrentRightVector() { return m_CurrentRightVector; }

    private:
        XMMATRIX m_ViewMatrix;
        XMMATRIX m_ProjectionMatrix;

        XMVECTOR m_Position;
        XMVECTOR m_Rotation;

        // Camera Properties
        float m_Speed = 3.0f;

        // Default Values
        const XMVECTOR m_UpVector       = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        const XMVECTOR m_ForwardVector  = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        const XMVECTOR m_BackwardVector = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
        const XMVECTOR m_DownVector     = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
        const XMVECTOR m_LeftVector     = XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f);
        const XMVECTOR m_RightVector    = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

        bool m_FPS_Control = false;
        EngineContext* m_EngineContext = nullptr;

        XMVECTOR m_CurrentForwardVector;
        XMVECTOR m_CurrentLeftVector;
        XMVECTOR m_CurrentRightVector;
        XMVECTOR m_CurrentBackwardVector;
    };
}