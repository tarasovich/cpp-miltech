#ifndef DRONE_SIMULATION_FILECONFIGLOADEROPTIONS_HPP
#define DRONE_SIMULATION_FILECONFIGLOADEROPTIONS_HPP
#include "IConfigLoaderOptions.hpp"
#include <string>
#include <utility>

namespace miltech::simulation {

class FileConfigLoaderOptions : public IConfigLoaderOptions {
public:
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    FileConfigLoaderOptions(std::string dir, std::string mainName, std::string ammoName)
        : dir_(std::move(dir))
        , mainName_(std::move(mainName))
        , ammoName_(std::move(ammoName))
    {
    }

    const std::string& getDir() const { return dir_; }
    const std::string& getMainName() const { return mainName_; }
    const std::string& getAmmoName() const { return ammoName_; }

private:
    std::string dir_;
    std::string mainName_;
    std::string ammoName_;
};

}  // namespace miltech::simulation

#endif  // DRONE_SIMULATION_FILECONFIGLOADEROPTIONS_HPP
