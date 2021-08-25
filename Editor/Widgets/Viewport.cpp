#include "Viewport.h"
#include "../Threading/Threading.h"
#include "../Scene/World.h"
#include "../Input/Input.h"
#include "../Renderer/Model.h"
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
    Aurora::AURORA_PROFILE_FUNCTION();

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

    ImGui::Image((void*)m_RendererSubsystem->m_DeviceContext->m_ResolveFramebuffer->m_RenderTargetTexture->GetShaderResourceView().Get(), ImVec2(width, height));

    // Handle model dropping.
    if (auto payload = EditorExtensions::ReceiveDragPayload(EditorExtensions::DragPayloadType::DragPayloadType_Entity))
    {
        if (std::get<Aurora::DefaultObjectType>(payload->m_Data) == Aurora::DefaultObjectType::DefaultObjectType_PointLight)
        {
            std::shared_ptr<Aurora::Entity> entity = m_WorldSubsystem->EntityCreate();
            entity->SetEntityName("DefaultPointLight");
            entity->AddComponent<Aurora::Light>();
        }
        else
        {
            m_WorldSubsystem->CreateDefaultObject(std::get<Aurora::DefaultObjectType>(payload->m_Data));
        }
    }
    else if (auto payload = EditorExtensions::ReceiveDragPayload(EditorExtensions::DragPayloadType::DragPayloadType_Model))
    {
        const std::string filePath = std::get<const char*>(payload->m_Data); // Retrieve data from main thread

        // Load with seperate thread.
        m_EngineContext->GetSubsystem<Aurora::Threading>()->Execute([this, filePath](Aurora::JobInformation jobInformation)
            {
                m_EngineContext->GetSubsystem<Aurora::ResourceCache>()->Load<Aurora::Model>(filePath);
            });
    }
          
    m_EditorContext->GetWidget<EditorTools>()->OnTickViewport();
}
