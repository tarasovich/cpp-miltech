#ifndef DRONE_SIMULATION_CONFIGLOADERFACTORY_HPP
#define DRONE_SIMULATION_CONFIGLOADERFACTORY_HPP

#include "FileConfigLoaderOptions.hpp"
#include "IConfigLoader.hpp"
#include "IConfigLoaderOptions.hpp"
#include "JsonConfigLoader.hpp"

class ConfigLoaderFactory {
public:
    virtual ~ConfigLoaderFactory() = default;
    static IConfigLoader* create(const IConfigLoaderOptions& options)
    {
        if (const auto* fileOptions = dynamic_cast<const FileConfigLoaderOptions*>(&options)) {
            if (fileOptions->getMainName().ends_with(".json") || fileOptions->getAmmoName().ends_with(".json")) {
                const fs::path dir = fileOptions->getDir();
                return new JsonConfigLoader(dir / fileOptions->getMainName(), dir / fileOptions->getAmmoName());
            }
        }

        throw std::invalid_argument("ConfigLoaderFactory::create(): Unsupported config loader options");
    }
};

#endif  // DRONE_SIMULATION_CONFIGLOADERFACTORY_HPP
