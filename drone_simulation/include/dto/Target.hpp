#ifndef DRONE_SIMULATION_TARGET_HPP
#define DRONE_SIMULATION_TARGET_HPP

#include "Coord.hpp"
#include <cstdint>

namespace miltech::simulation {

struct Target {
    uint8_t timeSteps;
    Coord* positions;
};

}  // namespace miltech::simulation

#endif  // DRONE_SIMULATION_TARGET_HPP
