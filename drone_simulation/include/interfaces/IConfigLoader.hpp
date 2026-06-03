#pragma once

#include "AmmoParams.hpp"
#include "Config.hpp"

namespace miltech::simulation {

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class IConfigLoader {
public:
    virtual bool load() = 0;
    virtual Config *getConfig() const = 0;
    virtual AmmoParams *getAmmoParams() const = 0;
    virtual ~IConfigLoader() = default;
};

}  // namespace miltech::simulation
