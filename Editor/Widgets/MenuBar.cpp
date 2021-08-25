#include "MenuBar.h"
#include "../Scene/World.h"
#include "../Threading/Threading.h"
#include "../Backend/Utilities/Extensions.h"
#include <optional>

MenuBar::MenuBar(Editor* editorContext, Aurora::EngineContext* engineContext) : Widget(editorContext, engineContext)
{
	m_WidgetName = "Menu Bar";
	m_IsWindowedWidget = false;
}

void MenuBar::OnTickAlways()
{
	Aurora::AURORA_PROFILE_FUNCTION();

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New Scene", "Ctrl + N"))
			{
				NewScene();
			}

			if (ImGui::MenuItem("Save Scene", "Ctrl + S"))
			{
				SaveScene();
			}

			if (ImGui::MenuItem("Load Scene", "Ctrl + L"))
			{
				LoadScene();
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Close"))
			{
				m_EngineContext->GetSubsystem<Aurora::WindowContext>()->SetWindowRunState(false);
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			ImGui::Separator();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Assets"))
		{
			ImGui::Separator();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Object"))
		{

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Windows"))
		{

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void MenuBar::OnEvent(Aurora::InputEvent& inputEvent)
{
	Aurora::InputEventDispatcher dispatcher(inputEvent);
	dispatcher.Dispatch<Aurora::KeyPressedEvent>(AURORA_BIND_INPUT_EVENT(MenuBar::OnKeyPressed));
}

bool MenuBar::OnKeyPressed(Aurora::KeyPressedEvent& keyPressedEvent)
{
	if (keyPressedEvent.GetRepeatCount() > 0)
	{
		return false;
	}

	bool isControlPressed = m_EngineContext->GetSubsystem<Aurora::Input>()->IsKeyPressed(AURORA_KEY_LEFT_CONTROL) || m_EngineContext->GetSubsystem<Aurora::Input>()->IsKeyPressed(AURORA_KEY_RIGHT_CONTROL);
	bool isShiftPressed = m_EngineContext->GetSubsystem<Aurora::Input>()->IsKeyPressed(AURORA_KEY_LEFT_SHIFT) || m_EngineContext->GetSubsystem<Aurora::Input>()->IsKeyPressed(AURORA_KEY_RIGHT_SHIFT);

	switch (keyPressedEvent.GetKeyCode())
	{
		case AURORA_KEY_N:
		{
			if (isControlPressed) { NewScene(); }
			break;
		}

		case AURORA_KEY_L:
		{
			if (isControlPressed) { LoadScene(); }
			break;
		}

		case AURORA_KEY_S:
		{
			if (isControlPressed && isShiftPressed) { SaveScene(); }
			break;
		}
	}
}

void MenuBar::SaveScene()
{
	std::optional<std::string> filePath = EditorExtensions::SaveFile("Aurora Scene (*.aurora)\0*.aurora\0", m_EngineContext);

	if (filePath.has_value())
	{
		AURORA_INFO(Aurora::LogLayer::Serialization, "%s", filePath.value().c_str());

		m_EngineContext->GetSubsystem<Aurora::Threading>()->Execute([this, filePath](Aurora::JobInformation jobInformation)
		{
			m_EngineContext->GetSubsystem<Aurora::World>()->SerializeScene(filePath.value());
		});
	}
}

void MenuBar::LoadScene()
{
	std::optional<std::string> filePath = EditorExtensions::OpenFile("Aurora Scene (*.aurora)\0*.aurora\0", m_EngineContext);

	if (filePath.has_value())
	{
		AURORA_INFO(Aurora::LogLayer::Serialization, "%s", filePath.value().c_str());

		m_EngineContext->GetSubsystem<Aurora::Threading>()->Execute([this, filePath](Aurora::JobInformation jobInformation)
		{
			m_EngineContext->GetSubsystem<Aurora::World>()->DeserializeScene(filePath.value());
		});
	}
}

void MenuBar::NewScene()
{
	m_EngineContext->GetSubsystem<Aurora::World>()->New();
}
