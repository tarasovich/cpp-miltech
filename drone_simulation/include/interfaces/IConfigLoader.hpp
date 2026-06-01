#ifndef DRONE_SIMULATION_ICONFIGLOADER_HPP
#define DRONE_SIMULATION_ICONFIGLOADER_HPP
#include "AmmoParams.hpp"
#include "Config.hpp"

class IConfigLoader {
public:
    virtual bool load() = 0;
    virtual Config *getConfig() = 0;
    virtual AmmoParams *getAmmoParams() = 0;
    virtual ~IConfigLoader() = default;
};

#endif  // DRONE_SIMULATION_ICONFIGLOADER_HPP
