#pragma once

#include "types.hpp"
#include "IBallisticSolver.hpp"

namespace miltech::simulation {

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class AnalyticalSolver : public IBallisticSolver {
public:
    ~AnalyticalSolver() override = default;
    AnalyticalSolver() = default;

    BallisticSolution solve(const Coord &dronePos, const Coord &targetPos, const DroneContext &ctx) override;

private:
    static float calculateBombFallTime(const DroneContext &ctx);
    static float calculateBombFlightDistance(const DroneContext &ctx, const float &fTime);
};

}  // namespace miltech::simulation
