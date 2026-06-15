#include "StateTurning.hpp"
#include "StateAccelerating.hpp"
#include "types.hpp"
#include "helpers.hpp"
#include <cmath>
#include <memory>

namespace miltech::simulation {

std::unique_ptr<IDroneState> StateTurning::execute(DroneContext& ctx)
{
    const float delta = normalizeAngle(ctx.desiredDir - ctx.direction);

    if (std::fabs(delta) <= ctx.cfg->turnThreshold) {
        ctx.direction = ctx.desiredDir;

        std::unique_ptr<IDroneState> state = std::make_unique<StateAccelerating>();

        return transition(state, ctx);
    }

    const float turn = std::max(-ctx.stepTurn, std::min(ctx.stepTurn, delta));
    ctx.direction = normalizeAngle(ctx.direction + turn);

    if (std::fabs(normalizeAngle(ctx.desiredDir - ctx.direction)) <= 0.0f) {
        ctx.direction = ctx.desiredDir;
        return std::make_unique<StateAccelerating>();
    }

    return nullptr;
}

}  // namespace miltech::simulation
