#pragma once

#include "FileConfigLoaderOptions.hpp"
#include "IConfigLoaderOptions.hpp"
#include "JsonConfigLoader.hpp"

namespace miltech::simulation {

class IConfigLoader;

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class ConfigLoaderFactory {
public:
    virtual ~ConfigLoaderFactory() = default;
    static IConfigLoader* create(const IConfigLoaderOptions& options)
    {
        if (const auto* fileOptions = dynamic_cast<const FileConfigLoaderOptions*>(&options)) {
            if (fileOptions->getMainName().ends_with(".json") && fileOptions->getAmmoName().ends_with(".json")) {
                const fs::path dir = fileOptions->getDir();
                // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
                return new JsonConfigLoader(dir / fileOptions->getMainName(), dir / fileOptions->getAmmoName());
            }
        }

        throw std::invalid_argument("ConfigLoaderFactory::create(): Unsupported config loader options");
    }
};

}  // namespace miltech::simulation
