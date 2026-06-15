#ifndef DRONE_SIMULATION_BALLISTICSOLUTION_HPP
#define DRONE_SIMULATION_BALLISTICSOLUTION_HPP

#include "Coord.hpp"

namespace miltech::simulation {

// ============================================================
// Визначення масиву балістичних даних
// ============================================================
struct BallisticSolution {
    float fTime;
    float hDist;
    float tgtDist;
    Coord fireCoords;
    float fireDist;
};

}  // namespace miltech::simulation

#endif  // DRONE_SIMULATION_BALLISTICSOLUTION_HPP
