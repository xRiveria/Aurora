#define _XM_NO_INTRINSICS_
#include "EditorTools.h"
#include "../Backend/Source/ImGuizmo/ImGuizmo.h"
#include "Properties.h"
#include "../Scene/World.h"
#include "../Scene/Entity.h"
#include "../Input/Input.h"
#include "../Input/InputUtilities.h"

const static float PI_Formula = 180 / 3.14159265359;

EditorTools::EditorTools(Editor* editorContext, Aurora::EngineContext* engineContext) : Widget(editorContext, engineContext)
{
	m_IsWidgetVisible = false; // Should not be visible.
}

void EditorTools::OnTickAlways()
{
    TickGizmos();
}

void EditorTools::TickGizmos()
{
	if (m_EngineContext->GetSubsystem<Aurora::Input>()->IsKeyPressed(AURORA_KEY_E))
	{
		m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
	}
	else if (m_EngineContext->GetSubsystem<Aurora::Input>()->IsKeyPressed(AURORA_KEY_R))
	{
		m_GizmoType = ImGuizmo::OPERATION::SCALE;
	}
	else if (m_EngineContext->GetSubsystem<Aurora::Input>()->IsKeyPressed(AURORA_KEY_T))
	{
		m_GizmoType = ImGuizmo::OPERATION::ROTATE;
	}
	else if (m_EngineContext->GetSubsystem<Aurora::Input>()->IsKeyPressed(AURORA_KEY_Q))
	{
		m_GizmoType = -1;
	}

	// Gizmo
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
		float Frotation[3] = { DirectX::XMConvertToDegrees(transformComponent->m_RotationAngles.x), DirectX::XMConvertToDegrees(transformComponent->m_RotationAngles.y), DirectX::XMConvertToDegrees(transformComponent->m_RotationAngles.z) };
		float Fscale[3] = { transformComponent->m_ScaleLocal.x, transformComponent->m_ScaleLocal.x, transformComponent->m_ScaleLocal.x };
		ImGuizmo::RecomposeMatrixFromComponents(Ftranslation, Frotation, Fscale, *transform.m);

		ImGuizmo::Manipulate(&view._11, &projection._11, (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, *transform.m);

		if (ImGuizmo::IsUsing())
		{
			float Ftranslation[3] = { 0.0f, 0.0f, 0.0f }, Frotation[3] = { 0.0f, 0.0f, 0.0f }, Fscale[3] = { 0.0f, 0.0f, 0.0f };
			ImGuizmo::DecomposeMatrixToComponents(*transform.m, Ftranslation, Frotation, Fscale);
			// XMMatrixDecompose(&scaleLocal, &rotationLocal, &translationLocal, XMLoadFloat4x4(&transform));

			transformComponent->m_TranslationLocal = XMFLOAT3(Ftranslation);
			transformComponent->m_RotationAngles = DirectX::XMFLOAT3(DirectX::XMConvertToRadians(Frotation[0]), DirectX::XMConvertToRadians(Frotation[1]), DirectX::XMConvertToRadians(Frotation[2]));
			transformComponent->m_ScaleLocal = DirectX::XMFLOAT3(Fscale);
		}
	}
}
