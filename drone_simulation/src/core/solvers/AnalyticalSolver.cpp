#include "AnalyticalSolver.hpp"
#include <cmath>

namespace miltech::simulation {

constexpr float M_GI = 9.81;

BallisticSolution AnalyticalSolver::solve(const Coord &dronePos, const Coord &targetPos, const Config &config, const AmmoParams &ammo)
{
    const float fTime = calculateBombFallTime(config, ammo);
    const float hDist = calculateBombFlightDistance(config, ammo, fTime);

    float tgtDist = std::sqrt(std::pow(targetPos.x - dronePos.x, 2.0f) + std::pow(targetPos.y - dronePos.y, 2.0f));
    if (std::fabs(tgtDist) < 1e-6f) {
        tgtDist = 1e-6f;
    }

    const float dxT = targetPos.x - dronePos.x;
    const float dyT = targetPos.y - dronePos.y;

    const auto fireCoords = Coord{
        .x = targetPos.x - (dxT / tgtDist * hDist),
        .y = targetPos.y - (dyT / tgtDist * hDist),
    };

    const float fireDist = std::sqrt(std::pow(fireCoords.x - dronePos.x, 2.0f) + std::pow(fireCoords.y - dronePos.y, 2.0f));

    return BallisticSolution{.fTime = fTime, .hDist = hDist, .tgtDist = tgtDist, .fireCoords = fireCoords, .fireDist = fireDist};
}

float AnalyticalSolver::calculateBombFallTime(const Config &config, const AmmoParams &ammo)
{
    const float d = ammo.drag;
    const float m = ammo.mass;
    const float l = ammo.lift;

    // a = d·g·m − 2d²·l·V₀
    // b = −3g·m² + 3d·l·m·V₀
    // c = 6m²·Z₀
    float a{d * M_GI * m};
    float b{-3.0f * M_GI * m * m};
    const float c{6.0f * m * m * config.altitude};
    if (l != 0.0f) {
        // спрощення формули при l=0
        a -= 2.0f * d * d * l * config.attackSpeed;
        b += 3.0f * d * l * m * config.attackSpeed;
    }

    // p = − b² / (3a²)
    const float p = -1.0f * (b * b / (3.0f * a * a));
    if (p >= 0) {
        throw std::runtime_error("AnalyticalSolver::calculateBombFallTime(): No real solution for input data");
    }

    // q = 2b³ / (27a³) + c / a
    const float q = ((2.0f * b * b * b) / (27.0f * a * a * a)) + (c / a);

    // φ = arccos( 3q / (2p) · √(−3/p) )
    const float argArc = 3.0f * q / (2.0f * p) * std::sqrt(-3.0f / p);
    if (argArc < -1.0f || argArc > 1.0f) {
        throw std::runtime_error("AnalyticalSolver::calculateBombFallTime(): No real solution for input data");
    }

    const float phi = std::acos(argArc);

    // t = 2√(−p/3) · cos( (φ + 4π) / 3 ) − b / (3a)
    const float fTime = (2.0f * std::sqrt(-p / 3.0f) * std::cos((phi + static_cast<float>(4.0f * M_PI)) / 3.0f)) - (b / (3.0f * a));
    if (fTime <= 0.0f) {
        throw std::runtime_error("AnalyticalSolver::calculateBombFallTime(): No real solution for input data");
    }

    return fTime;
}

float AnalyticalSolver::calculateBombFlightDistance(const Config &config, const AmmoParams &ammo, const float &fTime)
{
    const float d = ammo.drag;
    const float m = ammo.mass;
    const float l = ammo.lift;

    // h = V₀t
    //   − t²d·V₀/(2m)
    //   + t³(6d·g·l·m − 6d²(l²-1)·V₀)/(36m²)
    //   + t⁴ (−6d²g·l·(1+l²+l⁴)m + 3d³l²(1+l²)V₀ + 6d³l⁴(1+l²)V₀) / (36(1+l²)²m³)
    //   + t⁵(3d³g·l³m − 3d⁴l²(1+l²)V₀) / (36(1+l²)m⁴)
    const float t2{fTime * fTime};  // для спрощення запису рівняння
    const float m2{m * m};
    const float d2{d * d};
    const float l2{l * l};
    float hDist = (config.attackSpeed * fTime) - (t2 * d * config.attackSpeed / (2.0f * m)) +
                  (t2 * fTime * ((6.0f * d * M_GI * l * m) - (6.0f * d2 * (l2 - 1.0f) * config.attackSpeed)) / (36.0f * m2));
    if (l != 0.0f) {
        // спрощення формули при l=0
        const float l2p1{l2 + 1.0f};
        hDist += (t2 * t2 *
                  ((-6.0f * d2 * M_GI * l * (l2p1 + (l2 * l2)) * m) + (3.0f * d2 * d * l2 * l2p1 * config.attackSpeed) +
                   (6.0f * d2 * d * l2 * l2 * l2p1 * config.attackSpeed)) /
                  (36.0f * l2p1 * l2p1 * m2 * m)) +
                 (t2 * t2 * fTime * ((3.0f * d2 * d * M_GI * l2 * l * m) - (3.0f * d2 * d2 * l2 * l2p1 * config.attackSpeed)) /
                  (36.0f * l2p1 * m2 * m2));
    }

    if (hDist <= 0.0f) {
        throw std::runtime_error("AnalyticalSolver::calculateBombFlightDistance(): No real solution for input data");
    }

    return hDist;
}

}  // namespace miltech::simulation
