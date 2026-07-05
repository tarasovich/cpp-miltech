#pragma once

#include "types.hpp"

namespace miltech::simulation {

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class IBallisticSolver {
public:
    virtual ~IBallisticSolver() = default;
    virtual BallisticSolution solve(const Coord &dronePos, const Coord &targetPos, const DroneContext &ctx) = 0;
};

}  // namespace miltech::simulation
