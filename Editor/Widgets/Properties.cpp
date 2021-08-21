#include "Properties.h"
#include "../Scene/Entity.h"
#include "../Scene/Components/Transform.h"
#include "../Scene/Components/Light.h"
#include "../Scene/Components/RigidBody.h"
#include "../Scene/Components/Renderable.h"
#include "../Scene/Components/Collider.h"
#include "../Renderer/Material.h"
#include "../Physics/Physics.h"
#include "../Backend/Source/imgui_internal.h"
#include "../Renderer/Renderer.h"
#include "../Backend/Utilities/Extensions.h"
#include "FileSystem.h"

std::weak_ptr<Aurora::Entity> Properties::m_InspectedEntity;
std::weak_ptr<Aurora::Material> Properties::m_InspectedMaterial;

XMFLOAT4 g_DefaultColor = { 1, 1, 1, 1 };
float g_ForceAmount[3] = { 0.0f, 0.0f, 50.0f };

Properties::Properties(Editor* editorContext, Aurora::EngineContext* engineContext) : Widget(editorContext, engineContext)
{
    m_WidgetName = "Properties";
    m_WidgetSize.x = 500.0f; // Minimum Width
}

void Properties::OnTickVisible()
{
    if (!m_InspectedEntity.expired())
    {
        Aurora::Entity* entityPointer = m_InspectedEntity.lock().get();
        Aurora::Renderable* meshPointer = entityPointer->GetComponent<Aurora::Renderable>();
        Aurora::Material* materialPointer = meshPointer ? meshPointer->GetMaterial() : nullptr;

        ImGui::Checkbox("##EntityCheckbox", &entityPointer->m_IsActive);
        ImGui::SameLine();

        char entityNameBuffer[256] = "Entity";
        memset(entityNameBuffer, 0, sizeof(entityNameBuffer));
        strcpy_s(entityNameBuffer, sizeof(entityNameBuffer), entityPointer->GetObjectName().c_str());

        if (ImGui::InputText("##EntityName", entityNameBuffer, sizeof(entityNameBuffer)))
        {
            entityPointer->SetName(std::string(entityNameBuffer));
        }
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4({ 0.65f, 0.65f, 0.65f, 1.0f }));
        ImGui::Text("Entity ID: %d", entityPointer->GetObjectID());
        ImGui::PopStyleColor();

        if (meshPointer)
        {
            // Temporary
            std::string materialName = materialPointer ? materialPointer->GetResourceName() : "N/A";

            char materialNameBuffer[256] = "Material";
            memset(materialNameBuffer, 0, sizeof(materialNameBuffer));
            strcpy_s(materialNameBuffer, sizeof(materialNameBuffer), materialName.c_str());

            ImGui::InputText("", materialNameBuffer, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_ReadOnly);
            if (auto payload = EditorExtensions::ReceiveDragPayload(EditorExtensions::DragPayloadType::DragPayloadType_Material))
            {
                meshPointer->SetMaterial(std::get<const char*>(payload->m_Data));
            }
        }

        ShowTransformProperties(entityPointer->GetComponent<Aurora::Transform>());
        ShowMaterialProperties(materialPointer);
        ShowLightProperties(entityPointer->GetComponent<Aurora::Light>());
        ShowColliderProperties(entityPointer->GetComponent<Aurora::Collider>());
        ShowRigidBodyProperties(entityPointer->GetComponent<Aurora::RigidBody>());

        ShowAddComponentButton();
    }
    else if (!m_InspectedMaterial.expired())
    {
        ShowMaterialProperties(m_InspectedMaterial.lock().get());
    }
}

void Properties::Inspect(const std::weak_ptr<Aurora::Entity>& entity)
{
    m_InspectedEntity = entity;

    // If we were previously inspecting a material, save its changes.
    if (!m_InspectedMaterial.expired())
    {
        m_InspectedMaterial.lock()->SaveToFile(m_InspectedMaterial.lock()->GetResourceFilePathNative());
    }
    m_InspectedMaterial.reset();
}

void Properties::Inspect(const std::weak_ptr<Aurora::Material>& material)
{
    m_InspectedEntity.reset();
    m_InspectedMaterial = material;
}

void Properties::ShowAddComponentButton() const
{
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.5 - 50);

    if (ImGui::Button("Add Component"))
    {
        ImGui::OpenPopup("##AddComponentContextMenu");
    }

    if (ImGui::BeginPopup("##AddComponentContextMenu"))
    {
        if (std::shared_ptr<Aurora::Entity> entity = m_InspectedEntity.lock())
        {
             if (ImGui::MenuItem("Rigidbody"))
             {
                 entity->AddComponent<Aurora::RigidBody>();
             }

             if (ImGui::MenuItem("Collider"))
             {
                 entity->AddComponent<Aurora::Collider>();
             }

             if (ImGui::MenuItem("Light"))
             {
                 entity->AddComponent<Aurora::Light>();
             }           
        }

        ImGui::EndPopup();
    }
}

inline bool ComponentBegin(const std::string& componentName)
{
    // Collapsible Contents
    const bool isCollapsed = ImGui::CollapsingHeader(componentName.c_str(), ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);

    return isCollapsed;
}

inline void ComponentEnd()
{
    ImGui::Separator();
}

static void DrawVector3Control(const std::string& label, XMFLOAT3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
{
    ImGuiIO& io = ImGui::GetIO();
    auto boldFont = io.Fonts->Fonts[0];

    ImGui::PushID(label.c_str());
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, columnWidth); //First Column
    ImGui::Text(label.c_str());
    ImGui::NextColumn();

    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

    float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
    ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

    // ====================================================================================

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });

    ImGui::PushFont(boldFont);
    if (ImGui::Button("X", buttonSize))
    {
        values.x = resetValue;
    }
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##X", &values.x, 0.3f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    // ====================================================================================

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
    ImGui::PushFont(boldFont);
    if (ImGui::Button("Y", buttonSize))
    {
        values.y = resetValue;
    }
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##Y", &values.y, 0.3f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    // ====================================================================================

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.35f, 0.9f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
    ImGui::PushFont(boldFont);
    if (ImGui::Button("Z", buttonSize))
    {
        values.z = resetValue;
    }
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##Z", &values.z, 0.3f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();

    ImGui::PopStyleVar();
    ImGui::Columns(1);
    ImGui::PopID();
}

static void DrawMaterialControl(const std::string& label, Aurora::DX11_Texture* materialTexture, Aurora::EngineContext* engineContext, const std::function<void(const std::shared_ptr<Aurora::DX11_Texture>&)> TextureSetter, bool drawColorControls = false, XMFLOAT4& colorValues = g_DefaultColor)
{
    ImGui::PushID(label.c_str());
    ImGuiIO& io = ImGui::GetIO();
    auto boldFont = io.Fonts->Fonts[0];
    ImGui::PushFont(boldFont);
    ImGui::Text(label.c_str());
    ImGui::PopFont();

    Aurora::DX11_Texture* texture = (materialTexture != nullptr) ? materialTexture : engineContext->GetSubsystem<Aurora::Renderer>()->m_DefaultWhiteTexture.get();

    if (materialTexture != nullptr)
    {
        ImGui::Image((void*)texture->GetShaderResourceView().Get(), ImVec2(60, 60));
    }
    else
    {
        ImGui::Image((void*)texture->GetShaderResourceView().Get(), ImVec2(60, 60));
    }

    ImGui::SameLine();

    bool isTrue = true; /// To Implemenet
    ImGui::Checkbox("Use", &isTrue);
    
    ImGui::SameLine();

    if (ImGui::Button("Load..."))
    {
        std::optional<std::string> filePath = EditorExtensions::OpenFile("Textures", engineContext);
        if (filePath.has_value())
        {
            std::string path = filePath.value();
            if (const std::shared_ptr<Aurora::DX11_Texture> texture = engineContext->GetSubsystem<Aurora::ResourceCache>()->Load<Aurora::DX11_Texture>(path))
            {
                TextureSetter(texture);
            }
        }
    }

    if (drawColorControls)
    {
        ImGui::SameLine();
        if (ImGui::ColorButton("##ColorButton", ImVec4(colorValues.x, colorValues.y, colorValues.z, colorValues.w)))
        {
            ImGui::OpenPopup("MaterialColorPicker");
        }

        if (ImGui::BeginPopup("MaterialColorPicker"))
        {
            ImGui::ColorPicker4("##MaterialColorPick", &colorValues.x);

            ImGui::EndPopup();
        }
    }

    ImGui::SameLine(); ImGui::NewLine();
    ImGui::Text("File: %s", texture->GetResourceFilePathNative().c_str());
    
    ImGui::Separator();
    ImGui::PopID();
}

void Properties::ShowTransformProperties(Aurora::Transform* transformComponent) const
{
    if (!transformComponent)
    {
        return;
    }

    if (ComponentBegin("Transform"))
    {
        DrawVector3Control("Translation", transformComponent->m_TranslationLocal);
        
        XMFLOAT3 radiansToDegrees = XMFLOAT3(XMConvertToDegrees(transformComponent->m_RotationInRadians.x), XMConvertToDegrees(transformComponent->m_RotationInRadians.y), XMConvertToDegrees(transformComponent->m_RotationInRadians.z));
        DrawVector3Control("Rotation", radiansToDegrees, 1.0f);
        transformComponent->m_RotationInRadians = XMFLOAT3(XMConvertToRadians(radiansToDegrees.x), XMConvertToRadians(radiansToDegrees.y), XMConvertToRadians(radiansToDegrees.z));
        
        DrawVector3Control("Scale", transformComponent->m_ScaleLocal, 1.0f);
        transformComponent->SetDirty(true);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Parent Name: %s", transformComponent->HasParentTransform() ? transformComponent->GetParentTransform()->GetEntityName().c_str() : "No Parent");
    ImGui::Text("Children Count: %.0f", static_cast<float>(transformComponent->m_Children.size()));
    ImGui::Spacing();
    ComponentEnd();
}

void Properties::ShowMaterialProperties(Aurora::Material* materialComponent) const
{
    if (!materialComponent)
    {
        return;
    }

    if (ComponentBegin("Material"))
    {
        ImGui::PushID(materialComponent->GetObjectID());

        DrawMaterialControl("Albedo Map", materialComponent->m_Textures[Aurora::MaterialSlot::MaterialSlot_Albedo].get(), m_EngineContext, [&materialComponent](const std::shared_ptr<Aurora::DX11_Texture>& texture) { materialComponent->SetTextureSlot(Aurora::MaterialSlot::MaterialSlot_Albedo, texture); }, true, materialComponent->m_AlbedoColor);
        DrawMaterialControl("Roughness Map", materialComponent->m_Textures[Aurora::MaterialSlot::MaterialSlot_Roughness].get(), m_EngineContext, [&materialComponent](const std::shared_ptr<Aurora::DX11_Texture>& texture) { materialComponent->SetTextureSlot(Aurora::MaterialSlot::MaterialSlot_Roughness, texture); });
        DrawMaterialControl("Normal Map", materialComponent->m_Textures[Aurora::MaterialSlot::MaterialSlot_Normal].get(), m_EngineContext, [&materialComponent](const std::shared_ptr<Aurora::DX11_Texture>& texture) { materialComponent->SetTextureSlot(Aurora::MaterialSlot::MaterialSlot_Normal, texture); });
        DrawMaterialControl("Metallic Map", materialComponent->m_Textures[Aurora::MaterialSlot::MaterialSlot_Metallic].get(), m_EngineContext, [&materialComponent](const std::shared_ptr<Aurora::DX11_Texture>& texture) { materialComponent->SetTextureSlot(Aurora::MaterialSlot::MaterialSlot_Metallic, texture); });
        DrawMaterialControl("Occlusion Map", materialComponent->m_Textures[Aurora::MaterialSlot::MaterialSlot_Occlusion].get(), m_EngineContext, [&materialComponent](const std::shared_ptr<Aurora::DX11_Texture>& texture) { materialComponent->SetTextureSlot(Aurora::MaterialSlot::MaterialSlot_Occlusion, texture); });

        ImGui::SliderFloat("Roughness", &materialComponent->m_Properties[Aurora::MaterialSlot::MaterialSlot_Roughness], 0.0, 1.0);
        ImGui::SliderFloat("Metalness", &materialComponent->m_Properties[Aurora::MaterialSlot::MaterialSlot_Metallic], 0.0, 1.0);

        if (ImGui::Button("Deserializing Test"))
        {
            materialComponent->LoadFromFile(materialComponent->GetResourceFilePathNative());
        }
        ImGui::PopID();
    }

    ComponentEnd(); // Seperator already avaliable in DrawMaterialControl.
}

void Properties::ShowLightProperties(Aurora::Light* lightComponent) const
{
    if (!lightComponent)
    {
        return;
    }

    if (ComponentBegin("Light"))
    {
        ImGui::PushID(lightComponent->GetObjectID());
        ImGui::DragFloat("Light Intensity", &lightComponent->m_Intensity, 0.1, 0.0, 500);
        ImGui::Text("Light Color:");
        ImGui::SameLine();

        if (ImGui::ColorButton("##LightColorButton", ImVec4(lightComponent->m_Color.x, lightComponent->m_Color.y, lightComponent->m_Color.z, 1.0)))
        {
            ImGui::OpenPopup("LightColorPopup");
        }

        bool areShadowsEnabled = lightComponent->IsCastingShadow();
        if (ImGui::Checkbox("Cast Shadows", &areShadowsEnabled))
        {
            lightComponent->SetIsCastingShadow(areShadowsEnabled);
        }

        if (ImGui::BeginPopup("LightColorPopup"))
        {
            ImGui::ColorPicker3("##LightColor", &lightComponent->m_Color.x);

            ImGui::EndPopup();
        }

        ImGui::PopID();
    }

    ComponentEnd();
}

void Properties::ShowColliderProperties(Aurora::Collider* colliderComponent) const
{
    if (!colliderComponent)
    {
        return;
    }

    if (ComponentBegin("Collider"))
    {
        ImGui::PushID(colliderComponent->GetObjectID());

        if (ImGui::BeginCombo("Collider Shape", colliderComponent->GetColliderShapeToString().c_str(), 0))
        {
            if (ImGui::Selectable("Box"))
            {
                colliderComponent->SetShapeType(Aurora::ColliderShape::ColliderShape_Box);
            }
            if (ImGui::Selectable("Sphere"))
            {
                colliderComponent->SetShapeType(Aurora::ColliderShape::ColliderShape_Sphere);
            }
            if (ImGui::Selectable("Static Plane"))
            {
                colliderComponent->SetShapeType(Aurora::ColliderShape::ColliderShape_StaticPlane);
            }
            if (ImGui::Selectable("Cylinder"))
            {
                colliderComponent->SetShapeType(Aurora::ColliderShape::ColliderShape_Cylinder);
            }
            if (ImGui::Selectable("Capsule"))
            {
                colliderComponent->SetShapeType(Aurora::ColliderShape::ColliderShape_Capsule);
            }
            if (ImGui::Selectable("Cone"))
            {
                colliderComponent->SetShapeType(Aurora::ColliderShape::ColliderShape_Cone);
            }
            if (ImGui::Selectable("Mesh"))
            {
                colliderComponent->SetShapeType(Aurora::ColliderShape::ColliderShape_Mesh);
            }

            ImGui::EndCombo();
        }

        // Size
        XMFLOAT3 colliderSize = colliderComponent->GetBoundingBox();
        if (ImGui::DragFloat3("Size", &colliderSize.x, 0.1f))
        {
            colliderComponent->SetBoundingBox(colliderSize);
        }

        // Center
        XMFLOAT3 centerLocation = colliderComponent->GetCenter();
        if (ImGui::DragFloat3("Center", &centerLocation.x, 0.1f))
        {
            colliderComponent->SetCenter(centerLocation);
        }

        /// Optimize but only if current shape is Mesh.

        ImGui::PopID();
    }

    ComponentEnd();
}

void Properties::ShowRigidBodyProperties(Aurora::RigidBody* rigidBodyComponent) const
{
    if (!rigidBodyComponent)
    {
        return;
    }

    if (ComponentBegin("Rigidbody"))
    {
        ImGui::PushID(rigidBodyComponent->GetObjectID());

        float rigidBodyMass = rigidBodyComponent->GetMass();
        if (ImGui::DragFloat("Mass", &rigidBodyMass, 0.1f))
        {
            rigidBodyComponent->SetMass(rigidBodyMass);
        }

        float friction = rigidBodyComponent->GetFriction();
        if (ImGui::DragFloat("Friction", &friction, 0.1f))
        {
            rigidBodyComponent->SetFriction(friction);
        }

        float rollingFriction = rigidBodyComponent->GetFrictionRolling();
        if (ImGui::DragFloat("Rolling Friction", &rollingFriction, 0.1f))
        {
            rigidBodyComponent->SetFrictionRolling(rollingFriction);
        }

        float restitution = rigidBodyComponent->GetRestitution();
        if (ImGui::DragFloat("Restitution", &restitution, 0.1f))
        {
            rigidBodyComponent->SetRestitution(restitution);
        }

        bool gravityState = rigidBodyComponent->GetGravityState();
        if (ImGui::Checkbox("Use Gravity", &gravityState))
        {
            rigidBodyComponent->SetUseGravity(gravityState);
        }

        bool kinematicState = rigidBodyComponent->GetKinematicState();
        if (ImGui::Checkbox("Kinematic", &kinematicState))
        {
            rigidBodyComponent->SetKinematicState(kinematicState);
        }

        /// Freeze Position
        /// Freeze Rotation
       
        ImGui::SliderFloat3("Force Amount", g_ForceAmount, 0.0f, 100.0f);
        if (ImGui::Button("Add Force"))
        {
            rigidBodyComponent->ApplyForce(XMFLOAT3(g_ForceAmount[0], g_ForceAmount[1], g_ForceAmount[2]), Aurora::ForceMode::ForceMode_Force);
        }

        if (ImGui::Button("Add Impulse"))
        {
            rigidBodyComponent->ApplyForce(XMFLOAT3(g_ForceAmount[0], g_ForceAmount[1], g_ForceAmount[2]), Aurora::ForceMode::ForceMode_Impulse);
        }

        ImGui::PopID();
    }

    ComponentEnd();
}