#include "Aurora.h"
#include "Transform.h"
#include "../Scene/World.h"
#include <iostream>

namespace Aurora
{
    XMMATRIX g_IdentityMatrix = XMMatrixIdentity();

    Transform::Transform(EngineContext* engineContext, Entity* entity, uint32_t componentID) : IComponent(engineContext, entity, componentID)
    {
        m_ParentTransform = nullptr;
        
    }

    void Transform::Tick(float deltaTime)
    {
        UpdateTransform();
    }

    void Transform::Serialize(SerializationStream& outputStream)
    {
        outputStream << YAML::Key << "Transform Component";
        outputStream << YAML::BeginMap;

        outputStream << YAML::Key << "Translation" << YAML::Value << m_TranslationLocal;
        outputStream << YAML::Key << "Scale" << YAML::Value << m_ScaleLocal;
        outputStream << YAML::Key << "Rotation" << YAML::Value << m_RotationLocal;

        outputStream << YAML::EndMap;
    }

    void Transform::UpdateTransform()
    {     
        //if (IsDirty())
        //{
           //SetDirty(false);
        XMStoreFloat4x4(&m_LocalMatrix, GetLocalMatrix()); // Matrix relative to itself.
        //}
        
        // Compute world transform.
        if (!HasParentTransform())
        {
            m_WorldMatrix = m_LocalMatrix;
        }
        else
        {
            XMMATRIX matrix = XMLoadFloat4x4(&m_LocalMatrix) * GetParentTransformMatrix();
            XMStoreFloat4x4(&m_WorldMatrix, matrix);
        }

        // Update Children
        for (const auto& child : m_Children)
        {
            child->UpdateTransform();
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

    void Transform::RotateAngles(const XMFLOAT3& value)
    {
        m_RotationAngles.x = value.x;
        m_RotationAngles.y = value.y;
        m_RotationAngles.z = value.z;
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
        
        return XMMatrixScalingFromVector(scaleLocal) * (XMMatrixRotationX(m_RotationAngles.x) * XMMatrixRotationY(m_RotationAngles.y) * XMMatrixRotationZ(m_RotationAngles.z)) * XMMatrixTranslationFromVector(translationLocal);
    }

    // ==== Hierarchy ====

    // Make this transform have no parent.
    void Transform::BecomeOrphan()
    {
        // If there is no parent, we don't need to do anything.
        if (!m_ParentTransform)
        {
            return;
        }

        // Create a temporary reference to the parent.
        Transform* temporaryParentReference = m_ParentTransform;

        // Delete the original reference.
        m_ParentTransform = nullptr;

        // Update the transform without the parent now.
        UpdateTransform();

        // Make the parent search for children. This is an indirect way of making the parent "forget" about this child as it won't be able to find it.
        if (temporaryParentReference)
        {
            temporaryParentReference->AcquireChildren();
        }
    }

    // Sets a parent for this transform.
    void Transform::SetParentTransform(Transform* newParent)
    {
        // If the new parent is null, it means that this should become a root transform.
        if (!newParent)
        {
            BecomeOrphan();
            return;
        }

        // Make sure the new parent is not this transform.
        if (GetObjectID() == newParent->GetObjectID())
        {
            return;
        }

        // Make sure the new parent is different from the existing parent.
        if (HasParentTransform())
        {
            if (GetParentTransform()->GetObjectID() == newParent->GetObjectID())
            {
                return;
            }
        }

        // If the new parent is a descendent of this transform.
        if (newParent->IsDescendantOf(this))
        {
            // If this transform already has a parent...
            if (this->HasParentTransform())
            {
                // Assign the parent of this transform to the children.
                for (const auto& childTransform : m_Children)
                {
                    childTransform->SetParentTransform(GetParentTransform());
                }
            }
            else // If this transform doesn't have a parent...
            {
                // Make the children orphans.
                for (const auto& childTransform : m_Children)
                {
                    childTransform->BecomeOrphan();
                }
            }
        }

        // Switch parent but keep a pointer to the old one.
        Transform* oldParent = m_ParentTransform;
        m_ParentTransform = newParent;
        if (oldParent)
        {
            oldParent->AcquireChildren(); // Update the old parent (so it removes this child).
        }

        // Make the new parent "aware" of this transform/child.
        if (m_ParentTransform)
        {
            m_ParentTransform->AcquireChildren();
        }

        UpdateTransform();
    }

    void Transform::AddChildTransform(Transform* childTransform)
    {
        if (!childTransform)
        {
            return;
        }

        if (GetObjectID() == childTransform->GetObjectID())
        {
            return;
        }

        childTransform->SetParentTransform(this);
    }

    // Returns a child with the given index.
    Transform* Transform::GetChildByIndex(uint32_t childIndex)
    {
        if (!HasChildren())
        {
            AURORA_WARNING("%s has no children.", GetEntityName().c_str());
            return nullptr;
        }

        // Prevent an out of vector bounds error.
        if (childIndex >= GetChildrenCount())
        {
            AURORA_WARNING("There is no child with an index of \"%d\".", childIndex);
            return nullptr;
        }

        return m_Children[childIndex];
    }

    Transform* Transform::GetChildByName(const std::string& childName)
    {
        for (const auto& childTransform : m_Children)
        {
            if (childTransform->GetEntityName() == childName)
            {
                return childTransform;
            }
        }

        return nullptr;
    }

    // Searches the entire hierarchy, finds any children and saves them in m_Children. This is a recursive function, the children will find their own children and so on.
    void Transform::AcquireChildren()
    {
        m_Children.clear();
        m_Children.shrink_to_fit();

        auto entities = m_EngineContext->GetSubsystem<World>()->EntityGetAll();

        for (const std::shared_ptr<Entity>& entity : entities)
        {
            if (!entity) { continue; }

            // Retrieve a possible child.
            Transform* possibleChild = entity->GetTransform();

            // If it doesn't have a parent, we can forget about it.
            if (!possibleChild->HasParentTransform())
            {
                continue;
            }

            // If its parent matches this transform.
            if (possibleChild->GetParentTransform()->GetObjectID() == GetObjectID())
            {
                // Welcome home son.
                m_Children.emplace_back(possibleChild);
                // Make the child do the same thing all over, essentially resolving the entire hierarchy.
                possibleChild->AcquireChildren();
            }
        }
    }

    bool Transform::IsDescendantOf(const Transform* transform) const
    {
        for (const Transform* childTransform : transform->GetChildren())
        {
            if (GetObjectID() == childTransform->GetObjectID())
            {
                return true;
            }

            if (childTransform->HasChildren())
            {
                if (IsDescendantOf(childTransform))
                {
                    return true;
                }
            }
        }

        return false;
    }

    // Recursively get all children of this transform, and their children.
    void Transform::GetDescendants(std::vector<Transform*>* descendants)
    {
        for (Transform* childTransform : m_Children)
        {
            descendants->emplace_back(childTransform);

            if (childTransform->HasChildren())
            {
                childTransform->GetDescendants(descendants);
            }
        }
    }

    XMMATRIX Transform::GetParentTransformMatrix() const
    {
        XMMATRIX worldMatrix = HasParentTransform() ? GetParentTransform()->GetWorldMatrix() : g_IdentityMatrix;
        return worldMatrix;
    }
}
