#include "StateStopped.hpp"
#include "StateAccelerating.hpp"
#include "StateTurning.hpp"
#include "types.hpp"
#include "helpers.hpp"
#include <cmath>
#include <memory>

namespace miltech::simulation {

std::unique_ptr<IDroneState> StateStopped::execute(DroneContext& ctx)
{
    ctx.speed = 0.0f;

    const float delta = normalizeAngle(ctx.desiredDir - ctx.direction);
    if (std::fabs(delta) > ctx.cfg->turnThreshold) {
        std::unique_ptr<IDroneState> state = std::make_unique<StateTurning>();

        return transition(state, ctx);
    }

    ctx.direction = ctx.desiredDir;
    return std::make_unique<StateAccelerating>();
}

}  // namespace miltech::simulation
