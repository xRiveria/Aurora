#pragma once
#include "yaml-cpp/yaml.h"
#include <DirectXMath.h>

using namespace DirectX;

namespace YAML
{
    template<>
    struct convert<XMFLOAT2>
    {
        static Node encode(const XMFLOAT2& vector)
        {
            Node node;
            node.push_back(vector.x);
            node.push_back(vector.y);

            return node;
        }

        static bool decode(const Node& node, XMFLOAT2& vector)
        {
            if (!node.IsSequence() || node.size() != 2)
            {
                return false;
            }

            vector.x = node[0].as<float>();
            vector.y = node[1].as<float>();

            return true;
        }
    };

    template<>
    struct convert<XMFLOAT3>
    {
        static Node encode(const XMFLOAT3& vector)
        {
            Node node;
            node.push_back(vector.x);
            node.push_back(vector.y);
            node.push_back(vector.z);
            return node;
        }

        static bool decode(const Node& node, XMFLOAT3& vector)
        {
            if (!node.IsSequence() || node.size() != 3)
            {
                return false;
            }

            vector.x = node[0].as<float>();
            vector.y = node[1].as<float>();
            vector.z = node[2].as<float>();

            return true;
        }
    };

    template<>
    struct convert<XMFLOAT4>
    {
        static Node encode(const XMFLOAT4& vector)
        {
            Node node;
            node.push_back(vector.x);
            node.push_back(vector.y);
            node.push_back(vector.z);
            node.push_back(vector.w);

            return node;
        }

        static bool decode(const Node& node, XMFLOAT4& vector)
        {
            if (!node.IsSequence() || node.size() != 4)
            {
                return false;
            }

            vector.x = node[0].as<float>();
            vector.y = node[1].as<float>();
            vector.z = node[2].as<float>();
            vector.w = node[3].as<float>();

            return true;
        }
    };
}

namespace Aurora
{
    inline YAML::Emitter& operator<<(YAML::Emitter& outputStream, const XMFLOAT2& vector)
    {
        outputStream << YAML::Flow;
        outputStream << YAML::BeginSeq << vector.x << vector.y << YAML::EndSeq;

        return outputStream;
    }

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