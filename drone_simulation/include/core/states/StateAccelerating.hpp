#pragma once

#include "AbstractState.hpp"
#include "IDroneState.hpp"
#include <memory>

namespace miltech::simulation {

class StateAccelerating : public AbstractState {
public:
    std::unique_ptr<IDroneState> execute(DroneContext& ctx) override;
    std::string name() const override { return "Accelerating"; }
    uint8_t idx() const override { return 1; }
};

}  // namespace miltech::simulation