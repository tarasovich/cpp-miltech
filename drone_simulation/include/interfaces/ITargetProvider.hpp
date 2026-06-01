#ifndef DRONE_SIMULATION_ITARGETPROVIDER_HPP
#define DRONE_SIMULATION_ITARGETPROVIDER_HPP
#include "Target.hpp"

class ITargetProvider {
public:
    virtual uint8_t getTargetCount() const = 0;
    virtual Target getTarget(uint8_t index) const = 0;
    virtual ~ITargetProvider() = default;
};

inline std::ostream& operator<<(std::ostream& os, const ITargetProvider& targetProvider)
{
    os << "Target provider:\n"
       << "  targetCount: " << static_cast<int>(targetProvider.getTargetCount()) << "\n";

    return os;
}

#endif  // DRONE_SIMULATION_ITARGETPROVIDER_HPP
