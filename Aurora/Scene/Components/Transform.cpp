#include "Aurora.h"
#include "Transform.h"

namespace Aurora
{
    Transform::Transform(EngineContext* engineContext, Entity* entity, uint32_t componentID) : IComponent(engineContext, entity, componentID)
    {

    }

    void Transform::Tick(float deltaTime)
    {
        UpdateTransform();
    }

    void Transform::UpdateTransform()
    {
        if (IsDirty())
        {
            SetDirty(false);
            XMStoreFloat4x4(&m_WorldMatrix, GetLocalMatrix());
        }
    }

    void Transform::Translate(const XMFLOAT3& value)
    {
        SetDirty();
        m_TranslationLocal.x = value.x;
        m_TranslationLocal.y = value.y;
        m_TranslationLocal.z = value.z;
    }

    void Transform::RotateRollPitchYaw(const XMFLOAT3& value)
    {
        SetDirty();

        XMVECTOR quaternion = XMLoadFloat4(&m_RotationLocal);
        XMVECTOR x = XMQuaternionRotationRollPitchYaw(value.x, 0, 0);
        XMVECTOR y = XMQuaternionRotationRollPitchYaw(0, value.y, 0);
        XMVECTOR z = XMQuaternionRotationRollPitchYaw(0, 0, value.z);

        quaternion = XMQuaternionMultiply(x, quaternion);
        quaternion = XMQuaternionMultiply(quaternion, y);
        quaternion = XMQuaternionMultiply(z, quaternion);

        quaternion = XMQuaternionNormalize(quaternion);

        XMStoreFloat4(&m_RotationLocal, quaternion);
    }

    void Transform::Rotate(const XMFLOAT4& value)
    {
        SetDirty();

        XMVECTOR result = XMQuaternionMultiply(XMLoadFloat4(&m_RotationLocal), XMLoadFloat4(&value));
        result = XMQuaternionNormalize(result);
        XMStoreFloat4(&m_RotationLocal, result);
    }

    void Transform::Scale(const XMFLOAT3& value)
    {
        SetDirty();

        m_ScaleLocal.x = (value.x == 0.0f) ? std::numeric_limits<float>::epsilon() : value.x;
        m_ScaleLocal.y = (value.y == 0.0f) ? std::numeric_limits<float>::epsilon() : value.y;
        m_ScaleLocal.z = (value.z == 0.0f) ? std::numeric_limits<float>::epsilon() : value.z;
    }

    XMFLOAT3 Transform::GetPosition() const
    {
        return *((XMFLOAT3*)&m_WorldMatrix._41);
    }

    XMFLOAT4 Transform::GetRotation() const
    {
        XMFLOAT4 rotationLocal;
        XMStoreFloat4(&rotationLocal, GetRotationVector());

        return rotationLocal;
    }

    XMFLOAT3 Transform::GetScale() const
    {
        XMFLOAT3 scaleLocal;
        XMStoreFloat3(&scaleLocal, GetScaleVector());
        return scaleLocal;
    }

    XMVECTOR Transform::GetPositionVector() const
    {
        return XMLoadFloat3((XMFLOAT3*)&m_WorldMatrix._41);
    }

    XMVECTOR Transform::GetRotationVector() const
    {
        XMVECTOR scaleLocal, rotationLocal, translationLocal;
        XMMatrixDecompose(&scaleLocal, &rotationLocal, &translationLocal, XMLoadFloat4x4(&m_WorldMatrix));

        return rotationLocal;
    }

    XMVECTOR Transform::GetScaleVector() const
    {
        XMVECTOR scaleLocal, rotationLocal, translationLocal;
        XMMatrixDecompose(&scaleLocal, &rotationLocal, &translationLocal, XMLoadFloat4x4(&m_WorldMatrix));

        return scaleLocal;
    }

    XMMATRIX Transform::GetLocalMatrix() const
    {
        XMVECTOR scaleLocal = XMLoadFloat3(&m_ScaleLocal);
        XMVECTOR rotationLocal = XMLoadFloat4(&m_RotationLocal);
        XMVECTOR translationLocal = XMLoadFloat3(&m_TranslationLocal);

        return XMMatrixScalingFromVector(scaleLocal) * XMMatrixRotationQuaternion(rotationLocal) * XMMatrixTranslationFromVector(translationLocal);
    }
}