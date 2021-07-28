#include "Aurora.h"
#include "Serializer.h"
#include "../Scene/World.h"
#include <yaml-cpp/yaml.h>
#include <fstream>

namespace Aurora
{
    void Serializer::SerializeScene(World* scene)
    {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene";
        out << YAML::Value << "Name";
        out << YAML::Key << "Entities";

        out << YAML::EndMap;

        std::ofstream fout("../Resources/Scenes/TestScene.aurora");
        fout << out.c_str();
        AURORA_INFO("Scene Successfully Serialized.");
    }
}