#include "MissionProcessor.hpp"
#include <cmath>

namespace miltech::simulation {

void interpolateTarget(const float time, const Config &config, const Coord *targetCoords, const uint8_t tgtCount, Coord &result)
{
    const uint8_t positionIndex = static_cast<uint8_t>(std::floor(time / config.arrayTimeStep)) % tgtCount;
    const uint8_t nextPositionIndex = (positionIndex + 1) % tgtCount;
    const float frac = (time - (static_cast<float>(positionIndex) * config.arrayTimeStep)) / config.arrayTimeStep;

    result = targetCoords[positionIndex] + (targetCoords[nextPositionIndex] - targetCoords[positionIndex]) * frac;
}

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

    const float accel = config.attackSpeed * config.attackSpeed / (2.0f * config.accelPath);

    const float dx = ballistics.fireCoords.x - step.pos.x;
    const float dy = ballistics.fireCoords.y - step.pos.y;
    resultDir = std::atan2(dy, dx);
    const float deltaAngle = normalizeAngle(resultDir - step.direction);
    // Якщо кут між поточним напрямком і новим напрямком > turnThreshold:
    if (std::fabs(deltaAngle) > config.turnThreshold) {
        // 5. Дрон гальмує (шлях гальмування = accelerationPath)
        if (step.speed > 0.0f) {
            resultTime += step.speed / accel;
        }

        // 6. Повертається на місці. Час повороту = |deltaAngle| / angularSpeed
        if (!isTurnAdded) {
            resultTime += std::fabs(deltaAngle) / config.angularSpeed;
        }

        // 7. Розганяється у новому напрямку
        resultTime += config.attackSpeed / accel;
    }
    else if (step.speed < config.attackSpeed) {
        // час розгону
        resultTime += (config.attackSpeed - step.speed) / accel;
    }
}

void MissionProcessor::doInit(IConfigLoader *&configLoader, const ITargetProvider *&targets)
{
    configLoader->load();

    config_ = configLoader->getConfig();
    ammoParams_ = configLoader->getAmmoParams();
    targets_ = targets;

    std::cout << *config_ << '\n';
    std::cout << *ammoParams_ << '\n';
    std::cout << *targets_ << '\n';

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

    steps_[currentStep_].pos = config_->startPos;
    steps_[currentStep_].direction = config_->initialDir;
    steps_[currentStep_].state = DroneState::STOPPED;
    steps_[currentStep_].targetIdx = -1;
}

bool MissionProcessor::doHasNext() const
{
    return !isCompleted_ && currentStep_ + 1 < maxSteps_;
}

bool MissionProcessor::doStep()
{
    auto *curStep = &steps_[currentStep_];

    float bestTotalTime{-1.0f};
    float bestDir{0.0f};
    Coord bestTgtPos{};
    Coord bestTgtVelocity{};
    for (uint8_t i = 0; i < targets_->getTargetCount(); ++i) {
        float totalTime{};
        float desiredDir{};

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
        calculateDirAndTimeToFire(basicSolution, *curStep, *config_, desiredDir, totalTime);

        // Прогнозована позиція цілі на через totalTime
        const Coord predictedTargetPos = targetPos + velocity * totalTime;
        const auto predictedSolution = solver_->solve(curStep->pos, predictedTargetPos, *config_, *ammoParams_);
        calculateDirAndTimeToFire(predictedSolution, *curStep, *config_, desiredDir, totalTime);

        // 10.Обрати ціль з мінімальним загальним часом (з врахуванням timeToStop при зміні цілі)
        // > timeToStop вже врахований у totalTime
        if (bestTotalTime < 0.0f || totalTime < bestTotalTime) {
            curStep->targetIdx = i;
            curStep->ballistic = predictedSolution;
            bestDir = desiredDir;
            bestTgtPos = targetPos;
            bestTgtVelocity = velocity;
            bestTotalTime = totalTime;
        }
    }

    // такого бути не повинно
    if (curStep->targetIdx < 0) {
        throw std::runtime_error("MissionProcessor::doStep(): No valid target found");
    }

    // Симуляція завершується, коли дрон досягне точки скиду (hitRadius) і скине боєприпас.
    const float hitRadius = config_->hitRadius / 2;

    // точка влучання бомби
    curStep->aimPoint.x = curStep->pos.x + (std::cos(curStep->direction) * curStep->ballistic.hDist);
    curStep->aimPoint.y = curStep->pos.y + (std::sin(curStep->direction) * curStep->ballistic.hDist);

    // позиція цілі на момент прильоту бомби
    curStep->predictedTarget = bestTgtPos + bestTgtVelocity * curStep->ballistic.fTime;

    // перевірка точності влучання
    const float dx = curStep->predictedTarget.x - curStep->aimPoint.x;
    const float dy = curStep->predictedTarget.y - curStep->aimPoint.y;
    if ((dx * dx) + (dy * dy) <= (hitRadius * hitRadius) && curStep->state == DroneState::MOVING) {
        std::cout << "DROPPED on step " << currentStep_ + 1 << '\n';
        isCompleted_ = true;
        return true;
    }

    // Наступний крок
    auto *nextStep = &steps_[currentStep_ + 1];
    nextStep->targetIdx = curStep->targetIdx;
    nextStep->state = curStep->state;
    nextStep->direction = curStep->direction;
    nextStep->speed = curStep->speed;

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

    const float accel = config_->attackSpeed * config_->attackSpeed / (2.0f * config_->accelPath);
    const float stepTurn = config_->angularSpeed * config_->simTimeStep;

    // 12.Оновити координати, швидкість та стан дрона відповідно до поточної фази
    bool isUpdateDroneCoords = false;
    switch (nextStep->state) {
        case DroneState::STOPPED:
            nextStep->state = DroneState::ACCELERATING;  // не стоїмо на місці
            break;
        case DroneState::ACCELERATING:
            // розгін
            nextStep->speed += accel * config_->simTimeStep;

            if (nextStep->speed >= config_->attackSpeed) {
                // досягли attackSpeed
                nextStep->speed = config_->attackSpeed;
                nextStep->state = DroneState::MOVING;  // рух зі сталою шв.
            }

            isUpdateDroneCoords = true;  // оновити координати дрону
            break;
        case DroneState::DECELERATING:
            // гальмування
            nextStep->speed -= accel * config_->simTimeStep;

            if (nextStep->speed <= 0.0f) {
                // зупинились
                nextStep->speed = 0.0f;
                nextStep->state = DroneState::STOPPED;
            }

            isUpdateDroneCoords = true;  // оновити координати дрону
            break;
        case DroneState::TURNING: {
            // поворот на залишок
            const float turn = std::max(-stepTurn, std::min(stepTurn, deltaAngle));
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
