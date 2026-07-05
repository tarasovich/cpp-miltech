#pragma once

#include "AbstractState.hpp"
#include "IDroneState.hpp"
#include <memory>

namespace miltech::simulation {

class StateDecelerating : public AbstractState {
public:
    std::unique_ptr<IDroneState> execute(DroneContext& ctx) override;
    std::string name() const override { return "Decelerating"; }
    uint8_t idx() const override { return 2; }
};

}  // namespace miltech::simulation