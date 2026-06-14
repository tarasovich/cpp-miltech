#pragma once

#include "types.hpp"
#include "IConfigLoader.hpp"
#include <filesystem>

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

    std::unique_ptr<Config> getConfig() override
    {
        this->requireLoad();

        return std::move(resultConfig_);
    }

    std::unique_ptr<AmmoParams> getAmmoParams() override
    {
        this->requireLoad();

        return std::move(resultAmmoParams_);
    }

protected:
    Config *initResultConfig()
    {
        if (resultConfig_ != nullptr) {
            throw std::logic_error("FileConfigLoader::initResultConfig(): resultConfig already initialized");
        }

        resultConfig_ = std::make_unique<Config>();

        return resultConfig_.get();
    }

    AmmoParams *initResultAmmoParams()
    {
        if (resultAmmoParams_ != nullptr) {
            throw std::logic_error("FileConfigLoader::initResultAmmoParams(): resultAmmoParams already initialized");
        }

        resultAmmoParams_ = std::make_unique<AmmoParams>();

        return resultAmmoParams_.get();
    }

    virtual bool parse() = 0;

private:
    bool isLoaded_{false};
    fs::path mainPath_;
    fs::path ammoPath_;

    std::unique_ptr<Config> resultConfig_;
    std::unique_ptr<AmmoParams> resultAmmoParams_;

    void requireLoad() const
    {
        if (!isLoaded_) {
            throw std::logic_error("FileConfigLoader: not loaded");
        }
    }
};

}  // namespace miltech::simulation
