#include "ProjectSettings.h"
#include "Settings.h"

float g_ColumnWidth = 150.0f;

ProjectSettings::ProjectSettings(Editor* editorContext, Aurora::EngineContext* engineContext) : Widget(editorContext, engineContext)
{
    m_SettingsSubsystem = m_EngineContext->GetSubsystem<Aurora::Settings>();
}

void PropertyInput(const std::string& labelText, std::string& stringValue)
{
    ImGui::Text(labelText.c_str());
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);

    std::string stringID = "##" + labelText;
    char inputTextBuffer[256];
    memset(inputTextBuffer, 0, sizeof(inputTextBuffer));
    strcpy_s(inputTextBuffer, sizeof(inputTextBuffer), stringValue.c_str());

    if (ImGui::InputText(stringID.c_str(), inputTextBuffer, sizeof(inputTextBuffer)))
    {
        stringValue = inputTextBuffer;
    }

    ImGui::PopItemWidth();
    ImGui::NextColumn();
}

void ProjectSettings::OnTickVisible()
{
    ImGui::Columns(2);
    ImGui::AlignTextToFramePadding();

    PropertyInput("Application Name", m_SettingsSubsystem->GetApplicationName());
    PropertyInput("Project Directory", m_SettingsSubsystem->GetProjectDirectory());

    ImGui::Columns(1); // Reset

    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Resource Directories", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Columns(2);
        ImGui::AlignTextToFramePadding();

        for (auto& resourceDirectory : m_SettingsSubsystem->GetResourceDirectories())
        {
            PropertyInput(Aurora::ToString(resourceDirectory.first), resourceDirectory.second);
        }

        ImGui::Columns(1); // Reset
    }
}
/*
ImGui::Columns(2);
ImGui::SetColumnWidth(0, g_ColumnWidth);
ImGui::Text("Project Directory");
ImGui::NextColumn();

char projectDirectoryBuffer[256];
memset(projectDirectoryBuffer, 0, sizeof(projectDirectoryBuffer));
strcpy_s(projectDirectoryBuffer, sizeof(projectDirectoryBuffer), m_SettingsSubsystem->GetProjectDirectory().c_str());

if (ImGui::InputText("##Project Directory", projectDirectoryBuffer, sizeof(projectDirectoryBuffer)))
{
    m_SettingsSubsystem->SetProjectDirectory(projectDirectoryBuffer);
}

ImGui::Columns(1);

if (ImGui::CollapsingHeader("Resource Directories", ImGuiTreeNodeFlags_DefaultOpen))
{
    for (const std::pair<Aurora::ResourceDirectory, std::string>& resourceDirectory : m_SettingsSubsystem->GetResourceDirectories())
    {
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, g_ColumnWidth);
        ImGui::Text(Aurora::ToString(resourceDirectory.first));
        ImGui::NextColumn();

        char directoryBuffer[256];
        memset(directoryBuffer, 0, sizeof(directoryBuffer));
        strcpy_s(directoryBuffer, sizeof(directoryBuffer), resourceDirectory.second.c_str());

        if (ImGui::InputText(Aurora::ToString(resourceDirectory.first), directoryBuffer, sizeof(directoryBuffer)))
        {
            m_SettingsSubsystem->SetResourceDirectory(resourceDirectory.first, std::string(directoryBuffer));
        }

        ImGui::Columns(1);
    }
}
*/