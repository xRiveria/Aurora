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
        void AddProperty(const std::string& keyName, std::vector<uint32_t>& keyValues);
        void AddProperty(const std::string& keyName, std::vector<XMFLOAT3>& keyValues);
        void AddProperty(const std::string& keyName, std::vector<XMFLOAT2>& keyValues);

        // Deserialization
        bool LoadFromFile(const std::string& fileType);
        bool ValidateFileType(const std::string& fileType);
        bool ValidateKey(const std::string& keyName);
        bool ValidateKeyAndValue(const std::string& keyName, const std::string& keyValue);

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
        bool GetProperty(const std::string& keyName, std::vector<uint32_t>* values);
        bool GetProperty(const std::string& keyName, std::vector<XMFLOAT2>* values);
        bool GetProperty(const std::string& keyName, std::vector<XMFLOAT3>* values);

        bool GetPropertyFromSubNode(const std::string& subNode, const std::string& keyName, std::string* value);
        bool GetPropertyFromSubNode(const std::string& subNode, const std::string& keyName, uint32_t* value);
        bool GetPropertyFromSubNode(const std::string& subNode, const std::string& mapNode, const std::string& keyName, std::string* value);
        bool GetPropertyFromSubNode(const std::string& subNode, const std::string& mapNode, const std::string& keyName, uint32_t* value);

    private:
        YAML::Emitter m_ActiveEmitter;
        YAML::Node m_ActiveNode;
    };
}