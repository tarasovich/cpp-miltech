#ifndef DRONE_SIMULATION_JSONCONFIGLOADER_HPP
#define DRONE_SIMULATION_JSONCONFIGLOADER_HPP
#include "FileConfigLoader.hpp"

#include <fstream>

class JsonConfigLoader : public FileConfigLoader {
protected:
    bool parseMainConfig(const fs::path &filePath) override;
    bool parseAmmoConfig(const fs::path &filePath) override;

public:
    explicit JsonConfigLoader(const std::string &mainPath = "config.json", const std::string &ammoPath = "ammo.json")
        : FileConfigLoader{mainPath, ammoPath}
    {
    }
};
#endif  // DRONE_SIMULATION_JSONCONFIGLOADER_HPP
