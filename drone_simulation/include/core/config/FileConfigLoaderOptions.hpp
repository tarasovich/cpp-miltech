#ifndef DRONE_SIMULATION_FILECONFIGLOADEROPTIONS_HPP
#define DRONE_SIMULATION_FILECONFIGLOADEROPTIONS_HPP
#include "IConfigLoaderOptions.hpp"
#include <string>



class FileConfigLoaderOptions : public IConfigLoaderOptions {
public:
    FileConfigLoaderOptions(const std::string& dir, const std::string& mainName, const std::string& ammoName)
        : dir(dir), mainName(mainName), ammoName(ammoName) {}

    const std::string& getDir() const { return dir; }
    const std::string& getMainName() const { return mainName; }
    const std::string& getAmmoName() const { return ammoName; }

private:
    std::string dir;
    std::string mainName;
    std::string ammoName;
};

#endif  // DRONE_SIMULATION_FILECONFIGLOADEROPTIONS_HPP
