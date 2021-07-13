#include "Aurora.h"
#include "Camera.h"
#include "../Input/Input.h"

namespace Aurora
{
    Camera::Camera(EngineContext* engineContext, Entity* entity, uint32_t componentID) : IComponent(engineContext, entity, componentID)
    {

    }

    Camera::~Camera()
    {

    }

    void Camera::Initialize()
    {
        m_Position = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
        m_Rotation = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

        ComputeViewMatrix();

        AURORA_INFO("Initialized Camera.");
    }

    void Camera::Tick(float deltaTime)
    {
        Input* inputSystem = m_EngineContext->GetSubsystem<Input>();

        if (inputSystem->IsKeyPressed(AURORA_KEY_W))
        {
            m_Position = m_Position + GetCurrentForwardVector() * m_Speed * deltaTime;

            ComputeViewMatrix();
        }

        if (inputSystem->IsKeyPressed(AURORA_KEY_S))
        {
            m_Position = m_Position + GetCurrentBackwardVector() * m_Speed * deltaTime;

            ComputeViewMatrix();
        }

        if (inputSystem->IsKeyPressed(AURORA_KEY_A))
        {
            m_Position = m_Position + GetCurrentLeftVector() * m_Speed * deltaTime;

            ComputeViewMatrix();
        }

        if (inputSystem->IsKeyPressed(AURORA_KEY_D))
        {
            m_Position = m_Position + GetCurrentRightVector() * m_Speed * deltaTime;

            ComputeViewMatrix();
        }

        if (inputSystem->IsKeyPressed(AURORA_KEY_UP))
        {
            m_Position = m_Position + m_UpVector * m_Speed * deltaTime;

            ComputeViewMatrix();
        }

        if (inputSystem->IsKeyPressed(AURORA_KEY_DOWN))
        {
            m_Position = m_Position + m_DownVector * m_Speed * deltaTime;

            ComputeViewMatrix();
        }

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

        if (m_FPS_Control)
        {
            AdjustRotation((float)inputSystem->GetMousePositionDelta().second * 0.1f * deltaTime, (float)inputSystem->GetMousePositionDelta().first * 0.1f * deltaTime, 0.0f);
        }
    }

    void Camera::ComputePerspectiveMatrix(float fovInDegrees, float aspectRatio, float nearZ, float farZ)
    {
        float fovInRadians = (fovInDegrees / 360.0f) * XM_2PI;
        m_ProjectionMatrix = XMMatrixPerspectiveFovLH(fovInRadians, aspectRatio, nearZ, farZ);
    }

    void Camera::ComputeViewMatrix()
    {
        XMFLOAT3 rotation;
        XMStoreFloat3(&rotation, m_Rotation);

        // Calculate Rotation Matrix.
        XMMATRIX cameraRotationMatrix = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);

        // Calculate unit vector of camera target based on our camera's forward value transformed by the rotation. Direction.
        XMVECTOR cameraTarget = XMVector3TransformCoord(m_ForwardVector, cameraRotationMatrix);

        // Adjust camera target to be offset by the camera's current position.
        cameraTarget += m_Position;

        // Calculate up direction based on current rotation.
        XMVECTOR upDirection = XMVector3TransformCoord(m_UpVector, cameraRotationMatrix);

        // Rebuild View Matrix.
        m_ViewMatrix = XMMatrixLookAtLH(m_Position, cameraTarget, upDirection);

        XMMATRIX vectorRotationMatrix = XMMatrixRotationRollPitchYaw(0.0f, rotation.y, 0.0f);

        m_CurrentForwardVector = XMVector3TransformCoord(m_ForwardVector, vectorRotationMatrix);
        m_CurrentBackwardVector = XMVector3TransformCoord(m_BackwardVector, vectorRotationMatrix);
        m_CurrentLeftVector = XMVector3TransformCoord(m_LeftVector, vectorRotationMatrix);
        m_CurrentRightVector = XMVector3TransformCoord(m_RightVector, vectorRotationMatrix);
    }

    void Camera::ComputeLookAtPosition(XMFLOAT3 lookAtPosition)
    {
        // Verify that our look at position is not the same as the camera's position.
        XMFLOAT3 currentPosition;
        XMStoreFloat3(&currentPosition, m_Position);
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
        m_Position = XMLoadFloat3(&newPosition);
        ComputeViewMatrix();
    }

    void Camera::SetRotation(float x, float y, float z)
    {
        XMFLOAT3 newRotation = XMFLOAT3(x, y, z);
        m_Rotation = XMLoadFloat3(&newRotation);
        ComputeViewMatrix();
    }

    void Camera::AdjustRotation(float x, float y, float z)
    {
        XMFLOAT3 rotation;
        XMStoreFloat3(&rotation, m_Rotation);

        rotation.x += x;
        rotation.y += y;
        rotation.z += z;
        m_Rotation = XMLoadFloat3(&rotation);

        ComputeViewMatrix();
    }
}