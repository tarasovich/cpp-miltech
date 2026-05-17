#pragma once
#include <cstdint>

constexpr uint8_t AMMO_NAME_SIZE = 32;

constexpr float M_GI = 9.81; // прискорення вільного падіння

struct AmmoParams {
    char name[AMMO_NAME_SIZE];
    float mass; // маса (кг)
    float drag; // коефіцієнт опору
    float lift; // коефіцієнт підйому
};

constexpr AmmoParams ammos[5] = {
    {"VOG-17", 0.35f, 0.07f, 0.0f},
    {"M67", 0.6f, 0.1f, 0.0f},
    {"RKG-3", 1.2f, 0.1f, 0.0f},
    {"GLIDING-VOG", 0.45f, 0.1f, 1.0f},
    {"GLIDING-RKG", 1.4f, 0.1f, 1.0f}
};

struct BallisticsInput {
    float droneX; // координата X дрона
    float droneY; // координата Y дрона
    float altitude; // висота дрона над землею
    float targetX; // координата X цілі
    float targetY; // координата Y цілі
    float attackSpeed; // швидкість атаки (м/с)
    float accelerationPath; // шлях розгону (м)
    char ammoName[AMMO_NAME_SIZE]; // назва боєприпасу
};

struct BallisticsResult {
    float fireX;
    float fireY;
};

BallisticsResult calculateBallistics(const BallisticsInput &input);