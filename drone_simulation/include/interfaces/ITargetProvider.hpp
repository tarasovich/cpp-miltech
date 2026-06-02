#ifndef DRONE_SIMULATION_ITARGETPROVIDER_HPP
#define DRONE_SIMULATION_ITARGETPROVIDER_HPP
#include "Target.hpp"
#include <iostream>
#include <cstdint>

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class ITargetProvider {
public:
    [[nodiscard]] virtual uint8_t getTargetCount() const = 0;
    [[nodiscard]] virtual Target getTarget(uint8_t index) const = 0;
    virtual ~ITargetProvider() = default;
};

inline std::ostream& operator<<(std::ostream& os, const ITargetProvider& targetProvider)
{
    os << "Target provider:\n"
       << "  targetCount: " << static_cast<unsigned int>(targetProvider.getTargetCount()) << "\n";

    return os;
}

#endif  // DRONE_SIMULATION_ITARGETPROVIDER_HPP
