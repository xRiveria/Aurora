#pragma once
#include <cstdint>
#include <string>

namespace Aurora
{
    struct WindowDescription
    {
        WindowDescription(const std::string& name, uint32_t width, uint32_t height) : m_Name(name), m_Width(width), m_Height(height)
        {

        }

        std::string m_Name = "Window";
        uint32_t m_Width = 1280;
        uint32_t m_Height = 720;
    };
}