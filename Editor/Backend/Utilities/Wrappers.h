#pragma once
#include "../Source/imgui.h"
#include "../Utilities/DifferentiatorTool.h"
#include <any>

namespace EditorExtensions::Wrappers
{
    static bool g_IsInitialDrag = true;
    static std::any g_InitialValue = 0;

    static void FloatSlider(const char* sliderLabel, float* reflectValue, float sliderSpeed, const std::any& initialValue, std::function<void(const std::any& updateValue)> UpdateFunction)
    {
        if (ImGui::DragFloat(sliderLabel, reflectValue, sliderSpeed))
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
        }
    }
}
