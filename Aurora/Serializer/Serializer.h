#pragma once

namespace YAML
{
    class Emitter;
}

namespace Aurora
{
    class World;
    class Entity;

    class Serializer
    {
    public:
        Serializer(World* world);

        void SerializeScene(World* scene);
        bool DeserializeScene(const std::string& filePath);
        
    private:
        void SerializeEntity(YAML::Emitter& outputStream, Entity* entity);

    private:
        World* m_World;
    };
}