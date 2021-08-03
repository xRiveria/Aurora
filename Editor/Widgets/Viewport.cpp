#include "Viewport.h"
#include "../Scene/World.h"
#include "../Input/Input.h"
#include "../Scene/Components/Light.h"
#include "../Renderer/Renderer.h"
#include "../Backend/Utilities/Extensions.h"
#include "../Backend/Editor.h"

Viewport::Viewport(Editor* editorContext, Aurora::EngineContext* engineContext) : Widget(editorContext, engineContext)
{
    m_WidgetName = "Viewport";
    m_WidgetFlags |= ImGuiWindowFlags_NoScrollbar;

    m_WidgetPadding = 0.0f;
    m_WorldSubsystem = m_EngineContext->GetSubsystem<Aurora::World>();
    m_InputSubsystem = m_EngineContext->GetSubsystem<Aurora::Input>();
    m_RendererSubsystem = m_EngineContext->GetSubsystem<Aurora::Renderer>();
}

void Viewport::OnTickVisible()
{
    if (!m_RendererSubsystem)
    {
        return;
    }

    // Get Size
    float width = static_cast<float>(ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x);
    float height = static_cast<float>(ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y);

    // Update engine's viewport.
    if (m_Width != width || m_Height != height)
    {
        m_RendererSubsystem->SetRenderDimensions(width, height);

        m_Width = width;
        m_Height = height;
    }

    auto internalState = Aurora::DX11_Utility::ToInternal(&m_RendererSubsystem->m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Color]);
    ImGui::Image((void*)internalState->m_ShaderResourceView.Get(), ImVec2(width, height));

    // Handle model dropping.
    if (auto payload = EditorExtensions::ReceiveDragPayload(EditorExtensions::DragPayloadType::DragPayloadType_Entity))
    {
        if (std::get<Aurora::DefaultObjectType>(payload->m_Data) == Aurora::DefaultObjectType::DefaultObjectType_PointLight)
        {
            std::shared_ptr<Aurora::Entity> entity = m_WorldSubsystem->EntityCreate();
            entity->SetName("DefaultPointLight");
            entity->AddComponent<Aurora::Light>();
        }
        else
        {
            m_WorldSubsystem->CreateDefaultObject(std::get<Aurora::DefaultObjectType>(payload->m_Data));
        }
    }
   
    m_EditorContext->GetWidget<EditorTools>()->OnTickViewport();
}
