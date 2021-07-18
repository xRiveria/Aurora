#include "Properties.h"
#include "../Scene/Entity.h"
#include "../Scene/Components/Transform.h"
#include "../Scene/Components/Material.h"
#include "../Scene/Components/Mesh.h"
#include "../Backend/Source/imgui_internal.h"
#include "../Renderer/Renderer.h"
#include "../Backend/Extensions.h"
#include "FileSystem.h"

std::weak_ptr<Aurora::Entity> Properties::m_InspectedEntity;
std::weak_ptr<Aurora::Material> Properties::m_InspectedMaterial;

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
        Aurora::Mesh* meshPointer = entityPointer->GetComponent<Aurora::Mesh>();
        Aurora::Material* materialPointer = meshPointer ? entityPointer->GetComponent<Aurora::Material>() : nullptr;

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

        ShowTransformProperties(entityPointer->GetComponent<Aurora::Transform>());
        ShowMaterialProperties(materialPointer);

        ShowAddComponentButton();
    }
}

void Properties::Inspect(const std::weak_ptr<Aurora::Entity>& entity)
{
    m_InspectedEntity = entity;

    /// Rotation.
}

void Properties::Inspect(const std::weak_ptr<Aurora::Material>& material)
{
    m_InspectedEntity.reset();
    m_InspectedMaterial = material;
}

void Properties::ShowAddComponentButton() const
{
    if (ImGui::Button("Add Component"))
    {
        
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
    ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
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
    ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
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
    ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();

    ImGui::PopStyleVar();
    ImGui::Columns(1);
    ImGui::PopID();
}

static void DrawMaterialControl(const std::string& label, Aurora::TextureMap& materialTexture, Aurora::EngineContext* engineContext)
{
    ImGuiIO& io = ImGui::GetIO();
    auto boldFont = io.Fonts->Fonts[0];
    ImGui::PushFont(boldFont);
    ImGui::Text(label.c_str());
    ImGui::PopFont();

    if (materialTexture.m_Resource != nullptr)
    {
        Aurora::DX11_Utility::DX11_TexturePackage* texture = Aurora::DX11_Utility::ToInternal(&materialTexture.m_Resource->m_Texture);
        ImGui::Image((void*)texture->m_ShaderResourceView.Get(), ImVec2(60, 60));
    }
    else
    {
        Aurora::DX11_Utility::DX11_TexturePackage* texture = Aurora::DX11_Utility::ToInternal(&engineContext->GetSubsystem<Aurora::Renderer>()->m_DefaultWhiteTexture->m_Texture);
        ImGui::Image((void*)texture->m_ShaderResourceView.Get(), ImVec2(60, 60));
    }

    ImGui::SameLine();

    bool isTrue = true; ///
    ImGui::Checkbox("Use", &isTrue);
    
    ImGui::SameLine();

    if (ImGui::Button("Load..."))
    {
        std::optional<std::string> filePath = OpenFilePath("Textures", engineContext);
        if (filePath.has_value())
        {
            std::string path = filePath.value();
            std::string fileName = Aurora::FileSystem::GetFileNameFromFilePath(path);
            std::shared_ptr<Aurora::AuroraResource> resource = engineContext->GetSubsystem<Aurora::ResourceCache>()->LoadTexture(path, fileName);
            materialTexture.m_FilePath = fileName; /// We need to streamline AuroraResource and TextureMap.
            materialTexture.m_Resource = resource;
        }
    }

    ImGui::SameLine(); ImGui::NewLine();
    ImGui::Text("File: %s", materialTexture.m_FilePath.c_str());

    ImGui::Separator();
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

        // Converts our radians to degrees for display in the inspector, and translates its new values back to radians.
        XMFLOAT3 rotation = { transformComponent->m_RotationLocal.x, transformComponent->m_RotationLocal.y, transformComponent->m_RotationLocal.z };
        DrawVector3Control("Rotation", rotation);
        transformComponent->RotateRollPitchYaw(rotation);

        DrawVector3Control("Scale", transformComponent->m_ScaleLocal, 1.0f);
    }

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
        DrawMaterialControl("Albedo Map", materialComponent->m_Textures[Aurora::TextureSlot::BaseColorMap], m_EngineContext);
        DrawMaterialControl("Normal Map", materialComponent->m_Textures[Aurora::TextureSlot::NormalMap], m_EngineContext);
        DrawMaterialControl("Emissive Map", materialComponent->m_Textures[Aurora::TextureSlot::EmissiveMap], m_EngineContext);
        DrawMaterialControl("Occulusion Map", materialComponent->m_Textures[Aurora::TextureSlot::OcclusionMap], m_EngineContext);
    }

    ComponentEnd();
}
