#include "types.hpp"
#include "helpers.hpp"
#include "IConfigLoader.hpp"
#include "ITargetProvider.hpp"
#include "IBallisticSolver.hpp"
#include "MissionProcessor.hpp"

#include "StateMoving.hpp"
#include "StateStopped.hpp"

#include <cmath>

namespace miltech::simulation {

// ============================================================
// Розрахунок орієнтовного часу прильоту дрона до точки скиду (totalTime)
// також вираховує напрямок до цілі,
// враховує "timeToStop" та час розгону залежно від поточної швидкості дрона
// ============================================================
void calculateDirAndTimeToFire(const BallisticSolution &ballistics,
                               const SimStep &step,
                               const DroneContext &ctx,
                               float &resultDir,  // NOLINT (bugprone-easily-swappable-parameters)
                               float &resultTime)
{
    // Якщо ціль занадто близько
    bool isTurnAdded{false};

    if (ballistics.tgtDist < ballistics.hDist) {
        // враховуємо час відльоту і розвороту
        resultTime = ((2 * ballistics.hDist) - ballistics.tgtDist) / ctx.cfg->attackSpeed;
        resultTime += static_cast<float>(M_PI) / ctx.cfg->angularSpeed;  // NOLINT (readability-magic-numbers)
        isTurnAdded = true;
    }
    else {
        resultTime = ballistics.fireDist / ctx.cfg->attackSpeed;
    }

    // час польоту боєприпасу
    resultTime += ballistics.fTime;

    const float dx = ballistics.fireCoords.x - step.pos.x;
    const float dy = ballistics.fireCoords.y - step.pos.y;
    resultDir = std::atan2(dy, dx);
    const float deltaAngle = normalizeAngle(resultDir - ctx.direction);
    // Якщо кут між поточним напрямком і новим напрямком > turnThreshold:
    if (std::fabs(deltaAngle) > ctx.cfg->turnThreshold) {
        // 5. Дрон гальмує (шлях гальмування = accelerationPath)
        if (ctx.speed > 0.0f) {
            resultTime += ctx.speed / ctx.accel;
        }

        // 6. Повертається на місці. Час повороту = |deltaAngle| / angularSpeed
        if (!isTurnAdded) {
            resultTime += std::fabs(deltaAngle) / ctx.cfg->angularSpeed;
        }

        // 7. Розганяється у новому напрямку
        resultTime += ctx.cfg->attackSpeed / ctx.accel;
    }
    else if (ctx.speed < ctx.cfg->attackSpeed) {
        // час розгону
        resultTime += (ctx.cfg->attackSpeed - ctx.speed) / ctx.accel;
    }
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
MissionProcessor::MissionProcessor(const uint16_t maxSteps, const float baseTgtSwitchPenalty)
    : maxSteps_(maxSteps)
    , baseTgtSwitchPenalty_(baseTgtSwitchPenalty)
{
    if (maxSteps_ == 0) {
        throw std::invalid_argument("MissionProcessor::MissionProcessor(): maxSteps must be greater than 0");
    }
}

void MissionProcessor::doInit(const std::unique_ptr<IConfigLoader> &configLoader, std::unique_ptr<ITargetProvider> &targets)
{
    configLoader->load();

    auto cfg = configLoader->getConfig();
    auto ammo = configLoader->getAmmoParams();
    ctx_ = std::make_unique<DroneContext>(cfg->initialDir,
                                          0.0f,
                                          0.0f,
                                          std::move(cfg),
                                          std::move(ammo),
                                          std::make_unique<StateStopped>(),
                                          cfg->attackSpeed * cfg->attackSpeed / (2.0f * cfg->accelPath),
                                          cfg->angularSpeed * cfg->simTimeStep,
                                          cfg->hitRadius / 1.5f);
    targets_ = std::move(targets);

    std::cout << *ctx_->cfg << '\n';
    std::cout << *ctx_->ammo << '\n';
    std::cout << *targets_ << '\n';

    doReset();
}

void MissionProcessor::changeSolver(std::unique_ptr<IBallisticSolver> &solver)
{
    if (!solver) {
        throw std::invalid_argument("MissionProcessor::changeSolver(): solver is null");
    }

    solver_ = std::move(solver);
}

void MissionProcessor::doReset()
{
    currentStep_ = 0;
    currentTime_ = 0.0f;

    steps_.clear();
    steps_.reserve(maxSteps_);

    // ReSharper disable once CppExpressionWithoutSideEffects
    initStep(currentStep_);
}

SimStep &MissionProcessor::initStep(const uint16_t stepIdx)
{
    steps_.resize(stepIdx + 1);

    auto &step = steps_.at(stepIdx);

    if (stepIdx == 0) {
        step.pos = ctx_->cfg->startPos;
        step.state = (StateStopped()).idx();
        step.targetIdx = -1;
    }
    else {
        step = steps_.at(stepIdx - 1);
    }

    return step;
}

bool MissionProcessor::doHasNext() const
{
    return !isCompleted_ && currentStep_ + 1 < maxSteps_;
}

bool MissionProcessor::doStep()
{
    auto &curStep = steps_.at(currentStep_);

    int bestTgtIdx = -1;
    float bestTotalTime{-1.0f};
    Coord bestTgtPos{};
    Coord bestTgtVelocity{};
    for (uint8_t i = 0; i < targets_->getTargetCount(); ++i) {
        float totalTime{};
        float desiredDir{};

        const bool isAnotherTarget{curStep.targetIdx < 0 || static_cast<uint8_t>(curStep.targetIdx) != i};

        const auto target = targets_->getTarget(i);
        const float simPos = currentTime_ / ctx_->cfg->arrayTimeStep;
        const auto tgtPosIdx = static_cast<uint8_t>(std::floor(simPos)) % target.positions.size();
        const auto prevPosIdx = static_cast<uint8_t>((tgtPosIdx + target.positions.size() - 1) % target.positions.size());

        // Вектор швидкості
        const Coord velocity = (target.positions.at(tgtPosIdx) - target.positions.at(prevPosIdx)) * (1.0f / ctx_->cfg->arrayTimeStep);
        const float dt = (simPos - std::floor(simPos)) * ctx_->cfg->arrayTimeStep;
        // Поточна позиція цілі
        const Coord targetPos = target.positions.at(tgtPosIdx) + velocity * dt;

        // Розрахувати орієнтовний час прильоту дрона до точки скиду (totalTime) для поточної позиції цілі
        const auto basicSolution = solver_->solve(curStep.pos, targetPos, *ctx_);
        calculateDirAndTimeToFire(basicSolution, curStep, *ctx_, desiredDir, totalTime);

        // Прогнозована позиція цілі на через totalTime
        const Coord predictedTargetPos = targetPos + velocity * totalTime;
        const auto predictedSolution = solver_->solve(curStep.pos, predictedTargetPos, *ctx_);
        calculateDirAndTimeToFire(predictedSolution, curStep, *ctx_, desiredDir, totalTime);

        // Штраф за зміну цілі
        if (isAnotherTarget) {
            const float targetSpeed = std::hypot(velocity.x, velocity.y);
            const float turnTime = std::fabs(normalizeAngle(desiredDir - ctx_->direction)) / ctx_->cfg->angularSpeed;
            const float switchPenalty = baseTgtSwitchPenalty_ + (targetSpeed * turnTime / ctx_->cfg->hitRadius);
            totalTime += switchPenalty;
        }

        // 10.Обрати ціль з мінімальним загальним часом (з врахуванням timeToStop при зміні цілі)
        // > timeToStop вже врахований у totalTime
        if (bestTotalTime < 0.0f || totalTime < bestTotalTime) {
            bestTgtIdx = i;
            curStep.ballistic = predictedSolution;
            ctx_->desiredDir = desiredDir;
            bestTgtPos = targetPos;
            bestTgtVelocity = velocity;
            bestTotalTime = totalTime;
        }
    }

    // такого бути не повинно
    if (bestTgtIdx < 0) {
        throw std::runtime_error("MissionProcessor::doStep(): No valid target found");
    }

    // оновлюємо ціль поточного кроку
    curStep.targetIdx = bestTgtIdx;

    // точка влучання бомби
    curStep.aimPoint.x = curStep.pos.x + (std::cos(ctx_->direction) * curStep.ballistic.hDist);
    curStep.aimPoint.y = curStep.pos.y + (std::sin(ctx_->direction) * curStep.ballistic.hDist);

    // позиція цілі на момент прильоту бомби
    curStep.predictedTarget = bestTgtPos + bestTgtVelocity * curStep.ballistic.fTime;

    // перевірка точності влучання
    const float dx = curStep.predictedTarget.x - curStep.aimPoint.x;
    const float dy = curStep.predictedTarget.y - curStep.aimPoint.y;

    // Симуляція завершується, коли дрон досягне точки скиду і скине боєприпас.
    if ((dx * dx) + (dy * dy) <= (ctx_->hitRadius * ctx_->hitRadius) && curStep.state == (StateMoving()).idx()) {
        std::cout << "DROPPED on step " << currentStep_ + 1 << '\n';
        isCompleted_ = true;
        return true;
    }

    // Наступний крок
    auto &nextStep = initStep(currentStep_ + 1);

    if (auto nextState = ctx_->state->execute(*ctx_)) {
        ctx_->state = std::move(nextState);
    }

    // Оновлення позиції дрона
    nextStep.direction = ctx_->direction;
    nextStep.state = ctx_->state->idx();

    if (ctx_->speed > 0.0f) {
        nextStep.pos.x = curStep.pos.x + (std::cos(ctx_->direction) * ctx_->speed * ctx_->cfg->simTimeStep);
        nextStep.pos.y = curStep.pos.y + (std::sin(ctx_->direction) * ctx_->speed * ctx_->cfg->simTimeStep);
    }
    else {
        nextStep.pos = curStep.pos;
    }

    ++currentStep_;
    currentTime_ += ctx_->cfg->simTimeStep;

    return true;
}

MissionProcessor::~MissionProcessor() = default;

}  // namespace miltech::simulation
