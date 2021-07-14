#pragma once
#include <memory>
#include "Engine.h"
#include "EngineContext.h"
#include "../Graphics/RHI_Implementation.h"
#include <optional>

class Editor
{
public:
	Editor();
	~Editor();

	void Tick();

private:
	void InitializeEditor();
	void BeginDockingContext();

	void ImGuiImplementation_Initialize(Aurora::EngineContext* engineContext);
	void ImGuiImplementation_Shutdown();
	void ImGuiImplementation_ApplyStyling();

private:
	std::optional<std::string> OpenFilePath(const char* filter); // Temporary

private:
	// Engine Contexts
	std::unique_ptr<Aurora::Engine> m_Engine;
	Aurora::EngineContext* m_EngineContext = nullptr; // Consists of subsystems.

	// Editor Contexts
	bool m_EditorBegun = false;
	/// std::vector<std::shared_ptr<Widget>> m_Widgets;
};