#ifndef DRONE_SIMULATION_IBALLISTICSOLVER_HPP
#define DRONE_SIMULATION_IBALLISTICSOLVER_HPP
#include "Target.hpp"
#include "Drone.hpp"

class IBallisticSolver {
  public:
    virtual float solve(Drone drone, Target target) = 0;
    virtual ~IBallisticSolver() = default;
};

#endif  // DRONE_SIMULATION_IBALLISTICSOLVER_HPP
