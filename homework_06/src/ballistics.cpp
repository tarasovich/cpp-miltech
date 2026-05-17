#include "ballistics.hpp"

#include <cmath>
#include <cstring>
#include <stdexcept>

void validateInput(const BallisticsInput &input, AmmoParams &outAmmoParams) {
    // навряд скоро будуть такі дрони
    if (input.attackSpeed < 0 || input.attackSpeed > 60) {
        throw std::invalid_argument("Attack speed out of range");
    }

    // з міркувань безпеки самого дрону та точності розрахунків
    if (input.altitude < 10) {
        throw std::invalid_argument("Drone too low");
    }

    //
    if (input.accelerationPath < 0) {
        throw std::invalid_argument("Acceleration path out of range");
    }

    constexpr size_t ammosCount = std::size(ammos);
    for (size_t i = 0; i < ammosCount; i++) {
        if (std::strcmp(input.ammoName, ammos[i].name) == 0) {
            outAmmoParams = ammos[i];
            return;
        }
    }

    throw std::invalid_argument("Unknow ammo \"" + std::string(input.ammoName) + "\"");
}

// ============================================================
// Балістична задача - часу польоту (метод Кардано)
// ============================================================
float calculateBombFallTime(const BallisticsInput &input, const AmmoParams &ammo) {
    const float d = ammo.drag;
    const float m = ammo.mass;
    const float l = ammo.lift;

    // a = d·g·m − 2d²·l·V₀
    // b = −3g·m² + 3d·l·m·V₀
    // c = 6m²·Z₀
    float a{d * M_GI * m};
    float b{-3.0f * M_GI * m * m};
    const float c{6.0f * m * m * input.altitude};
    if (l != 0.0f) {
        // спрощення формули при l=0
        a -= 2.0f * d * d * l * input.attackSpeed;
        b += 3.0f * d * l * m * input.attackSpeed;
    }

    // p = − b² / (3a²)
    const float p = -1.0f * (b * b / (3.0f * a * a));
    if (p >= 0) {
        return false;
    }

    // q = 2b³ / (27a³) + c / a
    const float q = (2.0f * b * b * b) / (27.0f * a * a * a) + c / a;

    // φ = arccos( 3q / (2p) · √(−3/p) )
    const float argArc = 3.0f * q / (2.0f * p) * std::sqrt(-3.0f / p);
    if (argArc < -1.0f || argArc > 1.0f) {
        throw std::logic_error("No real solution for input data");
    }

    const float phi = std::acos(argArc);

    // t = 2√(−p/3) · cos( (φ + 4π) / 3 ) − b / (3a)
    return 2.0f * std::sqrt(-p / 3.0f) * std::cos((phi + static_cast<float>(4.0f * M_PI)) / 3.0f) - b / (3.0f * a);
}

// ============================================================
// Балістична задача - горизонтальна дистанція (степеневий ряд до t⁵)
// ============================================================
float calculateBombFlightDistance(const BallisticsInput &input, const AmmoParams &ammo, float t) {
    const float d = ammo.drag;
    const float m = ammo.mass;
    const float l = ammo.lift;

    // h = V₀t
    //   − t²d·V₀/(2m)
    //   + t³(6d·g·l·m − 6d²(l²-1)·V₀)/(36m²)
    //   + t⁴ (−6d²g·l·(1+l²+l⁴)m + 3d³l²(1+l²)V₀ + 6d³l⁴(1+l²)V₀) / (36(1+l²)²m³)
    //   + t⁵(3d³g·l³m − 3d⁴l²(1+l²)V₀) / (36(1+l²)m⁴)
    const float t2{t * t}, // для спрощення запису рівняння
            m2{m * m},
            d2{d * d},
            l2{l * l};
    float hDist = input.attackSpeed * t
                  - t2 * d * input.attackSpeed / (2.0f * m)
                  + t2 * t * (6.0f * d * M_GI * l * m - 6.0f * d2 * (l2 - 1.0f) * input.attackSpeed) / (36.0f * m2);
    if (l != 0.0f) {
        // спрощення формули при l=0
        const float l2p1{l2 + 1.0f};
        hDist += t2 * t2 * (-6.0f * d2 * M_GI * l * (l2p1 + l2 * l2) * m + 3.0f * d2 * d * l2 * l2p1 * input.attackSpeed + 6.0f * d2 * d * l2 * l2 * l2p1 * input.attackSpeed)
                / (
                    36.0f * l2p1 * l2p1 * m2 * m)
                + t2 * t2 * t * (3.0f * d2 * d * M_GI * l2 * l * m - 3.0f * d2 * d2 * l2 * l2p1 * input.attackSpeed) / (36.0f * l2p1 * m2 * m2);
    }

    if (hDist <= 0.0f) {
        throw std::logic_error("No real solution for input data");
    }

    return hDist;
}

// ============================================================
// Балістична задача - точка скиду
// ============================================================
BallisticsResult calculateBallistics(const BallisticsInput &input) {
    AmmoParams ammoParams{};

    validateInput(input, ammoParams);

    const float fallTime = calculateBombFallTime(input, ammoParams);
    const float hDist = calculateBombFlightDistance(input, ammoParams, fallTime);

    const float dxT = input.targetX - input.droneX;
    const float dyT = input.targetY - input.droneY;

    float tgtDist = std::hypot(dxT, dyT);
    if (std::fabs(tgtDist) < 1e-6f) {
        tgtDist = 1e-6f;
    }

    return BallisticsResult{
        input.targetX - dxT / tgtDist * hDist,
        input.targetY - dyT / tgtDist * hDist,
    };
}
