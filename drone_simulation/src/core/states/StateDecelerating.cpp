#include "StateDecelerating.hpp"

#include "StateStopped.hpp"
#include "types.hpp"
#include "helpers.hpp"
#include <cmath>
#include <memory>

namespace miltech::simulation {

std::unique_ptr<IDroneState> StateDecelerating::execute(DroneContext& ctx)
{
    const float delta = normalizeAngle(ctx.desiredDir - ctx.direction);
    if (std::fabs(delta) <= ctx.cfg->turnThreshold) {
        ctx.direction = ctx.desiredDir;
    }

    // гальмування
    ctx.speed -= ctx.accel * ctx.cfg->simTimeStep;
    if (ctx.speed <= 0.0f) {
        // зупинились
        ctx.speed = 0.0f;
        return std::make_unique<StateStopped>();
    }

    return nullptr;
}

}  // namespace miltech::simulation
