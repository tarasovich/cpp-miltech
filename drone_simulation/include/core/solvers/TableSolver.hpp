#pragma once

#include <algorithm>

#include "types.hpp"
#include "IBallisticSolver.hpp"

namespace miltech::simulation {

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class TableSolver : public IBallisticSolver {
public:
    ~TableSolver() override = default;
    explicit TableSolver(const std::string &tablePath) { loadTable(tablePath); }

    BallisticSolution solve(const Coord &dronePos, const Coord &targetPos, const DroneContext &ctx) override;

private:
    struct TableResult {
        float t;
        float hDist;
    };

    struct BallisticTable {
        // 5 осей — кожна зі своїм набором вузлів (нерівномірний крок)
        std::vector<float> axisZ0;  // висота
        std::vector<float> axisV0;  // швидкість
        std::vector<float> axisM;   // маса
        std::vector<float> axisD;   // опір
        std::vector<float> axisL;   // підйомна сила

        // Плоский масив розміром |Z0| * |V0| * |M| * |D| * |L|
        std::vector<TableResult> data;
    };

    // Індекс і коефіцієнт для одного виміру
    struct Interp {
        int lo;      // нижній індекс в осі
        float frac;  // коефіцієнт [0..1]
    };

    BallisticTable table_;
    void loadTable(const std::string &tablePath);

    // Індекс у плоскому масиві: [iZ0][iV0][iM][iD][iL]
    size_t index(int iz, int iv, int im, int id, int il) const;
    const TableResult &at(const int iz, const int iv, const int im, const int id, const int il) const
    {
        return table_.data.at(index(iz, iv, im, id, il));
    }

    // Лінійна інтерполяція для Result (обидва поля паралельно)
    static TableResult lerp(const TableResult &a, const TableResult &b, float t);

    // Індекс і коефіцієнт для одного виміру
    static Interp findInterp(float val, const std::vector<float> &axis);

    TableResult lookup(float Z0, float V0, float m, float d, float l) const;
};

}  // namespace miltech::simulation
