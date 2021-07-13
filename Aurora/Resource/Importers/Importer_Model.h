#pragma once
#include <string>

namespace Aurora
{
    class EngineContext;

    class Importer_Model
    {
    public:
        Importer_Model(EngineContext* engineContext);
        ~Importer_Model() = default;

        void Load(const std::string& filePath, const std::string& albedoPath);

    private:
        void ImportModel_OBJ(const std::string& filePath, const std::string& albedoPath);

    private:
        EngineContext* m_EngineContext;
    };
}