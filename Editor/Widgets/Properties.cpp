#include "Properties.h"
#include "../Scene/Entity.h"
#include "../Scene/Components/Transform.h"
#include "../Scene/Components/Light.h"
#include "../Scene/Components/RigidBody.h"
#include "../Scene/Components/Renderable.h"
#include "../Scene/Components/Collider.h"
#include "../Scene/Components/AudioSource.h"
#include "../Audio/AudioClip.h"
#include "../Scene/World.h"
#include "../Threading/Threading.h"
#include "../Renderer/Model.h"
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
    m_ResourceCache = engineContext->GetSubsystem<Aurora::ResourceCache>();
}

void Properties::OnTickVisible()
{
    Aurora::AURORA_PROFILE_FUNCTION();

    if (!m_InspectedEntity.expired())
    {
        Aurora::Entity* entityPointer = m_InspectedEntity.lock().get();
        Aurora::Renderable* renderablePointer = entityPointer->GetComponent<Aurora::Renderable>();
        Aurora::Material* materialPointer = renderablePointer ? renderablePointer->GetMaterial() : nullptr;

        ImGui::Checkbox("##EntityCheckbox", &entityPointer->m_IsActive);
        ImGui::SameLine();

        char entityNameBuffer[256] = "Entity";
        memset(entityNameBuffer, 0, sizeof(entityNameBuffer));
        strcpy_s(entityNameBuffer, sizeof(entityNameBuffer), entityPointer->GetEntityName().c_str());

        if (ImGui::InputText("##EntityName", entityNameBuffer, sizeof(entityNameBuffer)))
        {
            entityPointer->SetEntityName(std::string(entityNameBuffer));
        }
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4({ 0.65f, 0.65f, 0.65f, 1.0f }));
        ImGui::Text("Entity ID: %d", entityPointer->GetObjectID());
        ImGui::PopStyleColor();

        ShowTransformProperties(entityPointer->GetComponent<Aurora::Transform>());
        ShowRenderableProperties(renderablePointer);
        ShowMaterialProperties(materialPointer);
        ShowLightProperties(entityPointer->GetComponent<Aurora::Light>());
        ShowColliderProperties(entityPointer->GetComponent<Aurora::Collider>());
        ShowRigidBodyProperties(entityPointer->GetComponent<Aurora::RigidBody>());
        ShowAudioSourceProperties(entityPointer->GetComponent<Aurora::AudioSource>());

        ShowAddComponentButton();
    }
    else if (!m_InspectedMaterial.expired())
    {
        ShowMaterialProperties(m_InspectedMaterial.lock().get());
    }
}

void Properties::OnEvent(Aurora::InputEvent& inputEvent)
{
    Aurora::InputEventDispatcher dispatcher(inputEvent);
    dispatcher.Dispatch<Aurora::KeyPressedEvent>(AURORA_BIND_INPUT_EVENT(Properties::OnKeyPressed));
}

bool Properties::OnKeyPressed(Aurora::KeyPressedEvent& keyPressedEvent)
{
    if (keyPressedEvent.GetRepeatCount() > 0)
    {
        return false;
    }

    bool isControlPressed = m_EngineContext->GetSubsystem<Aurora::Input>()->IsKeyPressed(AURORA_KEY_LEFT_CONTROL) || m_EngineContext->GetSubsystem<Aurora::Input>()->IsKeyPressed(AURORA_KEY_RIGHT_CONTROL);

    switch (keyPressedEvent.GetKeyCode())
    {
        case AURORA_KEY_D:
        {
            if (!m_InspectedEntity.expired())
            {
                if (isControlPressed) { AURORA_INFO(Aurora::LogLayer::ECS, "Cloned Entity!"); m_InspectedEntity.lock()->Clone(); }
                return true;
            }
            break;
        }

        case AURORA_KEY_DELETE:
        {
            if (!m_InspectedEntity.expired())
            {
                m_InspectedEntity.lock().get()->MarkForDestruction();
                m_EngineContext->GetSubsystem<Aurora::World>()->SetSceneDirty();
                m_InspectedEntity.reset();
                return true;
            }
            break;
        }
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
            if (ImGui::MenuItem("Audio Source"))
            {
                entity->AddComponent<Aurora::AudioSource>();
            }

             if (ImGui::MenuItem("Light"))
             {
                 entity->AddComponent<Aurora::Light>();
             }

             if (ImGui::MenuItem("Rigidbody"))
             {
                 entity->AddComponent<Aurora::RigidBody>();
             }

             if (ImGui::MenuItem("Box Collider"))
             {
                 Aurora::Collider* collider = entity->AddComponent<Aurora::Collider>();
                 collider->SetShapeType(Aurora::ColliderShape::ColliderShape_Box);
             }

             if (ImGui::MenuItem("Sphere Collider"))
             {
                 Aurora::Collider* collider = entity->AddComponent<Aurora::Collider>();
                 collider->SetShapeType(Aurora::ColliderShape::ColliderShape_Sphere);
             }

             if (ImGui::MenuItem("Static Plane Collider"))
             {
                 Aurora::Collider* collider = entity->AddComponent<Aurora::Collider>();
                 collider->SetShapeType(Aurora::ColliderShape::ColliderShape_StaticPlane);
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

    if (materialTexture != nullptr)
    {
        ImGui::Image((void*)materialTexture->GetShaderResourceView().Get(), ImVec2(60, 60));

        if (auto dragPayload = EditorExtensions::ReceiveDragPayload(EditorExtensions::DragPayloadType::DragPayloadType_Texture))
        {
            const std::string filePath = std::get<const char*>(dragPayload->m_Data);

            engineContext->GetSubsystem<Aurora::Threading>()->Execute([TextureSetter, engineContext, filePath](Aurora::JobInformation jobInformation) mutable
            {
                auto texture = engineContext->GetSubsystem<Aurora::ResourceCache>()->Load<Aurora::DX11_Texture>(filePath);
                TextureSetter(texture);
            });
        }
    }
    else
    {
        ImGui::Image((void*)engineContext->GetSubsystem<Aurora::Renderer>()->m_DefaultWhiteTexture->GetShaderResourceView().Get(), ImVec2(60, 60));

        if (auto dragPayload = EditorExtensions::ReceiveDragPayload(EditorExtensions::DragPayloadType::DragPayloadType_Texture))
        {
            const std::string filePath = std::get<const char*>(dragPayload->m_Data);          
            engineContext->GetSubsystem<Aurora::Threading>()->Execute([TextureSetter, engineContext, filePath](Aurora::JobInformation jobInformation) mutable
            {
                auto texture = engineContext->GetSubsystem<Aurora::ResourceCache>()->Load<Aurora::DX11_Texture>(filePath);
                TextureSetter(texture);
            });
        }
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
    if (materialTexture != nullptr)
    {
        ImGui::Text("File: %s", materialTexture->GetResourceFilePathNative().c_str());
    }
    else
    {
        ImGui::Text("File: %s", "Default");
    }
    
    ImGui::Separator();
    ImGui::PopID();
}

void Properties::ShowRenderableProperties(Aurora::Renderable* renderableComponent) const
{
    if (!renderableComponent)
    {
        return;
    }

    if (ComponentBegin("Renderable"))
    {
        if (Aurora::Model* modelPointer = renderableComponent->GetGeometryModel())
        {
            char meshNameBuffer[256] = "Mesh File";
            memset(meshNameBuffer, 0, sizeof(meshNameBuffer));
            strcpy_s(meshNameBuffer, renderableComponent->GetGeometryModel()->GetResourceFilePathNative().c_str());

            ImGui::InputText("Mesh File", meshNameBuffer, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_ReadOnly);
        }

        if (Aurora::Material* materialPointer = renderableComponent->GetMaterial())
        {
            char materialNameBuffer[256] = "Material File";
            memset(materialNameBuffer, 0, sizeof(materialNameBuffer));
            strcpy_s(materialNameBuffer, sizeof(materialNameBuffer), materialPointer->GetResourceFilePathNative().c_str());

            ImGui::InputText("Material File", materialNameBuffer, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_ReadOnly);
            if (auto payload = EditorExtensions::ReceiveDragPayload(EditorExtensions::DragPayloadType::DragPayloadType_Material))
            {
                renderableComponent->SetMaterial(std::get<const char*>(payload->m_Data));
            }
        }
    }

    ComponentEnd();
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

        DrawMaterialControl("Albedo Map", materialComponent->m_Textures[Aurora::MaterialSlot::MaterialSlot_Albedo].get(), m_EngineContext, [materialComponent](const std::shared_ptr<Aurora::DX11_Texture>& texture) mutable { materialComponent->SetTextureSlot(Aurora::MaterialSlot::MaterialSlot_Albedo, texture); }, true, materialComponent->m_AlbedoColor);
        DrawMaterialControl("Roughness Map", materialComponent->m_Textures[Aurora::MaterialSlot::MaterialSlot_Roughness].get(), m_EngineContext, [materialComponent](const std::shared_ptr<Aurora::DX11_Texture>& texture) mutable { materialComponent->SetTextureSlot(Aurora::MaterialSlot::MaterialSlot_Roughness, texture); });
        DrawMaterialControl("Normal Map", materialComponent->m_Textures[Aurora::MaterialSlot::MaterialSlot_Normal].get(), m_EngineContext, [materialComponent](const std::shared_ptr<Aurora::DX11_Texture>& texture) mutable { materialComponent->SetTextureSlot(Aurora::MaterialSlot::MaterialSlot_Normal, texture); });
        DrawMaterialControl("Metallic Map", materialComponent->m_Textures[Aurora::MaterialSlot::MaterialSlot_Metallic].get(), m_EngineContext, [materialComponent](const std::shared_ptr<Aurora::DX11_Texture>& texture) mutable { materialComponent->SetTextureSlot(Aurora::MaterialSlot::MaterialSlot_Metallic, texture); });
        DrawMaterialControl("Occlusion Map", materialComponent->m_Textures[Aurora::MaterialSlot::MaterialSlot_Occlusion].get(), m_EngineContext, [materialComponent](const std::shared_ptr<Aurora::DX11_Texture>& texture) mutable { materialComponent->SetTextureSlot(Aurora::MaterialSlot::MaterialSlot_Occlusion, texture); });

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

void Properties::ShowAudioSourceProperties(Aurora::AudioSource* audioSourceComponent) const
{
    if (!audioSourceComponent)
    {
        return;
    }

    if (ComponentBegin("Audio Source"))
    {
        ImGui::PushID(audioSourceComponent->GetObjectID());

        char audioClipNameBuffer[256];
        memset(audioClipNameBuffer, 0, sizeof(audioClipNameBuffer));
        strcpy_s(audioClipNameBuffer, sizeof(audioClipNameBuffer), audioSourceComponent->GetAudioClipPath().c_str());

        ImGui::InputText("Audio Clip", audioClipNameBuffer, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_ReadOnly);
        if (auto payload = EditorExtensions::ReceiveDragPayload(EditorExtensions::DragPayloadType::DragPayloadType_AudioClip))
        {
            const std::string filePath = std::get<const char*>(payload->m_Data);
            
            m_EngineContext->GetSubsystem<Aurora::Threading>()->Execute([=](Aurora::JobInformation jobInformation)
            {
                audioSourceComponent->SetAudioClip(filePath);
            });

            // Stop playing if there's anything currently doing so.
            audioSourceComponent->Stop();
        }

        float volume = audioSourceComponent->GetVolume();
        if (ImGui::SliderFloat("Volume", &volume, 0.0f, 1.0f, "%.2f"))
        {
            audioSourceComponent->SetVolume(volume);
        }

        int priority = audioSourceComponent->GetPriority();
        if (ImGui::SliderInt("Priority", &priority, 0.0f, 255.0f))
        {
            audioSourceComponent->SetPriority(priority);
        }

        float pitch = audioSourceComponent->GetPitch();
        if (ImGui::SliderFloat("Pitch", &pitch, 0.0f, 3.0f, "%0.2f"))
        {
            audioSourceComponent->SetPitch(pitch);
        }

        float pan = audioSourceComponent->GetPan();
        if (ImGui::SliderFloat("Pan", &pan, -1.0f, 1.0f, "%0.2f"))
        {
            audioSourceComponent->SetPan(pan);
        }

        float distanceMinimum = audioSourceComponent->GetRolloffDistanceMinimum();
        if (ImGui::DragFloat("Rolloff Minimum", &distanceMinimum, 0.1f))
        {
            audioSourceComponent->SetRolloffDistanceMinimum(distanceMinimum);
        }

        float distanceMaximum = audioSourceComponent->GetRolloffDistanceMaximum();
        if (ImGui::DragFloat("Rolloff Maximum", &distanceMaximum, 0.1f))
        {
            audioSourceComponent->SetRolloffDistanceMaximum(distanceMaximum);
        }
            
        bool playOnStart = audioSourceComponent->GetPlayOnStartState();
        if (ImGui::Checkbox("Play On Start", &playOnStart))
        {
            audioSourceComponent->SetPlayOnStartState(playOnStart);
        }

        bool loopState = audioSourceComponent->GetLoopState();
        if (ImGui::Checkbox("Is Looping", &loopState))
        {
            audioSourceComponent->SetLoopState(loopState);

            if (!loopState)
            {
                audioSourceComponent->Stop();
            }
        }

        bool muteState = audioSourceComponent->GetMuteState();
        if (ImGui::Checkbox("Muted", &muteState))
        {
            audioSourceComponent->SetMuteState(muteState);
        }

        if (ImGui::Button("Play Clip"))
        {
            if (audioSourceComponent->GetAudioClip())
            {
                if (audioSourceComponent->GetAudioClip()->IsPlaying())
                {
                    audioSourceComponent->Stop();
                }

                audioSourceComponent->Play();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop Clip"))
        {
            audioSourceComponent->Stop();
        }
        ImGui::SameLine();
        if (ImGui::Button("Pause Clip"))
        {
            audioSourceComponent->Pause();
        }
        ImGui::SameLine();
        if (ImGui::Button("Unpause Clip"))
        {
            audioSourceComponent->Unpause();
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
            rigidBodyComponent->SetGravityState(gravityState);
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