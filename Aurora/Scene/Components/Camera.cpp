#include "Aurora.h"
#include "Camera.h"
#include "../Input/Input.h"
#include "../Entity.h"

namespace Aurora
{
    Camera::Camera(EngineContext* engineContext, Entity* entity, uint32_t componentID) : IComponent(engineContext, entity, componentID)
    {
        m_Type = ComponentType::Camera;
    }

    Camera::~Camera()
    {

    }

    void Camera::Initialize()
    {
        // m_Position = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
        // m_Rotation = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

        ComputeViewMatrix();

        AURORA_INFO(LogLayer::ECS, "Initialized Camera.");
    }

    void Camera::Tick(float deltaTime)
    {
        ComputeViewMatrix();
        //if (m_IsFirstRun)
       // {
        //    
       //     m_IsFirstRun = false;
       // }

        Input* inputSystem = m_EngineContext->GetSubsystem<Input>();
        XMVECTOR position = GetEntity()->GetComponent<Transform>()->GetPositionVector();

        if (inputSystem->IsKeyPressed(AURORA_KEY_W))
        {
            position = position + GetCurrentForwardVector() * m_Speed * deltaTime;

            ComputeViewMatrix();
        }

        if (inputSystem->IsKeyPressed(AURORA_KEY_S))
        {
            position = position + GetCurrentBackwardVector() * m_Speed * deltaTime;

            ComputeViewMatrix();
        }

        if (inputSystem->IsKeyPressed(AURORA_KEY_A))
        {
            position = position + GetCurrentLeftVector() * m_Speed * deltaTime;

            ComputeViewMatrix();
        }

        if (inputSystem->IsKeyPressed(AURORA_KEY_D))
        {
            position = position + GetCurrentRightVector() * m_Speed * deltaTime;

            ComputeViewMatrix();
        }

        if (inputSystem->IsKeyPressed(AURORA_KEY_UP))
        {
            position = position + m_UpVector * m_Speed * deltaTime;

            ComputeViewMatrix();
        }

        if (inputSystem->IsKeyPressed(AURORA_KEY_DOWN))
        {
            position = position + m_DownVector * m_Speed * deltaTime;

            ComputeViewMatrix();
        }

        XMStoreFloat3(&GetEntity()->GetComponent<Transform>()->m_TranslationLocal, position);
        GetEntity()->GetTransform()->SetDirty(true);
        FPSControl(deltaTime);
    }

    void Camera::FPSControl(float deltaTime)
    {
        // We will activate FPS Control if our mouse is in the viewport and if the right mouse button is held down. This control will be maintained as long as our mouse button is held down.
        Input* inputSystem = m_EngineContext->GetSubsystem<Input>();
        if (inputSystem->IsMouseButtonPressed(AURORA_MOUSE_BUTTON_RIGHT))
        {
            m_FPS_Control = true;
        }
        else
        {
            m_FPS_Control = false;
        }

        if (inputSystem->IsMouseButtonPressed(2))
        {
            m_Speed = 9.0f;
        }
        else
        {
            m_Speed = 3.0f;
        }

        if (m_FPS_Control)
        {
            AdjustRotation((float)inputSystem->GetMousePositionDelta().second * 5.0 * deltaTime, (float)inputSystem->GetMousePositionDelta().first * 5.0 * deltaTime, 0.0f);
            ComputeViewMatrix();
        }
    }

    void Camera::ComputePerspectiveMatrix(float fovInDegrees, float aspectRatio, float nearZ, float farZ)
    {
        float fovInRadians = (fovInDegrees / 360.0f) * XM_2PI;
        m_ProjectionMatrix = XMMatrixPerspectiveFovLH(fovInRadians, aspectRatio, nearZ, farZ);
    }

    void Camera::ComputeViewMatrix()
    {
        // XMFLOAT3 rotationInDegrees = XMFLOAT3(XMConvertToDegrees(GetEntity()->m_Transform->m_RotationInRadians.x), XMConvertToDegrees(GetEntity()->m_Transform->m_RotationInRadians.y), XMConvertToDegrees(GetEntity()->m_Transform->m_RotationInRadians.z));

        // Calculate Rotation Matrix.
        XMMATRIX cameraRotationMatrix = XMMatrixRotationRollPitchYaw(GetEntity()->m_Transform->m_RotationInRadians.x, GetEntity()->m_Transform->m_RotationInRadians.y, GetEntity()->m_Transform->m_RotationInRadians.z);

        // Calculate unit vector of camera target based on our camera's forward value transformed by the rotation. Direction.
        XMVECTOR cameraTarget = XMVector3TransformCoord(m_ForwardVector, cameraRotationMatrix);

        // Adjust camera target to be offset by the camera's current position.
        cameraTarget += GetEntity()->GetComponent<Transform>()->GetPositionVector();

        // Calculate up direction based on current rotation.
        XMVECTOR upDirection = XMVector3TransformCoord(m_UpVector, cameraRotationMatrix);

        // Rebuild View Matrix.
        m_ViewMatrix = XMMatrixLookAtLH(GetEntity()->GetComponent<Transform>()->GetPositionVector(), cameraTarget, upDirection);

        XMMATRIX vectorRotationMatrix = XMMatrixRotationRollPitchYaw(0.0f, GetEntity()->m_Transform->m_RotationInRadians.y, 0.0f);

        m_CurrentForwardVector = XMVector3TransformCoord(m_ForwardVector, vectorRotationMatrix);
        m_CurrentBackwardVector = XMVector3TransformCoord(m_BackwardVector, vectorRotationMatrix);
        m_CurrentLeftVector = XMVector3TransformCoord(m_LeftVector, vectorRotationMatrix);
        m_CurrentRightVector = XMVector3TransformCoord(m_RightVector, vectorRotationMatrix);
    }

    void Camera::ComputeLookAtPosition(XMFLOAT3 lookAtPosition)
    {
        // Verify that our look at position is not the same as the camera's position.
        XMFLOAT3 currentPosition;
        XMStoreFloat3(&currentPosition, GetEntity()->GetComponent<Transform>()->GetPositionVector());
        if (lookAtPosition.x == currentPosition.x && lookAtPosition.y == currentPosition.y && lookAtPosition.z == currentPosition.z)
        {
            return;
        }

        lookAtPosition.x = currentPosition.x - lookAtPosition.x;
        lookAtPosition.y = currentPosition.y - lookAtPosition.y;
        lookAtPosition.z = currentPosition.z - lookAtPosition.z;

        float pitch = 0.0f;
        if (lookAtPosition.y != 0.0f)
        {
            const float distance = sqrt(lookAtPosition.x * lookAtPosition.x + lookAtPosition.z * lookAtPosition.z);
            pitch = atan(lookAtPosition.y / distance);
        }

        float yaw = 0.0f;
        if (lookAtPosition.x != 0.0f)
        {
            yaw = atan(lookAtPosition.x / lookAtPosition.z);
        }
        if (lookAtPosition.z > 0)
        {
            yaw += XM_PI;
        }

        SetRotation(pitch, yaw, 0.0f);
    }

    void Camera::SetPosition(float x, float y, float z)
    {
        XMFLOAT3 newPosition = XMFLOAT3(x, y, z);
        GetEntity()->GetComponent<Transform>()->m_TranslationLocal = newPosition;
        ComputeViewMatrix();
    }

    void Camera::SetRotation(float x, float y, float z)
    {
        XMFLOAT3 rotationInRadians = XMFLOAT3(XMConvertToRadians(x), XMConvertToRadians(y), XMConvertToRadians(z));
        GetEntity()->m_Transform->m_RotationInRadians = rotationInRadians;
        ComputeViewMatrix();
    }

    void Camera::AdjustRotation(float x, float y, float z)
    {
        XMFLOAT3 rotationInRadians = XMFLOAT3(XMConvertToRadians(x), XMConvertToRadians(y), XMConvertToRadians(z));

        GetEntity()->m_Transform->m_RotationInRadians.x += rotationInRadians.x;
        GetEntity()->m_Transform->m_RotationInRadians.y += rotationInRadians.y;
        GetEntity()->m_Transform->m_RotationInRadians.z += rotationInRadians.z;

        ComputeViewMatrix();
    }
}