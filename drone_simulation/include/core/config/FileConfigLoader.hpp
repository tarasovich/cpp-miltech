#ifndef DRONE_SIMULATION_FILECONFIGLOADER_HPP
#define DRONE_SIMULATION_FILECONFIGLOADER_HPP

#include "IConfigLoader.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class FileConfigLoader : public IConfigLoader {
public:
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    explicit FileConfigLoader(const std::string &mainPath, const std::string &ammoPath)
        : mainPath(mainPath)
        , ammoPath(ammoPath)
    {
    }

    bool load() override
    {
        if (this->isLoaded) {
            throw std::logic_error("FileConfigLoader::load(): already loaded");
        }

        if (!this->parse()) {
            throw std::invalid_argument("FileConfigLoader::load(): could not load " + this->getMainPath().string());
        }

        this->isLoaded = true;

        return this->isLoaded;
    }

    fs::path getMainPath() const { return this->mainPath; }
    fs::path getAmmoPath() const { return this->ammoPath; }

    Config *getConfig() const override
    {
        this->requireLoad();

        return resultConfig;
    }

    AmmoParams *getAmmoParams() const override
    {
        this->requireLoad();

        return resultAmmoParams;
    }

    ~FileConfigLoader() override
    {
        std::cout << "destroy FileConfigLoader\n";

        if (resultConfig != nullptr) {
            delete resultConfig;
            resultConfig = nullptr;
        }

        if (resultAmmoParams != nullptr) {
            delete resultAmmoParams;
            resultAmmoParams = nullptr;
        }
    }

protected:
    Config *initResultConfig()
    {
        if (resultConfig != nullptr) {
            throw std::logic_error("FileConfigLoader::initResultConfig(): resultConfig already initialized");
        }

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        resultConfig = new Config();

        return resultConfig;
    }

    AmmoParams *initResultAmmoParams()
    {
        if (resultAmmoParams != nullptr) {
            throw std::logic_error("FileConfigLoader::initResultAmmoParams(): resultAmmoParams already initialized");
        }

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        resultAmmoParams = new AmmoParams;

        return resultAmmoParams;
    }

    virtual bool parse() = 0;

private:
    bool isLoaded = false;
    fs::path mainPath;
    fs::path ammoPath;

    Config *resultConfig = nullptr;
    AmmoParams *resultAmmoParams = nullptr;

    void requireLoad() const
    {
        if (!this->isLoaded) {
            throw std::logic_error("FileConfigLoader: not loaded");
        }
    }
};
#endif  // DRONE_SIMULATION_FILECONFIGLOADER_HPP
