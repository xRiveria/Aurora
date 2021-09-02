#pragma once
#include "IComponent.h"
#include <DirectXMath.h>

using namespace DirectX;

namespace Aurora
{
    class Renderer;
    class Input;

    enum ProjectionType
    {
        ProjectionType_Perspective,
        ProjectionType_Orthographic
    };

    class NewCamera : public IComponent
    {
    public:
        NewCamera(EngineContext* engineContext, Entity* entity, uint32_t componentID = 0);
        ~NewCamera() = default;

        // IComponent
        void Initialize() override;
        void Tick(float deltaTime) override;
        void Serialize(BinarySerializer* binarySerializer) override;
        void Deserialize(BinarySerializer* binaryDeserializer) override;

        // Matrices
        const XMMATRIX& GetViewMatrix() const { return m_ViewMatrix; }
        const XMMATRIX& GetProjectionMatrix() const { return m_ProjectionMatrix; }
        const XMMATRIX& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

        // Raycasting


        // Misc
        const XMFLOAT4& GetClearColor() const { return m_ClearColor; }
        void SetClearColor(const XMFLOAT4& clearColor) { m_ClearColor = clearColor;}
        bool GetFPSControlEnabled() const { return m_FPS_Control_Enabled; }
        void SetFPSControlEnabled(const bool isEnabled) { m_FPS_Control_Enabled = isEnabled; }
        bool GetFPSControlled() const { return m_FPS_Control_Assumed; }
        
        XMMATRIX ComputeViewMatrix() const;
        XMMATRIX ComputeProjectionMatrix(const bool reverseZ, const float nearPlane = 0.0f, const float farPlane = 0.0f);

    private:
        void FPSControl(float deltaTime);

    private:
        float m_Aperture = 50.0f;                   // Size of the lens diaphragm (mm). Controls depth of field and chromatic aberration.
        float m_ShutterSpeed = 1.0f / 60.0f;        // Length of time for which the camera shutter is open (seconds). Also controls the amount of motion blur.
        float m_ISO = 500.0f;                       // Sensitivity to light.

        float m_FOV_Horizontal_Radians = XMConvertToRadians(90.0f);
        float m_NearPlane = 0.3f;
        float m_FarPlane = 1000.0f;
        ProjectionType m_ProjectionType = ProjectionType::ProjectionType_Perspective;
        XMFLOAT4 m_ClearColor = XMFLOAT4(0.396f, 0.611f, 0.937f, 1.0f); // A nice cornflower blue.

        XMFLOAT3 m_Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
        XMMATRIX m_ViewMatrix = XMMatrixIdentity();
        XMMATRIX m_ProjectionMatrix = XMMatrixIdentity();
        XMMATRIX m_ViewProjectionMatrix = XMMatrixIdentity();
        XMVECTOR m_Rotation = XMQuaternionIdentity();

        // FPS
        bool m_IsDirty = false;
        bool m_FPS_Control_Enabled = true;
        bool m_FPS_Control_Assumed = false;
        bool m_FPS_Control_CursorHidden = false;
        XMFLOAT2 m_MouseLastPosition = XMFLOAT2(0.0f, 0.0f);
        XMFLOAT3 m_MovementSpeed = XMFLOAT3(0.0f, 0.0f, 0.0f);
        float m_MovementSpeedMinimum = 0.5f;
        float m_MovementSpeedMaximum = 5.0f;
        float m_MovementAcceleration = 1000.0f;
        float m_MovementDrag = 10.0f;
        XMFLOAT2 m_MouseSmoothed = XMFLOAT2(0.0f, 0.0f);
        XMFLOAT2 m_MouseRotation = XMFLOAT2(0.0f, 0.0f);
        float m_MouseSensitivity = 0.2f;
        float m_MouseSmoothing = 0.5f;

        // Dependancies
        Renderer* m_RendererSubsystem = nullptr;
        Input* m_InputSubsystem = nullptr;
    };
}