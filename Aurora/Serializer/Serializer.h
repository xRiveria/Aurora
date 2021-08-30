#pragma once
#include <string>

namespace YAML
{
    class Emitter;
}

namespace Aurora
{
    class World;
    class Entity;
    class EngineContext;

    class Serializer
    {
    public:
        Serializer(EngineContext* engineContext);

        // void SerializeScene(const std::string& filePath);
        // bool DeserializeScene(const std::string& filePath);
        
    private:
        // void SerializeEntity(YAML::Emitter& outputStream, Entity* entity);

    protected:
        // Critical. The version history is logged in the ArchiveVersionHistory.txt file.
        uint64_t m_SerializerVersion = 1;           // This should always be only incremented and only if a new serialization is implemented somewhere.
        uint64_t m_SerializerVersionBarrier = 6;    // This is the version number of which any below will not be compatible with the current version.

        World* m_World;
        EngineContext* m_EngineContext;
    };
}