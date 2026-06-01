#ifndef DRONE_SIMULATION_CONFIG_HPP
#define DRONE_SIMULATION_CONFIG_HPP

#include "Coord.hpp"
#include <string>
#include <iostream>

// ============================================================
// Config - Початкові дані симуляції
// ============================================================
struct Config {
    Coord startPos;        // початкова позиція (x, y)
    float altitude;        // висота
    float initialDir;      // початковий напрямок (рад)
    float attackSpeed;     // швидкість атаки (м/с)
    float accelPath;       // шлях розгону (м)
    std::string ammoName;  // обрані боєприпаси
    float arrayTimeStep;   // крок часу масиву цілей
    float simTimeStep;     // крок симуляції ??
    float hitRadius;       // радіус влучення
    float angularSpeed;    // кутова швидкість (рад/с)
    float turnThreshold;   // поріг повороту (рад)
};

inline std::ostream& operator<<(std::ostream& os, const Config& config)
{
    os << "Config:\n"
       << "  startPos: (" << config.startPos.x << ", " << config.startPos.y << ")\n"
       << "  altitude: " << config.altitude << "\n"
       << "  initialDir: " << config.initialDir << "\n"
       << "  attackSpeed: " << config.attackSpeed << "\n"
       << "  accelPath: " << config.accelPath << "\n"
       << "  ammoName: " << config.ammoName << "\n"
       << "  arrayTimeStep: " << config.arrayTimeStep << "\n"
       << "  simTimeStep: " << config.simTimeStep << "\n"
       << "  hitRadius: " << config.hitRadius << "\n"
       << "  angularSpeed: " << config.angularSpeed << "\n"
       << "  turnThreshold: " << config.turnThreshold << "\n";

    return os;
}

#endif  // DRONE_SIMULATION_CONFIG_HPP
