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
    static std::unique_ptr<IConfigLoader> create(const IConfigLoaderOptions& options)
    {
        if (const auto* fileOptions = dynamic_cast<const FileConfigLoaderOptions*>(&options)) {
            if (fileOptions->getMainName().ends_with(".json") && fileOptions->getAmmoName().ends_with(".json")) {
                const fs::path dir = fileOptions->getDir();
                return std::make_unique<JsonConfigLoader>(dir / fileOptions->getMainName(), dir / fileOptions->getAmmoName());
            }
        }

        throw std::invalid_argument("ConfigLoaderFactory::create(): Unsupported config loader options");
    }
};

}  // namespace miltech::simulation
