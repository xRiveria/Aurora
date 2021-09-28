#pragma once
#include "../Source/imgui.h"
#include "../Utilities/DifferentiatorTool.h"
#include <any>

namespace EditorExtensions::Wrappers
{
    bool g_IsInitialDrag = true;
    std::any g_InitialValue = 0;

    void BeginSequence()
    {
        ImGui::Columns(2);
        ImGui::AlignTextToFramePadding();
    }

    void EndSequence()
    {
        ImGui::Columns(1);
    }

    void NextColumn()
    {
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);
    }

    void FloatSlider(const char* sliderLabel, float* reflectValue, float sliderSpeed, const std::any& initialValue, std::function<void(const std::any& updateValue)> UpdateFunction)
    {
        std::string labelID = std::string("##") + sliderLabel;

        ImGui::Text(sliderLabel);

        NextColumn();

        if (ImGui::DragFloat(labelID.c_str(), reflectValue, sliderSpeed))
        {
            if (g_IsInitialDrag)
            {
                g_InitialValue = initialValue;
                g_IsInitialDrag = false;
            }

            UpdateFunction(*reflectValue);     
        }

        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            Aurora::DifferentiatorTool::PushAction(g_InitialValue, *reflectValue, UpdateFunction);
            g_IsInitialDrag = true;
            g_InitialValue.reset();
        }

        ImGui::PopItemWidth();
        ImGui::NextColumn();
    }

    void Float3Slider(const char* sliderLabel, float* reflectValue, float minimumValue, float maximumValue, const std::any& initialValue, std::function<void(const std::any& updateValue)> UpdateFunction)
    {
        std::string labelID = std::string("##") + sliderLabel;

        ImGui::Text(sliderLabel);

        NextColumn();

        if (ImGui::SliderFloat3(labelID.c_str(), reflectValue, minimumValue, maximumValue))
        {
            if (g_IsInitialDrag)
            {
                g_InitialValue = initialValue;
                g_IsInitialDrag = false;
            }

            UpdateFunction(*reflectValue);
        }

        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            Aurora::DifferentiatorTool::PushAction(g_InitialValue, *reflectValue, UpdateFunction);
            g_IsInitialDrag = true;
            g_InitialValue.reset();
        }

        ImGui::PopItemWidth();
        ImGui::NextColumn();
    }

    void Checkbox(const char* sliderLabel, bool* reflectValue, const std::any& initialValue, std::function<void(const std::any& updateValue)> UpdateFunction)
    {
        std::string labelID = std::string("##") + sliderLabel;

        ImGui::Text(sliderLabel);

        NextColumn();

        if (ImGui::Checkbox(labelID.c_str(), reflectValue))
        {
            if (g_IsInitialDrag)
            {
                g_InitialValue = initialValue;
                g_IsInitialDrag = false;
            }

            UpdateFunction(*reflectValue);
        }

        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            Aurora::DifferentiatorTool::PushAction(g_InitialValue, *reflectValue, UpdateFunction);
            g_IsInitialDrag = true;
            g_InitialValue.reset();
        }

        ImGui::PopItemWidth();
        ImGui::NextColumn();
    }

    void Button(const char* buttonLabel, const char* buttonText, std::function<void()> Function)
    {
        ImGui::Text(buttonLabel);

        NextColumn();

        if (ImGui::Button(buttonText))
        {
            Function();
        }

        ImGui::PopItemWidth();
        ImGui::NextColumn();
    }
}
