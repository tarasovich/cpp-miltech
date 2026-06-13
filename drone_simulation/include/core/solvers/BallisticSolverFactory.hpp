#pragma once

#include "AnalyticalSolver.hpp"
#include <cstdint>

namespace miltech::simulation {

class IBallisticSolver;

enum class SolverType : std::uint8_t { ANALYTICAL };

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class BallisticSolverFactory {
public:
    ~BallisticSolverFactory() = default;

    static IBallisticSolver* create(const SolverType& solverType)
    {
        switch (solverType) {
            case SolverType::ANALYTICAL:
                // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
                return new AnalyticalSolver();
            default:
                throw std::invalid_argument("BallisticSolverFactory::create(): Unsupported solver type");
        }
    }
};

}  // namespace miltech::simulation
