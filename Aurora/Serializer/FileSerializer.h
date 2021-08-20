#pragma once
#include "Serializer.h"

namespace Aurora
{
    class FileSerializer : public Serializer
    {
    public:
        FileSerializer(EngineContext* engineContext);

        // Serialization
        bool BeginSerialization(const std::string& fileType);
        bool EndSerialization(const std::string& filePath);
        void AddMapKey(const std::string& keyName);
        void EndMapKey();

        // Serializable Types
        void AddProperty(const std::string& keyName, const std::string& keyValue);
        void AddProperty(const std::string& keyName, bool keyValue);
        void AddProperty(const std::string& keyName, int keyValue);
        void AddProperty(const std::string& keyName, uint32_t keyValue);
        void AddProperty(const std::string& keyName, float keyValue);
        void AddProperty(const std::string& keyName, double keyValue);
        void AddProperty(const std::string& keyName, XMFLOAT2& keyValue);
        void AddProperty(const std::string& keyName, XMFLOAT3& keyValue);
        void AddProperty(const std::string& keyName, XMFLOAT4& keyValue);

        // Deserialization
        bool LoadFromFile(const std::string& fileType);
        bool KeyExists(const std::string& nodeName);

        // Deserializable Types
        bool GetProperty(const std::string& keyName, std::string* value);
        bool GetProperty(const std::string& keyName, bool* value);
        bool GetProperty(const std::string& keyName, int* value);
        bool GetProperty(const std::string& keyName, uint32_t* value);
        bool GetProperty(const std::string& keyName, float* value);
        bool GetProperty(const std::string& keyName, double* value);
        bool GetProperty(const std::string& keyName, XMFLOAT2* value);
        bool GetProperty(const std::string& keyName, XMFLOAT3* value);
        bool GetProperty(const std::string& keyName, XMFLOAT4* value);

        bool GetPropertyFromSubNode(const std::string& subNode, const std::string& keyName, uint32_t* value);
        bool GetPropertyFromSubNode(const std::string& subNode, const std::string& mapNode, const std::string& keyName, std::string* value);
        bool GetPropertyFromSubNode(const std::string& subNode, const std::string& mapNode, const std::string& keyName, uint32_t* value);

    private:
        YAML::Emitter m_ActiveEmitter;
        YAML::Node m_ActiveNode;
    };
}