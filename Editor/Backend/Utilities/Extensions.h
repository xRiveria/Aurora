#pragma once
#include <optional>
#include "EngineContext.h"
#include "../Window/WindowContext.h"
#include "../Utilities/IconLibrary.h"
#include "../Scene/Entity.h"
#include "../Source/imgui.h"
#include "../Backend/Source/imgui_internal.h"
#include <variant>

namespace EditorExtensions
{
	class ContextHelper
	{
	public:
		static ContextHelper& GetInstance()
		{
			static ContextHelper m_ContextHelperInstance;
			return m_ContextHelperInstance;
		}

		void SetSelectedEntity(const std::shared_ptr<Aurora::Entity>& entity)
		{
			// Keep returned entity instead as the transform gizmo can decide to reject it.
			m_SelectedEntity = entity;
		}

		std::weak_ptr<Aurora::Entity> m_SelectedEntity;
	};

	static const ImVec4 g_DefaultTint = { 255, 255, 255, 255 };

	inline std::optional<std::string> OpenFilePath(const char* filter, Aurora::EngineContext* engineContext)
	{
		OPENFILENAMEA fileDialog; //Passes data to and from GetOpenFileName & GetSaveFileName. It stores settings used to create the dialog box and the results of the user's selection. 
		CHAR szFile[260] = { 0 }; //Our selected file path's buffer.
		ZeroMemory(&fileDialog, sizeof(OPENFILENAME)); //Initialize openedFile's memory to 0.

		fileDialog.lStructSize = sizeof(OPENFILENAME); //Sets the struct size. We do this for every Win32 struct.
		fileDialog.hwndOwner = engineContext->GetSubsystem<Aurora::WindowContext>()->GetWindowHWND(0); //Gets our currently open window and retrieves it HWND which we set as the struct's owner.
		fileDialog.lpstrFile = szFile; //Buffer for our file.
		fileDialog.nMaxFile = sizeof(szFile); //Size of our file buffer.
		fileDialog.lpstrFilter = filter; //File filter.
		fileDialog.nFilterIndex = 1; //Which filter is set by default. 
		fileDialog.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR; //The last flag is very important. If you don't do this and call OpenFileName, it will change the working directory for your application to the folder you open the window from.  

		if (GetOpenFileNameA(&fileDialog) == true)
		{
			return fileDialog.lpstrFile; //We return the file path of the file we open and create a string out of the char* path.
		}

		return std::nullopt; //Return empty string if no file is selected. It means the dialog has been cancelled.
	}

	inline bool ImageButton(const IconType iconType, const float iconSize)
	{
		auto internalState = Aurora::DX11_Utility::ToInternal(&IconLibrary::GetInstance().GetTextureByType(iconType)->m_Texture);
		return ImGui::ImageButton((void*)internalState->m_ShaderResourceView.Get(),
			ImVec2(iconSize, iconSize),
			ImVec2(0, 0),
			ImVec2(1, 1),
			-1,
			ImColor(0, 0, 0, 0), // Border
			g_DefaultTint);
	}

	inline void Image(const IconType iconType, const float iconSize)
	{
		auto internalState = Aurora::DX11_Utility::ToInternal(&IconLibrary::GetInstance().GetTextureByType(iconType)->m_Texture);
		return ImGui::Image((void*)internalState->m_ShaderResourceView.Get(),
			ImVec2(iconSize, iconSize),
			ImVec2(0, 0),
			ImVec2(1, 1),
			g_DefaultTint,
			ImColor(0, 0, 0, 0)); // Border
	}

	// Drag & Drop
	enum DragPayloadType
	{
		DragPayloadType_Unknown,
		DragPayloadType_Entity,
		DragPayloadType_Model,
		DragPayloadType_Material,
		DragPayloadType_Texture
	};

	struct DragDropPayload
	{
		typedef std::variant<const char*, unsigned int> DataVariant; // Holds either a const char* or unsigned integer identifier.

		DragDropPayload(const DragPayloadType payloadType = DragPayloadType::DragPayloadType_Unknown, const DataVariant data = nullptr)
		{
			this->m_PayloadType = payloadType;
			this->m_Data = data;
		}

		DragPayloadType m_PayloadType;
		DataVariant m_Data;
	};

	inline void CreateDragPayload(const DragDropPayload& payload)
	{
		ImGui::SetDragDropPayload(reinterpret_cast<const char*>(&payload.m_PayloadType), reinterpret_cast<const void*>(&payload), sizeof(payload), ImGuiCond_Once);
	}

	inline DragDropPayload* ReceiveDragPayload(DragPayloadType payloadType)
	{
		if (ImGui::BeginDragDropTarget()) // Remember that BeginDragDropTarget() uses the last item in the window that was dragged, set with ImGui::SetDragDropPayload.
		{
			// ImGui::AcceptDragDropPayloadType will reject the data and return null if the given type does not match the type of the dragged payload.
			if (const auto payload = ImGui::AcceptDragDropPayload(reinterpret_cast<const char*>(&payloadType))) // If matches, accept the data that comes in and stores it inside the payload variable.
			{
				return static_cast<DragDropPayload*>(payload->Data); // Retrieve our payload object from the payload data currently saved in the drag operation.
			}

			ImGui::EndDragDropTarget();
		}

		return nullptr;
	}
}