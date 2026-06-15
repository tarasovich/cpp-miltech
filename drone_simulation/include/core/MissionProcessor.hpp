#pragma once

#include "types.hpp"
#include <memory>

namespace miltech::simulation {

class IBallisticSolver;
class IConfigLoader;
class ITargetProvider;

// ============================================================
// SimStep - Один крок симуляції для виведення.
// ============================================================
struct SimStep {
    Coord pos;                    // позиція дрона
    float direction;              // напрямок (рад)
    uint8_t state;                // стан автомата (STOPPED - 0, ACCELERATING - 1, DECELERATING - 2, TURNING - 3, MOVING - 4)
    int targetIdx;                // індекс поточної цілі
    BallisticSolution ballistic;  // балістичне рішення для кращої цілі кроку
    Coord aimPoint;               // куди впаде бомба (якщо скинути зараз)
    Coord predictedTarget;        // прогнозована позиція цілі
    // float time; // час
    // int num; // крок
};

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class MissionProcessor {
public:
    ~MissionProcessor();

    explicit MissionProcessor(uint16_t maxSteps = 0, float baseTgtSwitchPenalty = 0.0f);

    // Завантажити конфіг, підготувати дані для ітерації
    void init(const std::unique_ptr<IConfigLoader> &configLoader, std::unique_ptr<ITargetProvider> &targets)
    {
        if (isInitialized_) {
            throw std::logic_error("MissionProcessor::init(): Mission already initialized");
        }

        if (!configLoader || !targets) {
            throw std::invalid_argument("MissionProcessor::init(): dependency is null");
        }

        this->doInit(configLoader, targets);

        isInitialized_ = true;
    }

    // Перевірити, чи є ще необроблені цілі
    bool hasNext() const
    {
        requireInit();
        return doHasNext();
    }

    // Обробити наступну ціль
    bool step()
    {
        requireInit();
        return doStep();
    }

    // Почати ітерацію спочатку
    void reset()
    {
        requireInit();
        doReset();
    }

    // Підмінити solver на льоту
    void changeSolver(std::unique_ptr<IBallisticSolver> &solver);

    uint16_t getCurrentStep() const { return currentStep_; }
    uint16_t getStepsCount() const { return getCurrentStep() + 1; }

    const std::vector<SimStep> &getSteps() const { return steps_; }

private:
    bool isInitialized_{false};
    uint16_t maxSteps_{0};
    std::vector<SimStep> steps_;
    float currentTime_{0.0f};
    uint16_t currentStep_{0};
    bool isCompleted_{false};

    std::unique_ptr<DroneContext> ctx_ = nullptr;
    std::unique_ptr<ITargetProvider> targets_ = nullptr;
    std::unique_ptr<IBallisticSolver> solver_ = nullptr;

    float baseTgtSwitchPenalty_{0.0f};

    void requireInit() const
    {
        if (!isInitialized_) {
            throw std::logic_error("MissionProcessor: Mission not initialized");
        }

        if (solver_ == nullptr) {
            throw std::logic_error("MissionProcessor: Solver is not set");
        }
    }

    void doInit(const std::unique_ptr<IConfigLoader> &configLoader, std::unique_ptr<ITargetProvider> &targets);
    void doReset();
    bool doHasNext() const;
    bool doStep();

    SimStep &initStep(uint16_t stepIdx);
};

}  // namespace miltech::simulation
