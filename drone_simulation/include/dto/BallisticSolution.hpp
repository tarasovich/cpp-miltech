#pragma once

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
