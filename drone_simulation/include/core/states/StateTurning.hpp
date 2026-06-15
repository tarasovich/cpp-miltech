#pragma once

#include "AbstractState.hpp"
#include <memory>

namespace miltech::simulation {

class StateTurning : public AbstractState {
public:
    std::unique_ptr<IDroneState> execute(DroneContext& ctx) override;
    std::string name() const override { return "Turning"; }
    uint8_t idx() const override { return 3; }
};

}  // namespace miltech::simulation
