#include "EditorConsole.h"
#include "../Backend/Utilities/IconLibrary.h"
#include "../Backend/Utilities/Extensions.h"

EditorConsole::EditorConsole(Editor* editorContext, Aurora::EngineContext* engineContext) : Widget(editorContext, engineContext)
{
    m_WidgetName = "Startup Console";

    // Create an implementation of our console.
    m_EditorLogger = std::make_shared<EditorLogger>();
    m_EditorLogger->SetCallback([this](const Aurora::LogPackage& logPackage) { AddLogPackage(logPackage); });

    // Set the logger implementation for the engine to use.
    Aurora::Log::SetLogger(m_EditorLogger);
}

EditorConsole::~EditorConsole()
{
    Aurora::Log::m_EditorLogger.reset();
}

void EditorConsole::OnTickVisible()
{
    if (ImGui::Button("Clear Console"))
    {
        ClearConsole();
    }

    ImGui::SameLine();

    // Lambda for Info, Warning & Filter Buttons.
    const auto LogTypeVisibilityToggle = [this](const IconType iconType, uint32_t filterIndex)
    {
        bool& logVisibility = m_LogTypeVisibilityState[filterIndex];
        ImGui::PushStyleColor(ImGuiCol_Button, logVisibility ? ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered] : ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);
        if (EditorExtensions::ImageButton(iconType, 15.5f))
        {
            logVisibility = !logVisibility;
            m_ScrollToBottom = true;
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::Text("%d", m_LogTypeCount[filterIndex]);
        ImGui::SameLine(); 
    };

    LogTypeVisibilityToggle(IconType::IconType_Console_Info, 0); // Info
    LogTypeVisibilityToggle(IconType::IconType_Console_Warning, 1); // Warning
    LogTypeVisibilityToggle(IconType::IconType_Console_Error, 2); // Error

    // Text Filter
    const float labelWidth = 37.0f;
    m_LogFilter.Draw("Filter", ImGui::GetContentRegionAvail().x - labelWidth);
    ImGui::Separator();

    // Content Properties
    static const ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;
    static const ImVec2 size = ImVec2(-1.0f, -1.0f);

    // Content
    if (ImGui::BeginTable("##WidgetConsoleContent", 2, tableFlags, size))
    {
        for (uint32_t row = 0; row < m_Logs.size(); row++)
        {
            Aurora::LogPackage& logPackage = m_Logs[row];

            // Text and Visibility Filtering. We will show the log accordingly if the log's text passes the filter and its level is toggled.
            if (m_LogFilter.PassFilter(logPackage.m_Text.c_str()) && m_LogTypeVisibilityState[static_cast<int>(logPackage.m_LogType.first)])
            {
                // Switch Row
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                {
                    ImGui::Dummy(ImVec2(0.0f, 5.0f)); // To central-align our column icons.
                    ImGui::Image((void*)IconLibrary::GetInstance().GetTextureByType(logPackage.m_LogType.first == Aurora::LogType::Info ? IconType::IconType_Console_Info : logPackage.m_LogType.first == Aurora::LogType::Warning ? IconType::IconType_Console_Warning : IconType::IconType_Console_Error)->GetShaderResourceView().Get(), ImVec2(20.0f, 20.0f));
                }
                ImGui::TableSetColumnIndex(1);

                ImGui::PushID(row);
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, m_LogTypeColor[static_cast<int>(logPackage.m_LogType.first)]);
                    ImGui::BeginGroup();
                    ImGui::TextUnformatted(logPackage.EditorConsoleText().c_str());
                    ImGui::TextUnformatted(logPackage.m_LogSource.c_str());
                    ImGui::EndGroup();
                    // Context Menu
                    if (ImGui::BeginPopupContextItem("##WidgetConsoleContext"))
                    {
                        if (ImGui::MenuItem("Copy"))
                        {
                            ImGui::LogToClipboard();
                            ImGui::LogText("%s", logPackage.m_Text.c_str());
                            ImGui::LogFinish();
                        }

                        ImGui::Separator();

                        if (ImGui::MenuItem("Search"))
                        {
                            Aurora::FileSystem::OpenDirectoryWindow("https://www.google.com/search?q=" + logPackage.m_Text);
                        }

                        ImGui::EndPopup();
                    }
                    ImGui::PopStyleColor();                
                }
                ImGui::PopID();
            }
        }

        // Scroll content to the latest entry 
        if (m_ScrollToBottom)
        {
            ImGui::SetScrollHereY();
            m_ScrollToBottom = false;
        }

        ImGui::EndTable();
    }  
}

void EditorConsole::AddLogPackage(const Aurora::LogPackage& logPackage)
{
    // Save to Dequeue
    m_Logs.emplace_back(logPackage);
    if (static_cast<uint32_t>(m_Logs.size()) > m_LogMaximumCount)
    {
        m_Logs.pop_front();
    }

    // Update count.
    m_LogTypeCount[static_cast<int>(logPackage.m_LogType.first)]++;

    if (m_LogTypeVisibilityState[static_cast<int>(logPackage.m_LogType.first)])
    {
        m_ScrollToBottom = true;
    }
}

void EditorConsole::ClearConsole()
{
    m_Logs.clear();
    m_Logs.shrink_to_fit();

    m_LogTypeCount[0] = 0;
    m_LogTypeCount[1] = 0;
    m_LogTypeCount[2] = 0;
}