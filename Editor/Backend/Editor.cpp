#define _XM_NO_INTRINSICS_
#include "Editor.h"
#include "../Profiler/Instrumentor.h"
#include "Source/imgui.h"
#include "Source/imgui_internal.h"
#include "../Renderer/Renderer.h"
#include <GLFW/glfw3.h>
#include "../Window/WindowContext.h"
#include "Implementation/imgui_impl_glfw.h"
#include "Implementation/imgui_impl_dx11.h"
#include "../Scene/World.h"
#include "../Widgets/MenuBar.h"
#include "../Widgets/QuickDiagnostics.h"
#include "../Widgets/ObjectsPanel.h"
#include "../Widgets/Toolbar.h"
#include "../Widgets/MathPlayground.h"
#include <optional>
#include "../Widgets/Properties.h"
#include "Utilities/IconLibrary.h"
#include "Source/ImGuizmo/ImGuizmo.h"
#include "../Input/Input.h"
#include "../Input/InputUtilities.h"
#include "../Widgets/Viewport.h"
#include "../Time/Stopwatch.h"
#include "../Profiler/Profiler.h"
#include "Utilities/Extensions.h"
#include "FileSystem.h"
#include "EngineContext.h"
#include "../Graphics/DX11/Skybox.h"
#include "../Widgets/Hierarchy.h"
#include "../Widgets/ProjectSettings.h"
#include <vector>
#include "../Widgets/ThreadTracker.h"
#include "../Widgets/AssetRegistry.h"
#include "../Widgets/AssetBrowser.h"
#include "../Widgets/EditorConsole.h"

namespace EditorConfigurations
{
	const float g_FontSize = 17.0f;
}

EditorSubsystem::EditorSubsystem(Aurora::EngineContext* engineContext) : Aurora::ISubsystem(engineContext)
{

}

void EditorSubsystem::OnEvent(Aurora::InputEvent& inputEvent)
{
	// Push events to any widgets that ought to be listening to events.
	for (int i = 0; i < m_Editor->GetWidgets().size(); i++)
	{
		m_Editor->GetWidgets()[i]->OnEvent(inputEvent);
	}
}

Editor::Editor()
{
	Aurora::Instrumentor::GetInstance().BeginSession("Startup");

	Aurora::AURORA_PROFILE_FUNCTION();

	// Create Engine
	m_Engine = std::make_unique<Aurora::Engine>();

	// Acquire useful engine subsystems.
	m_EngineContext = m_Engine->GetEngineContext();;

	// Register event polling.
	m_EngineContext->RegisterSubsystem<EditorSubsystem>();
	m_EditorSubsystem = m_EngineContext->GetSubsystem<EditorSubsystem>();
	m_EditorSubsystem->SetEditorInstance(this);

	InitializeEditor();
	IconLibrary::GetInstance().Initialize(m_EngineContext, this);

	Aurora::Instrumentor::GetInstance().EndSession();
}

Editor::~Editor()
{
	ImGuiImplementation_Shutdown();
}

inline Aurora::DX11_Utility::DX11_TexturePackage* ToInternal(const Aurora::RHI_Texture* texture)
{
	return static_cast<Aurora::DX11_Utility::DX11_TexturePackage*>(texture->m_InternalState.get());
}

std::string g_CurrentlySelectedView = "Off";

void Editor::Tick()
{
	Aurora::Instrumentor::GetInstance().BeginSession("Tick");

	while (m_EngineContext->GetSubsystem<Aurora::WindowContext>()->IsWindowRunning())
	{
		m_Engine->Tick();

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		// Render ImGui Stuff
		BeginDockingContext();  // The start of a docking context.

		// Editor Tick
		{
			Aurora::Stopwatch widgetStopwatch("Widget Pass", true);
			Aurora::AURORA_PROFILE_FUNCTION();
			for (std::shared_ptr<Widget>& widget : m_Widgets) // Tick each individual widget. Each widget contains its own ImGui::Begin and ImGui::End behavior (based on visibility/constantness).
			{
				widget->Tick();
			}
		}

		// Make sure this is last.
		ImGui::Begin("Performance");
		for (int i = 0; i < Aurora::Profiler::GetInstance().GetEntries().size(); i++)
		{
			ImGui::Text("%s", Aurora::Profiler::GetInstance().GetEntries()[i].c_str());
		}
		Aurora::Profiler::GetInstance().Reset();
		ImGui::End();

		ImGui::Begin("Renderer");

		char sampleString[11];
		sprintf_s(sampleString, "%u", m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_DeviceContext->GetCurrentMultisampleLevel());
		if (ImGui::BeginCombo("Multisample Level", sampleString, 0))
		{
			if (ImGui::Selectable("Off"))
			{
				m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_DeviceContext->SetMultisampleLevel(1);
				m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_DeviceContext->ResizeBuffers();
			}

			if (ImGui::Selectable("2"))
			{
				m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_DeviceContext->SetMultisampleLevel(2);
				m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_DeviceContext->ResizeBuffers();
			}

			if (ImGui::Selectable("4"))
			{
				m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_DeviceContext->SetMultisampleLevel(4);
				m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_DeviceContext->ResizeBuffers();
			}

			if (ImGui::Selectable("8"))
			{
				m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_DeviceContext->SetMultisampleLevel(8);
				m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_DeviceContext->ResizeBuffers();
			}

			ImGui::EndCombo();
		}
		ImGui::Text("Maximum MSAA Level Supported: %u", m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_DeviceContext->GetMaxMultisampleLevel());

		// Needs a whole system of its own.
		
		if (ImGui::BeginCombo("Debug Views", g_CurrentlySelectedView.c_str(), 0))
		{
			if (ImGui::Selectable("Off"))
			{
				g_CurrentlySelectedView = "Off";
			}

			if (ImGui::Selectable("Shadow Depth Buffer"))
			{
				g_CurrentlySelectedView = "Shadow Depth Buffer";
			}

			if (ImGui::Selectable("Bloom Buffer"))
			{
				g_CurrentlySelectedView = "Bloom Buffer";
			}

			if (ImGui::Selectable("IBL HDR Texture"))
			{
				g_CurrentlySelectedView = "IBL HDR Texture";
			}

			ImGui::EndCombo();
		}

		if (g_CurrentlySelectedView == "Shadow Depth Buffer")
		{
			ImGui::Image((void*)m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_DeviceContext->m_ShadowDepthTexture->GetShaderResourceView().Get(), ImVec2(720, 480));
		}
		else if (g_CurrentlySelectedView == "Bloom Buffer")
		{
			ImGui::Image((void*)m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_DeviceContext->m_BloomRenderTexture->GetShaderResourceView().Get(), ImVec2(720, 480));
		}
		else if (g_CurrentlySelectedView == "IBL HDR Texture")
		{
			ImGui::Image((void*)m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_Skybox->m_EnvironmentTextureEquirectangular.m_SRV.Get(), ImVec2(720, 480));
		}

		ImGui::End();

		// Sky
		ImGui::Begin("Sky");

		//Aurora::DX11_Utility::DX11_TexturePackage* textures = Aurora::DX11_Utility::ToInternal(&m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_Skybox->m_EnvironmentTextureEquirectangular);
		// ImGui::Image((void*)m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_Skybox->m_EnvironmentTextureEquirectangular.m_SRV.Get(), ImVec2(600, 600));
		if (ImGui::Button("Front"))
		{
			m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_Camera->GetComponent<Aurora::Camera>()->SetRotation(0.0f, 90.0f, 0.0f); // front
		}
		if (ImGui::Button("Back"))
		{
			m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_Camera->GetComponent<Aurora::Camera>()->SetRotation(0.0f, 270.0f, 0.0f); // back
		}
		if (ImGui::Button("Top"))
		{
			m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_Camera->GetComponent<Aurora::Camera>()->SetRotation(-90.0f, 0.0f, 0.0f); // top
		}
		if (ImGui::Button("Bottom"))
		{
			m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_Camera->GetComponent<Aurora::Camera>()->SetRotation(90.0f, 0.0f, 0.0f); // bottom
		}
		if (ImGui::Button("Left"))
		{
			m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_Camera->GetComponent<Aurora::Camera>()->SetRotation(0.0f, 0.0f, 0.0f); // left
		}
		if (ImGui::Button("Right"))
		{
			m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_Camera->GetComponent<Aurora::Camera>()->SetRotation(0.0f, 180.0f, 0.0f); // right
		}

		ImGui::Image((void*)m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_DeviceContext->m_ShadowDepthTexture->GetShaderResourceView().Get(), ImVec2(600, 600));

		// Aurora::DX11_Utility::DX11_TexturePackage* texturee = Aurora::DX11_Utility::ToInternal(&m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_DepthBuffer_Main);
		// ImGui::Image((void*)texturee->m_ShaderResourceView.Get(), ImVec2(600, 600));
		ImGui::DragFloat("Light Bias", &m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_LightBias, 0.01, 0.005, 0.1);
		/*
		else
		{
			Aurora::DX11_Utility::DX11_TexturePackage* texture = Aurora::DX11_Utility::ToInternal(&m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_DefaultWhiteTexture->m_Texture);
			ImGui::Image((void*)texture->m_ShaderResourceView.Get(), ImVec2(300, 300));
		}
		if (ImGui::Button("Load..."))
		{
			std::optional<std::string> filePath = EditorExtensions::OpenFile("Textures", m_EngineContext);
			if (filePath.has_value())
			{
				std::string path = filePath.value();
				std::string fileName = Aurora::FileSystem::GetFileNameFromFilePath(path);
				std::shared_ptr<Aurora::AuroraResource> resource = m_EngineContext->GetSubsystem<Aurora::ResourceCache>()->LoadTextureHDR(path, 4);
				m_EngineContext->GetSubsystem<Aurora::Renderer>()->m_Skybox->m_SkyHDR->m_Texture = resource->m_Texture;
			}
		}
		*/

		ImGui::End();

		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
		float height = ImGui::GetFrameHeight() + 1.0; //Add a little padding here so things look nicer.

		if (ImGui::BeginViewportSideBar("##MainStatusBar", nullptr, ImGuiDir_Down, height, windowFlags)) // Specifies that this will be pipped at the top of the window, below the main menu bar.
		{
			if (ImGui::BeginMenuBar())
			{
				if (!m_EditorConsole->m_Logs.empty())
				{
					ImGui::PushStyleColor(ImGuiCol_Text, m_EditorConsole->m_LogTypeColor[static_cast<int>(m_EditorConsole->m_Logs.back().m_LogType.first)]);
					ImGui::TextUnformatted(m_EditorConsole->m_Logs.back().m_Text.c_str());
					ImGui::PopStyleColor();
				}
				else
				{
					ImGui::TextUnformatted("");
				}
			}

			ImGui::EndMenuBar();
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
	Aurora::Instrumentor::GetInstance().EndSession();

}

void Editor::InitializeEditor()
{
	Aurora::AURORA_PROFILE_FUNCTION();

	ImGuiImplementation_Initialize(m_EngineContext);
	ImGuiImplementation_ApplyStyling();

	// Create all ImGui widgets.
	m_Widgets.emplace_back(std::make_shared<EditorConsole>(this, m_EngineContext));
	m_EditorConsole = dynamic_cast<EditorConsole*>(m_Widgets.back().get());
	m_Widgets.emplace_back(std::make_shared<ProjectSettings>(this, m_EngineContext));
	m_Widgets.emplace_back(std::make_shared<QuickDiagnostics>(this, m_EngineContext));
	m_Widgets.emplace_back(std::make_shared<ThreadTracker>(this, m_EngineContext));
	m_Widgets.emplace_back(std::make_shared<MenuBar>(this, m_EngineContext));
	m_Widgets.emplace_back(std::make_shared<Toolbar>(this, m_EngineContext));
	m_Widgets.emplace_back(std::make_shared<Properties>(this, m_EngineContext));
	m_Widgets.emplace_back(std::make_shared<ObjectsPanel>(this, m_EngineContext));
	m_Widgets.emplace_back(std::make_shared<MathPlayground>(this, m_EngineContext)); // For me to play with the Math library.
	m_Widgets.emplace_back(std::make_shared<Viewport>(this, m_EngineContext));
	m_Widgets.emplace_back(std::make_shared<Hierarchy>(this, m_EngineContext));
	m_Widgets.emplace_back(std::make_shared<EditorTools>(this, m_EngineContext));
	m_Widgets.emplace_back(std::make_shared<AssetRegistry>(this, m_EngineContext));
	m_Widgets.emplace_back(std::make_shared<AssetBrowser>(this, m_EngineContext));

	AURORA_WARNING(Aurora::LogLayer::Engine, "All Editor Widgets Initialized.");
}

void Editor::BeginDockingContext()
{
	Aurora::AURORA_PROFILE_FUNCTION();

	static bool dockSpaceOpen = true;
	static bool opt_fullscreen = true;
	static bool opt_padding = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
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
	ImGuiIO& io = ImGui::GetIO();

	// When viewports are enabled, we tweak WindowRounding/WindowBg so Window platforms can look identifical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	ImVec4* colors = ImGui::GetStyle().Colors;

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