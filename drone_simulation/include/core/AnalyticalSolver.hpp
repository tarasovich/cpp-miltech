#ifndef DRONE_SIMULATION_ANALYTICALSOLVER_HPP
#define DRONE_SIMULATION_ANALYTICALSOLVER_HPP
#include "Config.hpp"
#include "Coord.hpp"
#include "IBallisticSolver.hpp"

namespace miltech::simulation {

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class AnalyticalSolver : public IBallisticSolver {
public:
    ~AnalyticalSolver() override = default;
    AnalyticalSolver() = default;

    Coord solve(const Coord &dronePos, const Coord &targetPos, const Config &config, const AmmoParams &ammo) override;

private:
    static float calculateBombFallTime(const Config &config, const AmmoParams &ammo);
    static float calculateBombFlightDistance(const Config &config, const AmmoParams &ammo);
};

}  // namespace miltech::simulation

#endif  // DRONE_SIMULATION_ANALYTICALSOLVER_HPP
