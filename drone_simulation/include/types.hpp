#pragma once

#include <string>
#include <iostream>
#include <memory>
#include <vector>

namespace miltech::simulation {

// class IDroneState;
enum class DroneState : uint8_t;

// ============================================================
// Параметри боєприпасу
// ============================================================
struct AmmoParams {
    std::string name;  // name
    float mass;        // маса (кг)
    float drag;        // коефіцієнт опору
    float lift;        // коефіцієнт підйому
};

// ============================================================
// Coord
// ============================================================
// Координата у 2D (або 3D для дрона). Зберігає x, y (та опціонально z).
// Ця структура має перевантажені оператори
struct Coord {
    float x;
    float y;

    // =========================================================
    // Структура Coord повинна підтримувати арифметичні операції
    // =========================================================

    // Додавання координат
    Coord operator+(const Coord& other) const
    {
        Coord result{};
        result.x = x + other.x;
        result.y = y + other.y;
        return result;
    }

    // Віднімання координат
    Coord operator-(const Coord& other) const
    {
        Coord result{};
        result.x = x - other.x;
        result.y = y - other.y;
        return result;
    }

    // Множення на скаляр
    Coord operator*(const float s) const
    {
        Coord result{};
        result.x = x * s;
        result.y = y * s;
        return result;
    }

    // Віднімання скаляр
    Coord operator-(const float s) const
    {
        Coord result{};
        result.x = x - s;
        result.y = y - s;
        return result;
    }

    // Ділення скаляр
    Coord operator/(const float s) const
    {
        Coord result{};
        result.x = x / s;
        result.y = y / s;
        return result;
    }
};

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

// ============================================================
// Визначення масиву балістичних даних
// ============================================================
struct BallisticSolution {
    float fTime;
    float hDist;
    float tgtDist;
    Coord fireCoords;
    float fireDist;
};

// ============================================================
// Ціль і її позиції
// ============================================================
struct Target {
    std::vector<Coord> positions;
};

// ============================================================
// Спільні дані
// ============================================================
struct DroneContext {
    float direction;                   // поточний напрямок (рад)
    float desiredDir;                  // обраний напрямок
    float speed;                       // поточна швидкість
    std::unique_ptr<Config> cfg;       // Конфіг
    std::unique_ptr<AmmoParams> ammo;  // Параметри боєприпасу
    // std::unique_ptr<IDroneState> state;  // стан автомата (STOPPED, ACCELERATING, DECELERATING, TURNING, MOVING)
    std::unique_ptr<DroneState> state;  // стан автомата (STOPPED, ACCELERATING, DECELERATING, TURNING, MOVING)
    float accel;                        // прискорення дрону
    float stepTurn;                     // кут повороту за час симуляції
    float hitRadius;                    // радіус ураження
};

// ============================================================
// Output Helpers
// ============================================================

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
