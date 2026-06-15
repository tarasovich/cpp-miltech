#pragma once

#include <cmath>

namespace miltech::simulation {

// ============================================================
// Нормалізація кута
// ============================================================
inline float normalizeAngle(float angle)
{
    while (angle > M_PI) {
        angle -= 2.0f * M_PI;
    }

    while (angle < -M_PI) {
        angle += 2.0f * M_PI;
    }

    return angle;
}

}  // namespace miltech::simulation
