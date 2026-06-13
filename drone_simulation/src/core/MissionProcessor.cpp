#include "Config.hpp"
#include "AmmoParams.hpp"
#include "Target.hpp"
#include "IConfigLoader.hpp"
#include "ITargetProvider.hpp"
#include "IBallisticSolver.hpp"
#include "MissionProcessor.hpp"
#include <cmath>

namespace miltech::simulation {

// ============================================================
// Нормалізація кута
// ============================================================
float normalizeAngle(float angle)
{
    while (angle > M_PI) {
        angle -= 2.0f * M_PI;
    }

    while (angle < -M_PI) {
        angle += 2.0f * M_PI;
    }

    return angle;
}

// ============================================================
// Розрахунок орієнтовного часу прильоту дрона до точки скиду (totalTime)
// також вираховує напрямок до цілі,
// враховує "timeToStop" та час розгону залежно від поточної швидкості дрона
// ============================================================
void calculateDirAndTimeToFire(const BallisticSolution &ballistics,
                               const SimStep &step,
                               const Config &config,
                               const MissionDerivedData &derivedData,
                               float &resultDir,  // NOLINT (bugprone-easily-swappable-parameters)
                               float &resultTime)
{
    // Якщо ціль занадто близько
    bool isTurnAdded{false};

    if (ballistics.tgtDist < ballistics.hDist) {
        // враховуємо час відльоту і розвороту
        resultTime = ((2 * ballistics.hDist) - ballistics.tgtDist) / config.attackSpeed;
        resultTime += static_cast<float>(M_PI) / config.angularSpeed;  // NOLINT (readability-magic-numbers)
        isTurnAdded = true;
    }
    else {
        resultTime = ballistics.fireDist / config.attackSpeed;
    }

    // час польоту боєприпасу
    resultTime += ballistics.fTime;

    const float dx = ballistics.fireCoords.x - step.pos.x;
    const float dy = ballistics.fireCoords.y - step.pos.y;
    resultDir = std::atan2(dy, dx);
    const float deltaAngle = normalizeAngle(resultDir - step.direction);
    // Якщо кут між поточним напрямком і новим напрямком > turnThreshold:
    if (std::fabs(deltaAngle) > config.turnThreshold) {
        // 5. Дрон гальмує (шлях гальмування = accelerationPath)
        if (step.speed > 0.0f) {
            resultTime += step.speed / derivedData.accel;
        }

        // 6. Повертається на місці. Час повороту = |deltaAngle| / angularSpeed
        if (!isTurnAdded) {
            resultTime += std::fabs(deltaAngle) / config.angularSpeed;
        }

        // 7. Розганяється у новому напрямку
        resultTime += config.attackSpeed / derivedData.accel;
    }
    else if (step.speed < config.attackSpeed) {
        // час розгону
        resultTime += (config.attackSpeed - step.speed) / derivedData.accel;
    }
}

void MissionProcessor::doInit(IConfigLoader *&configLoader, ITargetProvider *&targets)
{
    configLoader->load();

    config_ = configLoader->getConfig();
    ammoParams_ = configLoader->getAmmoParams();
    targets_ = targets;

    std::cout << *config_ << '\n';
    std::cout << *ammoParams_ << '\n';
    std::cout << *targets_ << '\n';

    initDerivedData();

    doReset();
}

void MissionProcessor::doReset()
{
    if (steps_ != nullptr) {
        delete[] steps_;
        steps_ = nullptr;
    }

    currentStep_ = 0;
    currentTime_ = 0.0f;

    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    steps_ = new SimStep[maxSteps_];

    // ReSharper disable once CppExpressionWithoutSideEffects
    initStep(currentStep_);
}

void MissionProcessor::initDerivedData()
{
    derivedData_.accel = config_->attackSpeed * config_->attackSpeed / (2.0f * config_->accelPath);
    derivedData_.stepTurn = config_->angularSpeed * config_->simTimeStep;
    derivedData_.hitRadius = config_->hitRadius / 1.5f;
}

SimStep *MissionProcessor::initStep(const uint16_t stepIdx)
{
    if (stepIdx == 0) {
        steps_[stepIdx] = SimStep{};
        steps_[stepIdx].pos = config_->startPos;
        steps_[stepIdx].direction = config_->initialDir;
        steps_[stepIdx].state = DroneState::STOPPED;
        steps_[stepIdx].speed = 0.0f;
        steps_[stepIdx].targetIdx = -1;
    }
    else {
        steps_[stepIdx] = steps_[stepIdx - 1];
    }

    return &steps_[stepIdx];
}

bool MissionProcessor::doHasNext() const
{
    return !isCompleted_ && currentStep_ + 1 < maxSteps_;
}

bool MissionProcessor::doStep()
{
    auto *curStep = &steps_[currentStep_];

    int bestTgtIdx = -1;
    float bestTotalTime{-1.0f};
    float bestDir{0.0f};
    Coord bestTgtPos{};
    Coord bestTgtVelocity{};
    for (uint8_t i = 0; i < targets_->getTargetCount(); ++i) {
        float totalTime{};
        float desiredDir{};

        const bool isAnotherTarget{curStep->targetIdx < 0 || static_cast<uint8_t>(curStep->targetIdx) != i};

        const auto target = targets_->getTarget(i);
        const float simPos = currentTime_ / config_->arrayTimeStep;
        const auto tgtPosIdx = static_cast<uint8_t>(std::floor(simPos)) % target.timeSteps;
        const auto prevPosIdx = static_cast<uint8_t>((tgtPosIdx + target.timeSteps - 1) % target.timeSteps);

        // Вектор швидкості
        const Coord velocity = (target.positions[tgtPosIdx] - target.positions[prevPosIdx]) * (1.0f / config_->arrayTimeStep);
        const float dt = (simPos - std::floor(simPos)) * config_->arrayTimeStep;
        // Поточна позиція цілі
        const Coord targetPos = target.positions[tgtPosIdx] + velocity * dt;

        // Розрахувати орієнтовний час прильоту дрона до точки скиду (totalTime) для поточної позиції цілі
        const auto basicSolution = solver_->solve(curStep->pos, targetPos, *config_, *ammoParams_);
        calculateDirAndTimeToFire(basicSolution, *curStep, *config_, derivedData_, desiredDir, totalTime);

        // Прогнозована позиція цілі на через totalTime
        const Coord predictedTargetPos = targetPos + velocity * totalTime;
        const auto predictedSolution = solver_->solve(curStep->pos, predictedTargetPos, *config_, *ammoParams_);
        calculateDirAndTimeToFire(predictedSolution, *curStep, *config_, derivedData_, desiredDir, totalTime);

        // Штраф за зміну цілі
        if (isAnotherTarget) {
            const float targetSpeed = std::hypot(velocity.x, velocity.y);
            const float turnTime = std::fabs(normalizeAngle(desiredDir - curStep->direction)) / config_->angularSpeed;
            const float switchPenalty = baseTgtSwitchPenalty_ + (targetSpeed * turnTime / config_->hitRadius);
            totalTime += switchPenalty;
        }

        // 10.Обрати ціль з мінімальним загальним часом (з врахуванням timeToStop при зміні цілі)
        // > timeToStop вже врахований у totalTime
        if (bestTotalTime < 0.0f || totalTime < bestTotalTime) {
            bestTgtIdx = i;
            curStep->ballistic = predictedSolution;
            bestDir = desiredDir;
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
    curStep->targetIdx = bestTgtIdx;

    // точка влучання бомби
    curStep->aimPoint.x = curStep->pos.x + (std::cos(curStep->direction) * curStep->ballistic.hDist);  // hDist в нашій ситуації статичний
    curStep->aimPoint.y = curStep->pos.y + (std::sin(curStep->direction) * curStep->ballistic.hDist);

    // позиція цілі на момент прильоту бомби
    curStep->predictedTarget = bestTgtPos + bestTgtVelocity * curStep->ballistic.fTime;  // fTime статичний

    // перевірка точності влучання
    const float dx = curStep->predictedTarget.x - curStep->aimPoint.x;
    const float dy = curStep->predictedTarget.y - curStep->aimPoint.y;

    // Симуляція завершується, коли дрон досягне точки скиду і скине боєприпас.
    if ((dx * dx) + (dy * dy) <= (derivedData_.hitRadius * derivedData_.hitRadius) && curStep->state == DroneState::MOVING) {
        std::cout << "DROPPED on step " << currentStep_ + 1 << '\n';
        isCompleted_ = true;
        return true;
    }

    // Наступний крок
    auto *nextStep = initStep(currentStep_ + 1);

    // 11. Перевірити кут повороту. Якщо > turnThreshold — змінити стан на DECELERATING/TURNING
    const float deltaAngle = normalizeAngle(bestDir - nextStep->direction);
    if (std::fabs(deltaAngle) > config_->turnThreshold) {
        nextStep->state = nextStep->speed > 0.0f ? DroneState::DECELERATING : DroneState::TURNING;
    }
    else {
        // Якщо кут ≤ turnThreshold — дрон змінює напрямок без зупинки.
        nextStep->direction = bestDir;

        // якщо поворот завершено
        if (nextStep->state == DroneState::TURNING) {
            // розганяється у новому напрямку
            nextStep->state = DroneState::ACCELERATING;
        }
    }

    // 12.Оновити координати, швидкість та стан дрона відповідно до поточної фази
    bool isUpdateDroneCoords = false;
    switch (nextStep->state) {
        case DroneState::STOPPED:
            nextStep->state = DroneState::ACCELERATING;  // не стоїмо на місці
            break;
        case DroneState::ACCELERATING:
            // розгін
            nextStep->speed += derivedData_.accel * config_->simTimeStep;

            if (nextStep->speed >= config_->attackSpeed) {
                // досягли attackSpeed
                nextStep->speed = config_->attackSpeed;
                nextStep->state = DroneState::MOVING;  // рух зі сталою шв.
            }

            isUpdateDroneCoords = true;  // оновити координати дрону
            break;
        case DroneState::DECELERATING:
            // гальмування
            nextStep->speed -= derivedData_.accel * config_->simTimeStep;

            if (nextStep->speed <= 0.0f) {
                // зупинились
                nextStep->speed = 0.0f;
                nextStep->state = DroneState::STOPPED;
            }

            isUpdateDroneCoords = true;  // оновити координати дрону
            break;
        case DroneState::TURNING: {
            // поворот на залишок
            const float turn = std::max(-derivedData_.stepTurn, std::min(derivedData_.stepTurn, deltaAngle));
            nextStep->direction = normalizeAngle(nextStep->direction + turn);

            // якщо поворот завершено
            if (std::fabs(normalizeAngle(bestDir - nextStep->direction)) <= 0.0f) {
                nextStep->direction = bestDir;
                nextStep->state = DroneState::ACCELERATING;  // починаємо розгон
            }
            break;
        }
        case DroneState::MOVING:
            isUpdateDroneCoords = true;  // оновити координати дрону
            break;
        default:
            break;
    }

    // Оновлення позиції дрона
    if (isUpdateDroneCoords) {
        nextStep->pos.x = curStep->pos.x + (std::cos(nextStep->direction) * nextStep->speed * config_->simTimeStep);
        nextStep->pos.y = curStep->pos.y + (std::sin(nextStep->direction) * nextStep->speed * config_->simTimeStep);
    }
    else {
        nextStep->pos = curStep->pos;
    }

    ++currentStep_;
    currentTime_ += config_->simTimeStep;
    return true;
}

}  // namespace miltech::simulation
