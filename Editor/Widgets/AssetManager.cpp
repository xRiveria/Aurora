#include "AssetManager.h"
#include "../Backend/Utilities/Extensions.h"
#include "../Resource/ResourceCache.h"

AssetManager::AssetManager(Editor* editorContext, Aurora::EngineContext* engineContext) : Widget(editorContext, engineContext)
{
    m_ResourceCache = m_EngineContext->GetSubsystem<Aurora::ResourceCache>();
    m_WidgetName = "Resource Manager";
}

void AssetManager::OnTickVisible()
{
    ImGui::Columns(2);
    ImGui::AlignTextToFramePadding();

    uint32_t cacheSize = static_cast<uint32_t>(m_ResourceCache->GetCachedResources().size());
    for (int i = 0; i < cacheSize; i++)
    {
        auto& resource = m_ResourceCache->GetCachedResources()[i];

        ImGui::PushID(resource->GetObjectName().c_str());

        char resourceHandle[256];
        sprintf_s(resourceHandle, "%u", resource->GetObjectID());
        std::string buffer = resourceHandle;

        std::string objectName = resource->GetObjectName();
        std::string resourceType = resource->TypeToString();

        PropertyInput("Handle", buffer);
        PropertyInput("File Path", objectName);
        PropertyInput("Resource Type", resourceType);

        if (i + 1 != cacheSize)
        {
            ImGui::Separator();
        }

        ImGui::PopID();
    }

    ImGui::Columns(1); // Reset
    ImGui::Spacing();
}
