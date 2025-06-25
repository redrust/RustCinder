#include <yaml-cpp/yaml.h>
#include <iostream>

int main(int argc, char* argv[]) {
    YAML::Node config = YAML::LoadFile("config.yaml");
    if (!config) {
        std::cerr << "Failed to load config.yaml" << std::endl;
        return 1;
    }
    return 0;
}