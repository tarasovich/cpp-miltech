#pragma once
#include <cstdint>

constexpr uint8_t AMMO_NAME_SIZE = 32;

constexpr float M_GI = 9.81;  // прискорення вільного падіння

struct AmmoParams {
    // NOLINTNEXTLINE
    char name[AMMO_NAME_SIZE];
    float mass;  // маса (кг)
    float drag;  // коефіцієнт опору
    float lift;  // коефіцієнт підйому
};

// NOLINTNEXTLINE
constexpr AmmoParams ammos[5] = {{.name="VOG-17", .mass=0.35f, .drag=0.07f, .lift=0.0f},
                                 {.name="M67", .mass=0.6f, .drag=0.1f, .lift=0.0f},
                                 {.name="RKG-3", .mass=1.2f, .drag=0.1f, .lift=0.0f},
                                 {.name="GLIDING-VOG", .mass=0.45f, .drag=0.1f, .lift=1.0f},
                                 {.name="GLIDING-RKG", .mass=1.4f, .drag=0.1f, .lift=1.0f}};

struct BallisticsInput {
    float droneX;                   // координата X дрона
    float droneY;                   // координата Y дрона
    float altitude;                 // висота дрона над землею
    float targetX;                  // координата X цілі
    float targetY;                  // координата Y цілі
    float attackSpeed;              // швидкість атаки (м/с)
    float accelerationPath;         // шлях розгону (м)
    // NOLINTNEXTLINE
    char ammoName[AMMO_NAME_SIZE];  // назва боєприпасу
};

struct BallisticsResult {
    float fireX;
    float fireY;
};

BallisticsResult calculateBallistics(const BallisticsInput &input);