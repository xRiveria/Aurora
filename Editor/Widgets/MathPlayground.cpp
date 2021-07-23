#include "MathPlayground.h"
#include "../Math/Vector2.h"
#include "../Math/Vector3.h"
#include "../Math/Vector4.h"

using namespace Aurora::Math;

Vector2 m_PointA(1.0f, 1.0f);
Vector2 m_PointB(1.0f, 1.0f);
Vector3 m_PointC(1.0f, 1.0f, 1.0f);
Vector4 m_PointD(1.0f, 1.0f, 1.0f, 1.0f);


MathPlayground::MathPlayground(Editor* editorContext, Aurora::EngineContext* engineContext) : Widget(editorContext, engineContext)
{
    m_WidgetName = "Math Playground";
    m_IsWindowedWidget = false;
}

void MathPlayground::OnTickAlways()
{
    ImGui::Begin(m_WidgetName.c_str());
    if (ImGui::CollapsingHeader("Vector 2"))
    {
        ImGui::InputFloat2("Point A", m_PointA.Data());
        ImGui::Text("Length of A: %.0f", m_PointA.Length());

        ImGui::Spacing();

        ImGui::InputFloat2("Point B", m_PointB.Data());
        ImGui::Text("Length of B: %.0f", m_PointB.Length());

        ImGui::Spacing();

        ImGui::Text("Distance Between AB: %.0f", Vector2::Distance(m_PointA, m_PointB));
    }

    if (ImGui::CollapsingHeader("Vector 3"))
    {
        ImGui::InputFloat3("Point D", m_PointC.Data());
        ImGui::Text("Length of C: %.0f", m_PointC.Length());

        if (ImGui::Button("Normalize##Vector3"))
        {
            m_PointC.Normalize();
        }
    }

    if (ImGui::CollapsingHeader("Vector 4"))
    {
        ImGui::InputFloat4("Point D", m_PointD.Data());
        ImGui::Text("Length of D: %.0f", m_PointD.Length());

        if (ImGui::Button("Normalize##Vector4"))
        {
            m_PointD.Normalize();
        }
    }

    ImGui::End();
}
