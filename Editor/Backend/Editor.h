#pragma once
#include <memory>
#include "Engine.h"
#include "EngineContext.h"
#include "Widget.h"
#include <optional>
#include "../Widgets/EditorTools.h"

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
	// Engine Contexts
	std::unique_ptr<Aurora::Engine> m_Engine;
	Aurora::EngineContext* m_EngineContext = nullptr; // Consists of subsystems.

	// Editor Contexts
	bool m_EditorBegun = false;
	std::shared_ptr<EditorTools> m_EditorTools;
	std::vector<std::shared_ptr<Widget>> m_Widgets;
};