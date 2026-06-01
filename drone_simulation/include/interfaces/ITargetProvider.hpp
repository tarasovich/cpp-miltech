#ifndef DRONE_SIMULATION_ITARGETPROVIDER_HPP
#define DRONE_SIMULATION_ITARGETPROVIDER_HPP
#include "Target.hpp"

class ITargetProvider {
  public:
    virtual int getTargetCount() = 0;
    virtual Target getTarget(int index) = 0;
    virtual ~ITargetProvider() = default;
};

#endif  // DRONE_SIMULATION_ITARGETPROVIDER_HPP
