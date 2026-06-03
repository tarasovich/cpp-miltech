#pragma once

#include "types.hpp"
#include <iostream>
#include <cstdint>

namespace miltech::simulation {

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

}  // namespace miltech::simulation
