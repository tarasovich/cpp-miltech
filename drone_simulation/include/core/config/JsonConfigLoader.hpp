#ifndef DRONE_SIMULATION_JSONCONFIGLOADER_HPP
#define DRONE_SIMULATION_JSONCONFIGLOADER_HPP
#include "FileConfigLoader.hpp"

#include <fstream>

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class JsonConfigLoader : public FileConfigLoader {
protected:
    bool parse() override;

public:
    ~JsonConfigLoader() override = default;
    explicit JsonConfigLoader(const std::string &mainPath = "config.json", const std::string &ammoPath = "ammo.json")
        : FileConfigLoader{mainPath, ammoPath}
    {
    }
};
#endif  // DRONE_SIMULATION_JSONCONFIGLOADER_HPP
