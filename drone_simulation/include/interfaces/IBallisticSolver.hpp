#ifndef DRONE_SIMULATION_IBALLISTICSOLVER_HPP
#define DRONE_SIMULATION_IBALLISTICSOLVER_HPP

#include "Coord.hpp"
#include "Config.hpp"
#include "AmmoParams.hpp"
#include "BallisticSolution.hpp"

namespace miltech::simulation {

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class IBallisticSolver {
public:
    virtual ~IBallisticSolver() = default;
    virtual BallisticSolution solve(const Coord &dronePos, const Coord &targetPos, const Config &config, const AmmoParams &ammo) = 0;
};

}  // namespace miltech::simulation

#endif  // DRONE_SIMULATION_IBALLISTICSOLVER_HPP
