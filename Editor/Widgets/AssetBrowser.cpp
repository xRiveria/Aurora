#include "AssetBrowser.h"
#include "../Backend/Utilities/Extensions.h"
#include "Settings.h"
#include "../Backend/Utilities/FileDialog.h"
#include "../Resource/ResourceCache.h"
#include "../Renderer/Material.h"
#include "Properties.h"

namespace AssetBrowserGlobals
{
    static bool g_ShowFileDialogView = true;
    static bool g_ShowFileDialogLoad = false;
    static std::string g_DoubleClickedPathImportDialog;
}

AssetBrowser::AssetBrowser(Editor * editorContext, Aurora::EngineContext * engineContext) : Widget(editorContext, engineContext)
{
    m_WidgetName = "Asset Browser";
    m_SettingsSubsystem = m_EngineContext->GetSubsystem<Aurora::Settings>();
    m_FileDialog = std::make_unique<FileDialog>(m_EngineContext, m_EditorContext, false);
    m_WidgetFlags |= ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    // Just clicked, not selected.
    m_FileDialog->SetOnItemClickedCallback([this](const std::string& filePath) { OnPathClicked(filePath); });
}

void AssetBrowser::OnTickVisible()
{
    Aurora::AURORA_PROFILE_FUNCTION();

    ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable;

    if (ImGui::BeginTable("AssetBrowser", 2, tableFlags))
    {
        ImGui::TableSetupColumn("DirectoryList", ImGuiTableColumnFlags_WidthStretch, 40.0f, 0);

        ImGui::TableNextColumn();
        m_FileDialog->ShowDialogDirectory();

        ImGui::TableNextColumn();
        m_FileDialog->ShowDialog(&AssetBrowserGlobals::g_ShowFileDialogView);

        ImGui::EndTable();
    }
}

void AssetBrowser::OnPathClicked(const std::string& filePath) const
{
    if (!Aurora::FileSystem::IsFile(filePath))
    {
        return;
    }

    if (Aurora::FileSystem::IsEngineMaterialFile(filePath))
    {
        const auto material = m_EngineContext->GetSubsystem<Aurora::ResourceCache>()->Load<Aurora::Material>(filePath);
        Properties::Inspect(material);
    }
}
