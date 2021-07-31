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
        Serializer() = default;

        void SerializeScene(World* scene);
        
    private:
        void SerializeEntity(YAML::Emitter& outputStream, Entity* entity);
    };
}