#pragma once

#include "AbstractState.hpp"
#include "IDroneState.hpp"
#include <memory>

namespace miltech::simulation {

class StateStopped : public AbstractState {
public:
    std::unique_ptr<IDroneState> execute(DroneContext& ctx) override;
    std::string name() const override { return "Stopped"; }
    uint8_t idx() const override { return 0; }
};

}  // namespace miltech::simulation