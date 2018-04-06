#ifndef CONFIG_H
#define CONFIG_H

#include <yaml-cpp/yaml.h>
#include <string>

class Config: public YAML::Node
{
    public:
        Config(std::string file_path):
            YAML::Node(YAML::LoadFile(file_path)) {}
};

#endif // CONFIG_H
