#define _XM_NO_INTRINSICS_
#include "EditorTools.h"
#include "../Backend/Source/ImGuizmo/ImGuizmo.h"
#include <functional>
#include "Properties.h"
#include "../Scene/World.h"
#include "../Scene/Entity.h"
#include "../Input/Input.h"
#include "../Input/InputUtilities.h"
#include "../Scene/Components/Camera.h"

const static float PI_Formula = 180 / 3.14159265359;

EditorTools::EditorTools(Editor* editorContext, Aurora::EngineContext* engineContext) : Widget(editorContext, engineContext)
{
	m_IsWidgetVisible = false; // Should not be visible.
}

void EditorTools::OnTickViewport()
{
    TickGizmos();
}

void EditorTools::OnEvent(Aurora::InputEvent& inputEvent)
{
	if (m_CanReceiveEvents)
	{
		Aurora::InputEventDispatcher dispatcher(inputEvent);
		dispatcher.Dispatch<Aurora::KeyPressedEvent>(AURORA_BIND_INPUT_EVENT(EditorTools::OnKeyPressed));
	}
}

bool EditorTools::OnKeyPressed(Aurora::KeyPressedEvent& inputEvent)
{
	if (inputEvent.GetRepeatCount() > 0)
	{
		return false;
	}

	switch (inputEvent.GetKeyCode())
	{
		AURORA_INFO(Aurora::LogLayer::Editor, "Received!");
		case AURORA_KEY_E:
			m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
			return true;

		case AURORA_KEY_R:
			m_GizmoType = ImGuizmo::OPERATION::SCALE;
			return true;

		case AURORA_KEY_T:
			m_GizmoType = ImGuizmo::OPERATION::ROTATE;
			return true;

		case AURORA_KEY_Q:
			m_GizmoType = -1;
			return true;
	}

	return false;
}

void EditorTools::TickGizmos()
{
	Aurora::AURORA_PROFILE_FUNCTION();

	if (!Properties::m_InspectedEntity.expired() && m_GizmoType != -1)
	{
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();

		float windowWidth = (float)ImGui::GetWindowWidth();
		float windowHeight = (float)ImGui::GetWindowHeight();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

		// Camera - Please work on this.
		Aurora::Camera* cameraEntity = m_EngineContext->GetSubsystem<Aurora::World>()->GetEntityByName("Default_Camera")->GetComponent<Aurora::Camera>();
		Aurora::Transform* transformComponent = Properties::m_InspectedEntity.lock().get()->m_Transform;

		// XMFLOAT4X4 transform = transformComponent->m_LocalMatrix;
		auto view = cameraEntity->GetViewMatrix();
		auto projection = cameraEntity->GetProjectionMatrix();

		DirectX::XMFLOAT4X4 transform;
		float Ftranslation[3] = { transformComponent->m_TranslationLocal.x, transformComponent->m_TranslationLocal.y, transformComponent->m_TranslationLocal.z };
		float Frotation[3] = { DirectX::XMConvertToDegrees(transformComponent->m_RotationInRadians.x), DirectX::XMConvertToDegrees(transformComponent->m_RotationInRadians.y), DirectX::XMConvertToDegrees(transformComponent->m_RotationInRadians.z) };
		float Fscale[3] = { transformComponent->m_ScaleLocal.x, transformComponent->m_ScaleLocal.x, transformComponent->m_ScaleLocal.x };
		ImGuizmo::RecomposeMatrixFromComponents(Ftranslation, Frotation, Fscale, *transform.m);

		ImGuizmo::Manipulate(&view._11, &projection._11, (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, *transform.m);

		if (ImGuizmo::IsUsing())
		{
			float Ftranslation[3] = { 0.0f, 0.0f, 0.0f }, Frotation[3] = { 0.0f, 0.0f, 0.0f }, Fscale[3] = { 0.0f, 0.0f, 0.0f };
			ImGuizmo::DecomposeMatrixToComponents(*transform.m, Ftranslation, Frotation, Fscale);
			// XMMatrixDecompose(&scaleLocal, &rotationLocal, &translationLocal, XMLoadFloat4x4(&transform));

			transformComponent->m_TranslationLocal = XMFLOAT3(Ftranslation);
			transformComponent->m_RotationInRadians = DirectX::XMFLOAT3(DirectX::XMConvertToRadians(Frotation[0]), DirectX::XMConvertToRadians(Frotation[1]), DirectX::XMConvertToRadians(Frotation[2]));
			transformComponent->m_ScaleLocal = DirectX::XMFLOAT3(Fscale);

			transformComponent->SetDirty(true);
		}
	}
}