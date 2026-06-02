#ifndef DRONE_SIMULATION_BALLISTICSOLVERFACTORY_HPP
#define DRONE_SIMULATION_BALLISTICSOLVERFACTORY_HPP

#include "IBallisticSolver.hpp"
#include "AnalyticalSolver.hpp"
#include <cstdint>

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

#endif  // DRONE_SIMULATION_BALLISTICSOLVERFACTORY_HPP
