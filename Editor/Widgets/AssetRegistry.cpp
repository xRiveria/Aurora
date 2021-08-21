#include "AssetRegistry.h"
#include "../Backend/Utilities/Extensions.h"
#include "../Resource/ResourceCache.h"

bool g_FilterByType = false;

AssetRegistry::AssetRegistry(Editor* editorContext, Aurora::EngineContext* engineContext) : Widget(editorContext, engineContext)
{
    m_ResourceCache = m_EngineContext->GetSubsystem<Aurora::ResourceCache>();
    m_WidgetName = "Asset Registry";
}

void AssetRegistry::OnTickVisible()
{
    ImGui::Checkbox("Filter By Type", &g_FilterByType);

    m_RegistryFilter.Draw("##RegisterFilter", ImGui::GetContentRegionAvailWidth());

    ImGui::Columns(2);
    ImGui::AlignTextToFramePadding();


    uint32_t cacheSize = static_cast<uint32_t>(m_ResourceCache->m_CachedResources.size());
    for (uint32_t i = 0; i < cacheSize; i++)
    {
        Aurora::AuroraResource* resource = m_ResourceCache->m_CachedResources[i].get();

        // Apply search filter.
        if (!g_FilterByType)
        {
            if (!m_RegistryFilter.PassFilter(resource->GetObjectName().c_str()))
            {
                continue;
            }
        }
        else
        {
            if (!m_RegistryFilter.PassFilter(resource->GetResourceTypeInCString()))
            {
                continue;
            }
        }

        ImGui::PushID(resource->GetObjectName().c_str());

        char resourceHandle[256];
        sprintf_s(resourceHandle, "%u", resource->GetObjectID());
        std::string buffer = resourceHandle;

        std::string objectName = resource->GetResourceName();
        std::string resourceType = resource->GetResourceTypeInCString();
        std::string resourceFilePathForeign = resource->GetResourceFilePath();
        std::string resourceFilePathNative = resource->GetResourceFilePathNative();

        PropertyInput("Handle", buffer);
        PropertyInput("File Name", objectName);
        PropertyInput("Resource Type", resourceType);
        PropertyInput("Resource Path (Foreign)", resourceFilePathForeign);
        PropertyInput("Resource Path (Native)", resourceFilePathNative);

        if (i + 1 != cacheSize)
        {
            ImGui::Separator();
        }
     
        ImGui::PopID();
    }

    ImGui::Columns(1); // Reset
    ImGui::Spacing();
}
