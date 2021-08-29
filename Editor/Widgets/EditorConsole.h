#pragma once
#include "../Backend/Widget.h"
#include "../Log/ILogger.h"
#include <DirectXMath.h>
#include <functional>
#include <deque>

using namespace DirectX;
// Implementation of ILogger
class EditorLogger : public Aurora::ILogger
{
public:
    typedef std::function<void(Aurora::LogPackage)> LogFunction;

    void SetCallback(LogFunction&& logFunction)
    {
        m_LogFunction = std::forward<LogFunction>(logFunction);
    }

    void LogMessage(const Aurora::LogPackage& logPackage) override
    {
        m_LogFunction(logPackage);
    }

private:
    LogFunction m_LogFunction;
};

class EditorConsole : public Widget
{
public:
    EditorConsole(Editor* editorContext, Aurora::EngineContext* engineContext);
    ~EditorConsole() override;

    void OnTickVisible() override;

    // Console Specific
    void AddLogPackage(const Aurora::LogPackage& logPackage);
    void ClearConsole();

private:
    std::deque<Aurora::LogPackage> m_Logs;
    uint32_t m_LogMaximumCount = 1000;

    // Log Types
    uint32_t m_LogTypeCount[3] = { 0, 0, 0 }; //Info, Warning & Error
    const std::vector<ImVec4> m_LogTypeColor =
    {
        ImVec4(1.0f, 1.0f, 1.0f, 1.0f),  // Info
        ImVec4(1.0f, 1.0f, 0.4f, 1.0f),  // Warning
        ImVec4(1.0f, 0.0f, 0.0f, 1.0f)   // Error
    };

    // Filtering
    bool m_ScrollToBottom = false;
    bool m_LogTypeVisibilityState[3] = { true, true, true }; // For filtering purposes.
    ImGuiTextFilter m_LogFilter;

    std::shared_ptr<EditorLogger> m_EditorLogger;
};