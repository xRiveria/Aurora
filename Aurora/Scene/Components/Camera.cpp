#include "Aurora.h"
#include "Camera.h"
#include "../Input/Input.h"
#include "../Scene/Entity.h"

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

    }

    void Camera::Tick(float deltaTime)
    {
        Input* inputSystem = m_EngineContext->GetSubsystem<Input>();

        if (inputSystem->IsKeyPressed(AURORA_KEY_W))
        {
            // XMFLOAT3 derp = { 0, 0, 1 };
            // m_Entity->m_Transform->m_TranslationLocal = m_Entity->m_Transform->m_TranslationLocal + derp;
        }

        if (inputSystem->IsKeyPressed(AURORA_KEY_S))
        {
            //m_Entity->m_Transform->m_TranslationLocal = m_Entity->m_Transform->m_TranslationLocal + GetCurrentBackwardVector() * m_Speed * deltaTime;

        }

        if (inputSystem->IsKeyPressed(AURORA_KEY_A))
        {
            //m_Entity->m_Transform->m_TranslationLocal = m_Entity->m_Transform->m_TranslationLocal + GetCurrentLeftVector() * m_Speed * deltaTime;

        }

        if (inputSystem->IsKeyPressed(AURORA_KEY_D))
        {
            //m_Entity->m_Transform->m_TranslationLocal = m_Entity->m_Transform->m_TranslationLocal + GetCurrentRightVector() * m_Speed * deltaTime;
        }

        if (inputSystem->IsKeyPressed(AURORA_KEY_UP))
        {
            m_Entity->m_Transform->GetPositionVector() + m_ForwardVector * m_Speed * deltaTime;
        }

        if (inputSystem->IsKeyPressed(AURORA_KEY_DOWN))
        {
            
        }
        
        FPSControl(deltaTime);

        XMStoreFloat4x4(&m_Projection, XMMatrixPerspectiveFovLH(m_FOV, m_Width / m_Height, m_zNearPlane, m_zFarPlane));

        TransformCamera(m_Entity->m_Transform);

        XMVECTOR _Eye = XMLoadFloat3(&m_Eye); // Eye Position
        XMVECTOR _At = XMLoadFloat3(&m_At); // Focus Direction
        XMVECTOR _Up = XMLoadFloat3(&m_Up); // Up Direction
        
        XMMATRIX _View = XMMatrixLookToLH(_Eye, _At, _Up);
        XMStoreFloat4x4(&m_View, _View);

        XMMATRIX _P = XMLoadFloat4x4(&m_Projection);
        XMMATRIX _VP = XMMatrixMultiply(_View, _P);
        XMStoreFloat4x4(&m_VP, _VP);

        // FPSControl(deltaTime);
    }

    void Camera::TransformCamera(Transform* transform)
    {
        XMVECTOR Scale, Rotate, Translate;
        XMMatrixDecompose(&Scale, &Rotate, &Translate, XMLoadFloat4x4(&transform->m_WorldMatrix));

        XMVECTOR _Eye = Translate;
        XMVECTOR _At = XMVectorSet(0, 0, 1, 0);
        XMVECTOR _Up = XMVectorSet(0, 1, 0, 0);

        XMMATRIX _V = XMMatrixLookAtLH(_Eye, _At, _Up);
        XMStoreFloat4x4(&m_View, _V);

        XMStoreFloat3(&m_Eye, _Eye);
        XMStoreFloat3(&m_At, _At);
        XMStoreFloat3(&m_Up, _Up);
    }

    void Camera::CreatePerspective(float newWidth, float newHeight, float newNear, float newFar, float newFOV)
    {
        m_zNearPlane = newNear;
        m_zFarPlane = newFar;
        m_Width = newWidth;
        m_Height = newHeight;
        m_FOV = newFOV;
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

        if (inputSystem->IsKeyPressed(AURORA_KEY_LEFT_CONTROL))
        {
            m_Speed = 9.0f;
        }
        else
        {
            m_Speed = 3.0f;
        }

        if (m_FPS_Control)
        {
            AdjustRotation((float)inputSystem->GetMousePositionDelta().second * 0.1f * deltaTime, (float)inputSystem->GetMousePositionDelta().first * 0.1f * deltaTime, 0.0f);
        }
    }

    void Camera::AdjustRotation(float x, float y, float z)
    {
        XMFLOAT3 rotation;
        XMVECTOR rotationVector = XMVectorSet(m_Entity->m_Transform->m_RotationLocal.x, m_Entity->m_Transform->m_RotationLocal.y, m_Entity->m_Transform->m_RotationLocal.z, m_Entity->m_Transform->m_RotationLocal.w);
        XMStoreFloat3(&rotation, rotationVector);

        rotation.x += x;
        rotation.y += y;
        rotation.z += z;
        m_Entity->m_Transform->m_RotationLocal = { rotation.x, rotation.y, rotation.z, 1.0 };
    }
}