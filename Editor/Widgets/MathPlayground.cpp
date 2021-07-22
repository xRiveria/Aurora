#include "MathPlayground.h"
#include "../Math/Vector2.h"
#include "../Math/Vector3.h"

using namespace Aurora::Math;

Vector2 m_PointA(1.0f, 1.0f);
Vector2 m_PointB(1.0f, 1.0f);
Vector3 m_PointC(1.0f, 1.0f, 1.0f);

MathPlayground::MathPlayground(Editor* editorContext, Aurora::EngineContext* engineContext) : Widget(editorContext, engineContext)
{
    m_WidgetName = "Math Playground";
    m_IsWindowedWidget = false;
}

void MathPlayground::OnTickAlways()
{
    ImGui::Begin(m_WidgetName.c_str());
    ImGui::Text("Vector2");

    ImGui::InputFloat("Point A (X): ", &m_PointA.x);
    ImGui::InputFloat("Point A (Y): ", &m_PointA.y);
    ImGui::Text("Length of A: %.0f", m_PointA.Length());

    ImGui::Spacing();

    ImGui::InputFloat("Point B (X): ", &m_PointB.x);
    ImGui::InputFloat("Point B (Y): ", &m_PointB.y);
    ImGui::Text("Length of B: %.0f", m_PointB.Length());
    ImGui::Spacing();
    ImGui::Text("Distance Between AB: %.0f", Vector2::Distance(m_PointA, m_PointB));

    ImGui::Spacing();

    ImGui::Text("Vector3");
    ImGui::InputFloat("Point C (X): ", &m_PointC.x);
    ImGui::InputFloat("Point C (Y): ", &m_PointC.y);
    ImGui::InputFloat("Point C (Z): ", &m_PointC.z);

    if (ImGui::Button("Normalize"))
    {
        m_PointC.Normalize();
    }

    ImGui::Text("Length of C: %.0f", m_PointC.Length());

    ImGui::End();
}
