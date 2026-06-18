#include "TableSolver.hpp"
#include <cstddef>
#include <fstream>
#include <cmath>

namespace miltech::simulation {

BallisticSolution TableSolver::solve(const Coord& dronePos, const Coord& targetPos, const DroneContext& ctx)
{
    const float dx = targetPos.x - dronePos.x;
    const float dy = targetPos.y - dronePos.y;

    const float tgtDist = std::sqrt((dx * dx) + (dy * dy));

    const float z0 = ctx.cfg->altitude;
    const float v0 = ctx.cfg->attackSpeed;
    const float m = ctx.ammo->mass;
    const float d = ctx.ammo->drag;
    const float l = ctx.ammo->lift;

    const TableResult result = lookup(z0, v0, m, d, l);

    const float ratio = result.hDist / tgtDist;

    const Coord fireCoords{
        .x = dronePos.x + (dx * ratio),
        .y = dronePos.y + (dy * ratio),
    };

    return BallisticSolution{
        .fTime = result.t,
        .hDist = result.hDist,
        .tgtDist = tgtDist,
        .fireCoords = fireCoords,
        .fireDist = result.hDist,
    };
}

void TableSolver::loadTable(const std::string& tablePath)
{
    std::ifstream f(tablePath);
    if (!f.is_open()) {
        throw std::runtime_error("TableSolver::loadTable(): Cannot open table file");
    };

    // NOLINTNEXTLINE(readability-isolate-declaration)
    int nZ{}, nV{}, nM{}, nD{}, nL{};
    f >> nZ >> nV >> nM >> nD >> nL;

    auto readAxis = [&](auto& axis, const std::size_t size) {
        axis.resize(size);
        for (auto& value : axis) {
            f >> value;
        }
    };

    readAxis(table_.axisZ0, nZ);
    readAxis(table_.axisV0, nV);
    readAxis(table_.axisM, nM);
    readAxis(table_.axisD, nD);
    readAxis(table_.axisL, nL);

    const size_t total = static_cast<size_t>(nZ) * nV * nM * nD * nL;
    table_.data.resize(total);

    // Порядок: Z0 → V0 → m → d → l (зовнішній → внутрішній)
    for (size_t i = 0; i < total; i++) {
        f >> table_.data.at(i).t >> table_.data.at(i).hDist;
    }

    if (!f.good()) {
        throw std::runtime_error("TableSolver::loadTable(): Cannot open table file");
    }
}

size_t TableSolver::index(const int iz, const int iv, const int im, const int id, const int il) const
{
    auto idx = static_cast<size_t>(iz);

    idx = (idx * table_.axisV0.size()) + iv;
    idx = (idx * table_.axisM.size()) + im;
    idx = (idx * table_.axisD.size()) + id;
    idx = (idx * table_.axisL.size()) + il;

    return idx;
}

TableSolver::TableResult TableSolver::lerp(const TableResult& a, const TableResult& b, const float t)
{
    return {.t = a.t + ((b.t - a.t) * t), .hDist = a.hDist + ((b.hDist - a.hDist) * t)};
}

TableSolver::Interp TableSolver::findInterp(const float val, const std::vector<float>& axis)
{
    if (val <= axis.front()) {
        return {.lo = 0, .frac = 0.0f};
    }

    if (val >= axis.back()) {
        return {.lo = static_cast<int>(axis.size()) - 2, .frac = 1.0f};
    }

    // NOLINTNEXTLINE(modernize-use-ranges)
    const auto it = std::lower_bound(axis.begin(), axis.end(), val);
    int i = static_cast<int>(it - axis.begin()) - 1;
    i = std::max(i, 0);

    const float frac = (val - axis.at(i)) / (axis.at(i + 1) - axis.at(i));

    return {.lo = i, .frac = frac};
}

TableSolver::TableResult TableSolver::lookup(const float Z0, const float V0, const float m, const float d, const float l) const
{
    const Interp iz = findInterp(Z0, table_.axisZ0);
    const Interp iv = findInterp(V0, table_.axisV0);
    const Interp im = findInterp(m, table_.axisM);
    const Interp id = findInterp(d, table_.axisD);
    const Interp il = findInterp(l, table_.axisL);

    // 2^5 = 32 вершини гіперкуба
    // Згортаємо: 32 → 16 → 8 → 4 → 2 → 1

    // l: 32 → 16
    TableResult v[16];  // NOLINT (cppcoreguidelines-avoid-c-arrays)
    for (int a = 0; a < 2; a++) {
        for (int b = 0; b < 2; b++) {
            for (int c = 0; c < 2; c++) {
                for (int e = 0; e < 2; e++) {
                    const auto& lo = at(iz.lo + a, iv.lo + b, im.lo + c, id.lo + e, il.lo);
                    const auto& hi = at(iz.lo + a, iv.lo + b, im.lo + c, id.lo + e, il.lo + 1);
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
                    v[(a * 8) + (b * 4) + (c * 2) + e] = lerp(lo, hi, il.frac);
                }
            }
        }
    }

    // d: 16 → 8
    TableResult w[8];  // NOLINT (cppcoreguidelines-avoid-c-arrays)
    for (int a = 0; a < 2; a++) {
        for (int b = 0; b < 2; b++) {
            for (int c = 0; c < 2; c++) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
                w[(a * 4) + (b * 2) + c] = lerp(v[(a * 8) + (b * 4) + (c * 2)], v[(a * 8) + (b * 4) + (c * 2) + 1], id.frac);
            }
        }
    }

    // m: 8 → 4
    TableResult u[4];  // NOLINT (cppcoreguidelines-avoid-c-arrays)
    for (int a = 0; a < 2; a++) {
        for (int b = 0; b < 2; b++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            u[(a * 2) + b] = lerp(w[(a * 4) + (b * 2)], w[(a * 4) + (b * 2) + 1], im.frac);
        }
    }

    // V0: 4 → 2
    TableResult s[2];  // NOLINT (cppcoreguidelines-avoid-c-arrays)
    for (int a = 0; a < 2; a++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        s[a] = lerp(u[static_cast<ptrdiff_t>(a * 2)], u[(a * 2) + 1], iv.frac);
    }

    // Z0: 2 → 1
    return lerp(s[0], s[1], iz.frac);
}

}  // namespace miltech::simulation
