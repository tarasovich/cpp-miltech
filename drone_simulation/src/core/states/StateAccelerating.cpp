#include "StateAccelerating.hpp"
#include "StateDecelerating.hpp"
#include "StateMoving.hpp"
#include "types.hpp"
#include "helpers.hpp"
#include <cmath>
#include <memory>

namespace miltech::simulation {

std::unique_ptr<IDroneState> StateAccelerating::execute(DroneContext& ctx)
{
    const float delta = normalizeAngle(ctx.desiredDir - ctx.direction);
    if (std::fabs(delta) > ctx.cfg->turnThreshold) {
        std::unique_ptr<IDroneState> state = std::make_unique<StateDecelerating>();
        return transition(state, ctx);
    }

    ctx.direction = ctx.desiredDir;
    ctx.speed += ctx.accel * ctx.cfg->simTimeStep;

    if (ctx.speed >= ctx.cfg->attackSpeed) {
        // досягли attackSpeed
        ctx.speed = ctx.cfg->attackSpeed;
        return std::make_unique<StateMoving>();  // рух зі сталою шв.
    }

    return nullptr;
}

}  // namespace miltech::simulation
