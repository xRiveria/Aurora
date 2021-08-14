#include "AssetBrowser.h"
#include "../Backend/Utilities/Extensions.h"
#include "Settings.h"
#include "../Backend/Utilities/FileDialog.h"

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

}

void AssetBrowser::OnTickVisible()
{
    m_FileDialog->ShowDialog(&AssetBrowserGlobals::g_ShowFileDialogView);
}

void AssetBrowser::OnPathClicked(const std::string& filePath) const
{
}
