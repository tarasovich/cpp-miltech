#pragma once

#include "IConfigLoader.hpp"
#include <filesystem>
#include <iostream>

namespace miltech::simulation {

namespace fs = std::filesystem;

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class FileConfigLoader : public IConfigLoader {
public:
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    explicit FileConfigLoader(const std::string &mainPath, const std::string &ammoPath)
        : mainPath_(mainPath)
        , ammoPath_(ammoPath)
    {
    }

    bool load() override
    {
        if (isLoaded_) {
            throw std::logic_error("FileConfigLoader::load(): already loaded");
        }

        if (!this->parse()) {
            throw std::invalid_argument("FileConfigLoader::load(): could not load " + this->getMainPath().string());
        }

        isLoaded_ = true;

        return isLoaded_;
    }

    fs::path getMainPath() const { return this->mainPath_; }
    fs::path getAmmoPath() const { return this->ammoPath_; }

    Config *getConfig() const override
    {
        this->requireLoad();

        return resultConfig_;
    }

    AmmoParams *getAmmoParams() const override
    {
        this->requireLoad();

        return resultAmmoParams_;
    }

    ~FileConfigLoader() override
    {
        std::cout << "FileConfigLoader::destructor\n";

        if (resultConfig_ != nullptr) {
            delete resultConfig_;
            resultConfig_ = nullptr;
        }

        if (resultAmmoParams_ != nullptr) {
            delete resultAmmoParams_;
            resultAmmoParams_ = nullptr;
        }
    }

protected:
    Config *initResultConfig()
    {
        if (resultConfig_ != nullptr) {
            throw std::logic_error("FileConfigLoader::initResultConfig(): resultConfig already initialized");
        }

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        resultConfig_ = new Config();

        return resultConfig_;
    }

    AmmoParams *initResultAmmoParams()
    {
        if (resultAmmoParams_ != nullptr) {
            throw std::logic_error("FileConfigLoader::initResultAmmoParams(): resultAmmoParams already initialized");
        }

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        resultAmmoParams_ = new AmmoParams;

        return resultAmmoParams_;
    }

    virtual bool parse() = 0;

private:
    bool isLoaded_{false};
    fs::path mainPath_;
    fs::path ammoPath_;

    Config *resultConfig_ = nullptr;
    AmmoParams *resultAmmoParams_ = nullptr;

    void requireLoad() const
    {
        if (!isLoaded_) {
            throw std::logic_error("FileConfigLoader: not loaded");
        }
    }
};

}  // namespace miltech::simulation
