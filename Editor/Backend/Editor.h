#pragma once
#include <memory>
#include "Engine.h"
#include "EngineContext.h"
#include "Widget.h"
#include <optional>
#include "../Widgets/EditorTools.h"
#include "../Widgets/EditorConsole.h"
#include "../Backend/Utilities/Extensions.h"

class EditorSubsystem : public Aurora::ISubsystem
{
public:
	EditorSubsystem(Aurora::EngineContext* engineContext);
	~EditorSubsystem() {}

	void SetEditorInstance(Editor* editor) { m_Editor = editor; }

	void OnEvent(Aurora::InputEvent& inputEvent) override;
	bool OnKeyPressed(Aurora::KeyPressedEvent& inputEvent);

private:
	Editor* m_Editor;
};

class Editor
{
public:
	Editor();
	~Editor();

	void Tick();

	Aurora::EngineContext* GetEngineContext() const { return m_EngineContext; }
	std::vector<std::shared_ptr<Widget>>& GetWidgets() { return m_Widgets; }

	template<typename T>
	T* GetWidget() const
	{
		for (const auto& widget : m_Widgets)
		{
			if (typeid(T) == typeid(*widget))
			{
				return static_cast<T*>(widget.get());
			}	
		}
		return nullptr;
	}

private:
	void InitializeEditor();
	void BeginDockingContext();

	void ImGuiImplementation_Initialize(Aurora::EngineContext* engineContext);
	void ImGuiImplementation_Shutdown();
	void ImGuiImplementation_ApplyStyling();

private:
	// Engine Contexts
	EditorConsole* m_EditorConsole;
	std::unique_ptr<Aurora::Engine> m_Engine;
	Aurora::EngineContext* m_EngineContext = nullptr; // Consists of subsystems.
	EditorSubsystem* m_EditorSubsystem;

	// Editor Contexts
	bool m_EditorBegun = false;
	std::vector<std::shared_ptr<Widget>> m_Widgets;
};