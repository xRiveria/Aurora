#include "Editor.h"
#include "Source/imgui.h"
#include "Source/imgui_internal.h"
#include "../Renderer/Renderer.h"
#include <GLFW/glfw3.h>
#include "../Window/WindowContext.h"
#include "Implementation/imgui_impl_glfw.h"
#include "Implementation/imgui_impl_dx11.h"
#include "../Scene/World.h"
#include "../Scene/Components/Light.h"
#include "../Scene/Components/Mesh.h"
 
namespace EditorConfigurations
{
	const float g_FontSize = 17.0f;
}

Editor::Editor()
{
	// Create Engine
	m_Engine = std::make_unique<Aurora::Engine>();

	// Acquire useful engine subsystems.
	m_EngineContext = m_Engine->GetEngineContext();
	InitializeEditor();
}

Editor::~Editor()
{
	ImGuiImplementation_Shutdown();
}

inline Aurora::DX11_Utility::DX11_TexturePackage* ToInternal(const Aurora::RHI_Texture* texture)
{
	return static_cast<Aurora::DX11_Utility::DX11_TexturePackage*>(texture->m_InternalState.get());
}

enum class VectorType
{
	Scale,
	Rotation,
	Translation
};

void Editor::Tick()
{
	while (true)
	{
		m_Engine->Tick();

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Render ImGui Stuff
		BeginDockingContext();  // The start of a docking context.

		ImGui::Begin("Test");
		auto internalState = ToInternal(&m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_RenderTarget_GBuffer[GBuffer_Types::GBuffer_Color]);
		ImGui::Image((void*)internalState->m_ShaderResourceView.Get(), ImVec2(m_EngineContext->GetSubsystem<Aurora::WindowContext>()->GetWindowWidth(0), m_EngineContext->GetSubsystem<Aurora::WindowContext>()->GetWindowHeight(0)));
		ImGui::End();

		ImGui::Begin("Light Properties");
		Aurora::Light* component = m_EngineContext->GetSubsystem<Aurora::World>()->GetEntityByName("Directional_Light")->GetComponent<Aurora::Light>();
		float* color[3] = { &component->m_Color.x, &component->m_Color.y, &component->m_Color.z };
		ImGui::SliderFloat3("Ambient Color", *color, 0.0, 1.0);

		float* position[3] = { &component->m_Position.x, &component->m_Position.y, &component->m_Position.z };
		ImGui::SliderFloat3("Position", *position, -150, 150);
		ImGui::End();

		std::vector<std::shared_ptr<Aurora::Entity>> sceneEntities = m_EngineContext->GetSubsystem<Aurora::World>()->EntityGetAll();
		std::vector<Aurora::Mesh> meshComponents;
		for (auto& entity : sceneEntities)
		{
			if (entity->HasComponent<Aurora::Mesh>())
			{
				meshComponents.push_back(*entity->GetComponent<Aurora::Mesh>());
			}
		}

		ImGui::Begin("Properties");
		for (Aurora::Mesh& meshComponent : meshComponents)
		{		
			// Reflect
			XMFLOAT3 position = meshComponent.GetEntity()->m_Transform->GetPosition(); // Retrieve.
			XMFLOAT3 scale = meshComponent.GetEntity()->m_Transform->GetScale();
			XMFLOAT3 rotation = { meshComponent.GetEntity()->m_Transform->GetRotation().x, meshComponent.GetEntity()->m_Transform->GetRotation().y, meshComponent.GetEntity()->m_Transform->GetRotation().z };

			const auto ShowFloat = [](VectorType vectorType, const char* label, Aurora::Mesh* meshComponent, float* axis, XMFLOAT3* value)
			{
				char dragLabel[256];
				strcpy_s(dragLabel, label);
				strcat_s(dragLabel, std::to_string(meshComponent->GetEntity()->GetObjectID()).c_str());

				if (ImGui::SliderFloat(dragLabel, axis, -100, 100))
				{
					switch (vectorType)
					{
						case VectorType::Translation:
							meshComponent->GetEntity()->m_Transform->Translate(*value); // Update, which gets pushed into the retrieval above. 
							break;

						case VectorType::Scale:
							meshComponent->GetEntity()->m_Transform->Scale(*value); // Update, which gets pushed into the retrieval above. 
							break;
					}
				}
			};

			ShowFloat(VectorType::Translation, "X", &meshComponent, &position.x, &position);
			ShowFloat(VectorType::Translation, "Y", &meshComponent, &position.y, &position);
			ShowFloat(VectorType::Translation, "Z", &meshComponent, &position.z, &position);

			ShowFloat(VectorType::Scale, "SX", &meshComponent, &scale.x, &scale);
			ShowFloat(VectorType::Scale, "SY", &meshComponent, &scale.y, &scale);
			ShowFloat(VectorType::Scale, "SZ", &meshComponent, &scale.z, &scale);
		}

		ImGui::End();
		
		ImGui::Begin("Hierarchy");
		auto& entities = m_EngineContext->GetSubsystem<Aurora::World>()->EntityGetAll();
		for (auto& entity : entities)
		{
			ImGui::Text(entity->GetObjectName().c_str());
		}
		ImGui::End();
	
		ImGui::End(); // Ends docking context.

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		m_EngineContext->GetSubsystem<Aurora::Renderer>()->Present();

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backupCurrentContext = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backupCurrentContext);
		}
	}
}

void Editor::InitializeEditor()
{
	ImGuiImplementation_Initialize(m_EngineContext);
	ImGuiImplementation_ApplyStyling();
}

void Editor::BeginDockingContext()
{
	static bool dockSpaceOpen = true;
	static bool opt_fullscreen = true;
	static bool opt_padding = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}
	else
	{
		dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
	}

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
	// and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	if (!opt_padding)
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace Demo", &dockSpaceOpen, window_flags);
	if (!opt_padding)
		ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	// DockSpace
	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();

	float minimumWindowsize = style.WindowMinSize.x;
	style.WindowMinSize.x = 250.0f;
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}
	style.WindowMinSize.x = minimumWindowsize;
}

void Editor::ImGuiImplementation_Initialize(Aurora::EngineContext* engineContext)
{
	// Version Validation
	IMGUI_CHECKVERSION();
	// Context Creation
	ImGui::CreateContext();

	// Configuration
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigWindowsResizeFromEdges = true;
	io.ConfigViewportsNoTaskBarIcon = true;

	// Font 
	const char* filePath = "../Resources/Fonts/opensans/OpenSans-Bold.ttf";
	io.Fonts->AddFontFromFileTTF(filePath, EditorConfigurations::g_FontSize);
	io.FontDefault = io.Fonts->AddFontFromFileTTF("../Resources/Fonts/opensans/OpenSans-Regular.ttf", 17.0f);
	
	//Setup Platform/Renderer Bindings
	GLFWwindow* window = static_cast<GLFWwindow*>(m_EngineContext->GetSubsystem<Aurora::WindowContext>()->GetRenderWindow());
	ImGui_ImplGlfw_InitForOther(window, true);
	Aurora::Renderer* renderer = m_EngineContext->GetSubsystem<Aurora::Renderer>();
	ImGui_ImplDX11_Init(renderer->m_GraphicsDevice->m_Device.Get(), renderer->m_GraphicsDevice->m_DeviceContextImmediate.Get());
}

void Editor::ImGuiImplementation_Shutdown()
{
	if (ImGui::GetCurrentContext())
	{
		// ImGui::RHI::Shutdown();
		// ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();
	}
}

void Editor::ImGuiImplementation_ApplyStyling()
{
	// Apply Dark Style
	ImGui::StyleColorsDark();
	ImVec4* colors = ImGui::GetStyle().Colors;

	// Window Background
	colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

	// Headers
	colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
	colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
	colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

	// Buttons
	colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
	colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
	colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

	// Frame BG
	colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
	colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
	colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

	// Tabs
	colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
	colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
	colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

	// Title
	colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
}