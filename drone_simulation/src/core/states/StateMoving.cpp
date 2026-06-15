#include "StateMoving.hpp"
#include "StateDecelerating.hpp"
#include "types.hpp"
#include "helpers.hpp"
#include <cmath>
#include <memory>

namespace miltech::simulation {

std::unique_ptr<IDroneState> StateMoving::execute(DroneContext& ctx)
{
    const float delta = normalizeAngle(ctx.desiredDir - ctx.direction);
    if (std::fabs(delta) > ctx.cfg->turnThreshold) {
        std::unique_ptr<IDroneState> state = std::make_unique<StateDecelerating>();
        return transition(state, ctx);
    }

    ctx.direction = ctx.desiredDir;
    return nullptr;
}

}  // namespace miltech::simulation
