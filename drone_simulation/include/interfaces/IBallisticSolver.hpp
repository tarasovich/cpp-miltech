#ifndef DRONE_SIMULATION_IBALLISTICSOLVER_HPP
#define DRONE_SIMULATION_IBALLISTICSOLVER_HPP

#include "Coord.hpp"
#include "Config.hpp"
#include "AmmoParams.hpp"

namespace miltech::simulation {

constexpr float M_GI = 9.81;

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class IBallisticSolver {
public:
    virtual ~IBallisticSolver() = default;
    virtual Coord solve(const Coord &dronePos, const Coord &targetPos, const Config &config, const AmmoParams &ammo) = 0;
};

}  // namespace miltech::simulation

#endif  // DRONE_SIMULATION_IBALLISTICSOLVER_HPP
