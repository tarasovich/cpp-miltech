#pragma once

#include "AnalyticalSolver.hpp"
#include <memory>

namespace miltech::simulation {

class IBallisticSolver;

enum class SolverType : std::uint8_t { ANALYTICAL, TABLE };

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class BallisticSolverFactory {
public:
    ~BallisticSolverFactory() = default;

    static std::unique_ptr<IBallisticSolver> create(const SolverType& solverType)
    {
        switch (solverType) {
            case SolverType::ANALYTICAL:
                return std::make_unique<AnalyticalSolver>();
            // case SolverType::TABLE:
            //     return new TableSolver();
            default:
                throw std::invalid_argument("BallisticSolverFactory::create(): Unsupported solver type");
        }
    }
};

}  // namespace miltech::simulation
