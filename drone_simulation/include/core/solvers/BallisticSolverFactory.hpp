#pragma once

#include "AnalyticalSolver.hpp"
#include "TableSolver.hpp"
#include <memory>

namespace miltech::simulation {

class IBallisticSolver;

enum class SolverType : std::uint8_t { ANALYTICAL, TABLE };

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class BallisticSolverFactory {
public:
    ~BallisticSolverFactory() = default;

    static std::unique_ptr<IBallisticSolver> create(const SolverType& solverType, const std::string& dataSource = "")
    {
        switch (solverType) {
            case SolverType::ANALYTICAL:
                return std::make_unique<AnalyticalSolver>();
            case SolverType::TABLE:
                return std::make_unique<TableSolver>(dataSource);
            default:
                throw std::invalid_argument("BallisticSolverFactory::create(): Unsupported solver type");
        }
    }
};

inline std::ostream& operator<<(std::ostream& os, const SolverType type)
{
    switch (type) {
        case SolverType::ANALYTICAL:
            return os << "analytical";

        case SolverType::TABLE:
            return os << "table";
    }

    return os << "unknown";
}

}  // namespace miltech::simulation
