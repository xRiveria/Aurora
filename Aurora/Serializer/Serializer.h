#pragma once

namespace Aurora
{
    class World;

    class Serializer
    {
    public:
        Serializer() = default;

        void SerializeScene(World* scene);
    };
}