#ifndef DRONE_SIMULATION_FILECONFIGLOADEROPTIONS_HPP
#define DRONE_SIMULATION_FILECONFIGLOADEROPTIONS_HPP
#include "IConfigLoaderOptions.hpp"
#include <string>
#include <utility>

class FileConfigLoaderOptions : public IConfigLoaderOptions {
public:
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    FileConfigLoaderOptions(std::string dir, std::string mainName, std::string ammoName)
        : dir(std::move(dir))
        , mainName(std::move(mainName))
        , ammoName(std::move(ammoName))
    {
    }

    const std::string& getDir() const { return dir; }
    const std::string& getMainName() const { return mainName; }
    const std::string& getAmmoName() const { return ammoName; }

private:
    std::string dir;
    std::string mainName;
    std::string ammoName;
};

#endif  // DRONE_SIMULATION_FILECONFIGLOADEROPTIONS_HPP
