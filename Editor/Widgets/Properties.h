#pragma once
#include "../Backend/Widget.h"
#include <memory>

namespace Aurora
{
    class Entity;
    class Transform;
    class Material;
    class RigidBody;
    class Light;
    class Collider;
    class Renderable;
    class ResourceCache;
}

class Properties : public Widget
{
public:
    Properties(Editor* editorContext, Aurora::EngineContext* engineContext);

    void OnTickVisible() override;

    static void Inspect(const std::weak_ptr<Aurora::Entity>& entity);
    static void Inspect(const std::weak_ptr<Aurora::Material>& material);

    void ShowAddComponentButton() const;

public:
    // Inspected Resources
    static std::weak_ptr<Aurora::Entity> m_InspectedEntity;
    static std::weak_ptr<Aurora::Material> m_InspectedMaterial;

private:
    void ShowRenderableProperties(Aurora::Renderable* renderableComponent) const;
    void ShowTransformProperties(Aurora::Transform* transformComponent) const;
    void ShowMaterialProperties(Aurora::Material* materialComponent) const;
    void ShowLightProperties(Aurora::Light* lightComponent) const;
    void ShowRigidBodyProperties(Aurora::RigidBody* rigidBodyComponent) const;
    void ShowColliderProperties(Aurora::Collider* colliderComponent) const;

private:
    Aurora::ResourceCache* m_ResourceCache = nullptr;
};