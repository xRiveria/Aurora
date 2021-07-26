#define _XM_NO_INTRINSICS_
#include "EditorTools.h"
#include "../Backend/Source/ImGuizmo/ImGuizmo.h"
#include "Properties.h"
#include "../Scene/World.h"
#include "../Scene/Entity.h"
#include "../Input/Input.h"
#include "../Input/InputUtilities.h"

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

		XMFLOAT4X4 transform = transformComponent->m_WorldMatrix;
		auto view = cameraEntity->GetViewMatrix();
		auto projection = cameraEntity->GetProjectionMatrix();
			
		ImGuizmo::Manipulate(&view._11, &projection._11, (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, &transform._11);

		if (ImGuizmo::IsUsing())
		{
			XMVECTOR scaleLocal, rotationLocal, translationLocal;
			XMMatrixDecompose(&scaleLocal, &rotationLocal, &translationLocal, XMLoadFloat4x4(&transform));

			XMFLOAT3 translation = { translationLocal.vector4_f32[0], translationLocal.vector4_f32[1], translationLocal.vector4_f32[2] };
			XMFLOAT3 scale = { scaleLocal.vector4_f32[0], scaleLocal.vector4_f32[1], scaleLocal.vector4_f32[2] };
			XMFLOAT4 rotate = { rotationLocal.vector4_f32[0], rotationLocal.vector4_f32[1], rotationLocal.vector4_f32[2], rotationLocal.vector4_f32[3] };

			transformComponent->Translate(translation);
			transformComponent->Scale(scale);
			transformComponent->Rotate(rotate);
		}
	}
}
