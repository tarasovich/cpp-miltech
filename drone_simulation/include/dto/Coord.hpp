#ifndef DRONE_SIMULATION_COORD_HPP
#define DRONE_SIMULATION_COORD_HPP

// ============================================================
// Coord
// ============================================================
// Координата у 2D (або 3D для дрона). Зберігає x, y (та опціонально z).
// Ця структура має перевантажені оператори

namespace miltech::simulation {

struct Coord {
    float x;
    float y;

    // =========================================================
    // Структура Coord повинна підтримувати арифметичні операції
    // =========================================================

    // Додавання координат
    Coord operator+(const Coord &other) const
    {
        Coord result{};
        result.x = x + other.x;
        result.y = y + other.y;
        return result;
    }

    // Віднімання координат
    Coord operator-(const Coord &other) const
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

}  // namespace miltech::simulation

#endif  // DRONE_SIMULATION_COORD_HPP
