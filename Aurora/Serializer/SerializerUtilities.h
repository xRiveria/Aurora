#pragma once
#include "yaml-cpp/yaml.h"
#include <DirectXMath.h>

using namespace DirectX;

namespace Aurora
{
    inline YAML::Emitter& operator<<(YAML::Emitter& outputStream, const XMFLOAT3& vector)
    {
        outputStream << YAML::Flow;
        outputStream << YAML::BeginSeq << vector.x << vector.y << vector.z << YAML::EndSeq;

        return outputStream;
    }

    inline YAML::Emitter& operator<<(YAML::Emitter& outputStream, const XMFLOAT4& vector)
    {
        outputStream << YAML::Flow;
        outputStream << YAML::BeginSeq << vector.x << vector.y << vector.z << vector.w << YAML::EndSeq;

        return outputStream;
    }
}