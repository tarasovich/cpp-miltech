#pragma once

#include "AbstractState.hpp"
#include "IDroneState.hpp"
#include <memory>

namespace miltech::simulation {

class StateMoving : public AbstractState {
public:
    std::unique_ptr<IDroneState> execute(DroneContext& ctx) override;
    std::string name() const override { return "Moving"; }
    uint8_t idx() const override { return 4; }
};

}  // namespace miltech::simulation