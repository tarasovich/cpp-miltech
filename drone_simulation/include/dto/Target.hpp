#pragma once

#include "Coord.hpp"
#include <cstdint>

namespace miltech::simulation {

struct Target {
    uint8_t timeSteps;
    Coord* positions;
};

}  // namespace miltech::simulation
