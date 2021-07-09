#pragma once
#include <string>

namespace Aurora
{
    class Importer_Model
    {
    public:
        Importer_Model(EngineContext* engineContext);

        void LoadModel_OBJ(const std::string& filePath);

    private:
        EngineContext* m_EngineContext;
    };
}