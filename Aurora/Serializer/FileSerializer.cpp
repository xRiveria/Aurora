#include "Aurora.h"
#include "FileSerializer.h"
#include "SerializerUtilities.h"
#include <yaml-cpp/yaml.h>
#include <fstream>

namespace Aurora
{
    FileSerializer::FileSerializer(EngineContext* engineContext) : Serializer(engineContext)
    {

    }

    bool FileSerializer::BeginSerialization(const std::string& fileType)
    {
        m_ActiveEmitter << YAML::BeginMap;
        m_ActiveEmitter << YAML::Key << "Type";
        m_ActiveEmitter << YAML::Value << fileType;

        return true;
    }

    bool FileSerializer::EndSerialization(const std::string& filePath)
    {
        m_ActiveEmitter << YAML::EndMap;

        std::ofstream outputFile(filePath);
        outputFile << m_ActiveEmitter.c_str();

        AURORA_INFO(LogLayer::Serialization, "File \"%s\" Successfully Serialized.", filePath.c_str());

        return true;
    }

    void FileSerializer::AddMapKey(const std::string& keyName)
    {
        m_ActiveEmitter << YAML::Key << keyName;
        m_ActiveEmitter << YAML::BeginMap;
    }

    void FileSerializer::EndMapKey()
    {
        m_ActiveEmitter << YAML::EndMap;
    }

    bool FileSerializer::LoadFromFile(const std::string& filePath)
    {
        m_ActiveNode = YAML::LoadFile(filePath); // sceneData now contains our scene's data as read from the stream.
        return true;
    }

    bool FileSerializer::ValidateFileType(const std::string& fileType)
    {
        if (m_ActiveNode["Type"].as<std::string>() != fileType)
        {
            return false;
        }

        return true;
    }

    bool FileSerializer::ValidateKey(const std::string& keyName)
    {
        if (!m_ActiveNode[keyName])
        {
            return false;
        }

        return true;
    }

    bool FileSerializer::ValidateKeyAndValue(const std::string& keyName, const std::string& keyValue)
    {
        if (m_ActiveNode[keyName].as<std::string>() == keyValue) // Is this really the type of file we're looking for? If there's no node for it within, we return immediately.
        {
            return true;
        }

        return false;;
    }

    bool FileSerializer::GetProperty(const std::string& keyName, std::string* value)
    {
        *value = m_ActiveNode[keyName].as<std::string>();
        return true;
    }

    bool FileSerializer::GetProperty(const std::string& keyName, bool* value)
    {
        *value = m_ActiveNode[keyName].as<bool>();
        return true;
    }

    bool FileSerializer::GetProperty(const std::string& keyName, int* value)
    {
        *value = m_ActiveNode[keyName].as<int>();
        return true;
    }

    bool FileSerializer::GetProperty(const std::string& keyName, uint32_t* value)
    {
        *value = m_ActiveNode[keyName].as<uint32_t>();
        return true;
    }

    bool FileSerializer::GetProperty(const std::string& keyName, float* value)
    {
        *value = m_ActiveNode[keyName].as<float>();
        return true;
    }

    bool FileSerializer::GetProperty(const std::string& keyName, double* value)
    {
        *value = m_ActiveNode[keyName].as<double>();
        return true;
    }

    bool FileSerializer::GetProperty(const std::string& keyName, XMFLOAT2* value)
    {
        *value = m_ActiveNode[keyName].as<XMFLOAT2>();
        return true;
    }

    bool FileSerializer::GetProperty(const std::string& keyName, XMFLOAT3* value)
    {
        *value = m_ActiveNode[keyName].as<XMFLOAT3>();
        return true;
    }

    bool FileSerializer::GetProperty(const std::string& keyName, XMFLOAT4* value)
    {
        *value = m_ActiveNode[keyName].as<XMFLOAT4>();
        return true;
    }

    bool FileSerializer::GetPropertyFromSubNode(const std::string& subNode, const std::string& keyName, std::string * value)
    {
        YAML::Node node = m_ActiveNode[subNode];
        *value = node[keyName].as<std::string>();

        return true;
    }

    bool FileSerializer::GetPropertyFromSubNode(const std::string& subNode, const std::string& keyName, uint32_t* value)
    {
        YAML::Node node = m_ActiveNode[subNode];
        *value = node[keyName].as<uint32_t>();

        return true;
    }

    bool FileSerializer::GetPropertyFromSubNode(const std::string& subNode, const std::string& mapKey, const std::string& keyName, std::string* value)
    {
        YAML::Node node = m_ActiveNode[subNode];
        *value = node[mapKey][keyName].as<std::string>();

        return true;
    }

    bool FileSerializer::GetPropertyFromSubNode(const std::string& subNode, const std::string& mapKey, const std::string& keyName, uint32_t* value)
    {
        YAML::Node node = m_ActiveNode[subNode];
        *value = node[mapKey][keyName].as<uint32_t>();

        return true;
    }

    void FileSerializer::AddProperty(const std::string& keyName, const std::string& keyValue)
    {
        m_ActiveEmitter << YAML::Key << keyName;
        m_ActiveEmitter << YAML::Value << keyValue;
    }

    void FileSerializer::AddProperty(const std::string& keyName, bool keyValue)
    {
        m_ActiveEmitter << YAML::Key << keyName;
        m_ActiveEmitter << YAML::Value << keyValue;
    }

    void FileSerializer::AddProperty(const std::string& keyName, int keyValue)
    {
        m_ActiveEmitter << YAML::Key << keyName;
        m_ActiveEmitter << YAML::Value << keyValue;
    }

    void FileSerializer::AddProperty(const std::string& keyName, uint32_t keyValue)
    {
        m_ActiveEmitter << YAML::Key << keyName;
        m_ActiveEmitter << YAML::Value << keyValue;
    }

    void FileSerializer::AddProperty(const std::string& keyName, float keyValue)
    {
        m_ActiveEmitter << YAML::Key << keyName;
        m_ActiveEmitter << YAML::Value << keyValue;
    }

    void FileSerializer::AddProperty(const std::string& keyName, double keyValue)
    {
        m_ActiveEmitter << YAML::Key << keyName;
        m_ActiveEmitter << YAML::Value << keyValue;
    }

    void FileSerializer::AddProperty(const std::string& keyName, XMFLOAT2& keyValue)
    {
        m_ActiveEmitter << YAML::Key << keyName;
        m_ActiveEmitter << YAML::Value << keyValue;
    }

    void FileSerializer::AddProperty(const std::string& keyName, XMFLOAT3& keyValue)
    {
        m_ActiveEmitter << YAML::Key << keyName;
        m_ActiveEmitter << YAML::Value << keyValue;
    }

    void FileSerializer::AddProperty(const std::string& keyName, XMFLOAT4& keyValue)
    {
        m_ActiveEmitter << YAML::Key << keyName;
        m_ActiveEmitter << YAML::Value << keyValue;
    }

    void FileSerializer::AddProperty(const std::string& keyName, std::vector<uint32_t>& keyValues)
    {
        m_ActiveEmitter << YAML::Key << keyName;
        m_ActiveEmitter << YAML::Value << keyValues;
    }

    bool FileSerializer::GetProperty(const std::string& keyName, std::vector<uint32_t>* values)
    {
        *values = m_ActiveNode[keyName].as<std::vector<uint32_t>>();
        return true;
    }

    bool FileSerializer::GetProperty(const std::string& keyName, std::vector<XMFLOAT2>* values)
    {
        uint32_t vectorCount = m_ActiveNode[(keyName + "_" + "Count")].as<uint32_t>();
        *values = std::vector<XMFLOAT2>(vectorCount);

        for (uint32_t i = 0; i < vectorCount; i++)
        {
            (*values)[i] = m_ActiveNode[(keyName + "_" + std::to_string(i))].as<XMFLOAT2>();
        }

        return true;
    }

    bool FileSerializer::GetProperty(const std::string& keyName, std::vector<XMFLOAT3>* values)
    {
        uint32_t vectorCount = m_ActiveNode[(keyName + "_" + "Count")].as<uint32_t>();
        *values = std::vector<XMFLOAT3>(vectorCount);

        for (uint32_t i = 0; i < vectorCount; i++)
        {
            (*values)[i] = m_ActiveNode[(keyName + "_" + std::to_string(i))].as<XMFLOAT3>();
        }

        return true;
    }

    void FileSerializer::AddProperty(const std::string& keyName, std::vector<XMFLOAT2>& keyValues)
    {
        m_ActiveEmitter << YAML::Key << (keyName + "_" + "Count") << YAML::Value << static_cast<uint32_t>(keyValues.size());
        for (int i = 0; i < keyValues.size(); i++)
        {
            m_ActiveEmitter << YAML::Key << (keyName + "_" + std::to_string(i));
            m_ActiveEmitter << YAML::Value << keyValues[i];
        }
    }

    void FileSerializer::AddProperty(const std::string& keyName, std::vector<XMFLOAT3>& keyValues)
    {
        m_ActiveEmitter << YAML::Key << (keyName + "_" + "Count") << YAML::Value << static_cast<uint32_t>(keyValues.size());
        for (int i = 0; i < keyValues.size(); i++)
        {
            m_ActiveEmitter << YAML::Key << keyName + "_" + std::to_string(i);
            m_ActiveEmitter << YAML::Value << keyValues[i];
        }
    }
}