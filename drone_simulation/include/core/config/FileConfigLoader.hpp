#ifndef DRONE_SIMULATION_FILECONFIGLOADER_HPP
#define DRONE_SIMULATION_FILECONFIGLOADER_HPP

#include "IConfigLoader.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

class FileConfigLoader : public IConfigLoader {
public:
    explicit FileConfigLoader(const std::string &mainPath, const std::string &ammoPath)
        : mainPath(mainPath)
        , ammoPath(ammoPath)
    {
    }

    bool load() override;

    fs::path getMainPath() const { return this->mainPath; }
    fs::path getAmmoPath() const { return this->ammoPath; }

    Config *getConfig() override
    {
        this->requireLoad();

        return resultConfig;
    }
    AmmoParams *getAmmoParams() override
    {
        this->requireLoad();

        return resultAmmoParams;
    }

    ~FileConfigLoader() override
    {
        std::cout << "destroy FileConfigLoader\n";

        delete resultConfig;
        resultConfig = nullptr;

        delete[] resultAmmoParams;
        resultAmmoParams = nullptr;
    }

    bool isLoaded = false;

protected:
    Config *resultConfig = nullptr;
    AmmoParams *resultAmmoParams = nullptr;

    virtual bool parseMainConfig(const fs::path &filePath) = 0;
    virtual bool parseAmmoConfig(const fs::path &filePath) = 0;

private:
    fs::path mainPath;
    fs::path ammoPath;

    void requireLoad() const
    {
        if (!this->isLoaded) {
            throw std::logic_error("FileConfigLoader: not loaded");
        }
    }
};
#endif  // DRONE_SIMULATION_FILECONFIGLOADER_HPP
