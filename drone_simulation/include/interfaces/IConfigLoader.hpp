#pragma once

#include <memory>

namespace miltech::simulation {

struct Config;
struct AmmoParams;

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class IConfigLoader {
public:
    virtual bool load() = 0;
    virtual std::unique_ptr<Config> getConfig() = 0;
    virtual std::unique_ptr<AmmoParams> getAmmoParams() = 0;
    virtual ~IConfigLoader() = default;
};

}  // namespace miltech::simulation
