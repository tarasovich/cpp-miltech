#pragma once

#include <string>
#include <iostream>

// ============================================================
// AmmoParams - Параметри одного типу боєприпасу.
// ============================================================

namespace miltech::simulation {

struct AmmoParams {
    std::string name;  // name
    float mass;        // маса (кг)
    float drag;        // коефіцієнт опору
    float lift;        // коефіцієнт підйому
};

inline std::ostream& operator<<(std::ostream& os, const AmmoParams& ammo)
{
    os << "AmmoParams:\n"
       << "  name: " << ammo.name << "\n"
       << "  mass: " << ammo.mass << "\n"
       << "  drag: " << ammo.drag << "\n"
       << "  lift: " << ammo.lift << "\n";
    return os;
}

}  // namespace miltech::simulation
