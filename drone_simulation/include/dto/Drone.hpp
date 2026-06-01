#ifndef DRONE_SIMULATION_DRONE_HPP
#define DRONE_SIMULATION_DRONE_HPP

#include "Coord.hpp"
#include "AmmoParams.hpp"
#include <string>

// ============================================================
// Drone - Параметри та стан дрона
// ============================================================
struct Drone {
    Coord pos;             // початкова позиція (x, y)
    float altitude;        // висота
    float dir;             // початковий напрямок (рад)
    float attackSpeed;     // швидкість атаки (м/с)
    float accelPath;       // шлях розгону (м)
    std::string ammoName;  // обрані боєприпаси
    float hitRadius;       // радіус влучення
    float angularSpeed;    // кутова швидкість (рад/с)
    float turnThreshold;   // поріг повороту (рад)

    float fTime;     // час польоту снаряда
    float hDist;     // дистанція польоту снаряда
    float stepTurn;  // кут повороту за час симуляції

    float accel;     // прискорення
    float curSpeed;  // поточна швидкість
};

#endif  // DRONE_SIMULATION_DRONE_HPP
