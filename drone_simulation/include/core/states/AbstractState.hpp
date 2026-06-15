#pragma once

#include "IDroneState.hpp"

namespace miltech::simulation {

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class AbstractState : public IDroneState {
public:
    ~AbstractState() override = default;

protected:
    static std::unique_ptr<IDroneState> transition(std::unique_ptr<IDroneState>& state, DroneContext& ctx)
    {
        if (auto nextState = state->execute(ctx)) {
            return nextState;
        }

        return std::move(state);
    }
};

}  // namespace miltech::simulation
