#pragma once

#include "FileConfigLoader.hpp"

#include <fstream>

namespace miltech::simulation {

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

}  // namespace miltech::simulation
