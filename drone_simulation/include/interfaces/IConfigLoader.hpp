#ifndef DRONE_SIMULATION_ICONFIGLOADER_HPP
#define DRONE_SIMULATION_ICONFIGLOADER_HPP
#include "AmmoParams.hpp"
#include "Config.hpp"

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class IConfigLoader {
public:
    virtual bool load() = 0;
    virtual Config *getConfig() const = 0;
    virtual AmmoParams *getAmmoParams() const = 0;
    virtual ~IConfigLoader() = default;
};

#endif  // DRONE_SIMULATION_ICONFIGLOADER_HPP
