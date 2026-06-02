#ifndef DRONE_SIMULATION_MISSIONPROCESSOR_HPP
#define DRONE_SIMULATION_MISSIONPROCESSOR_HPP

#include "IBallisticSolver.hpp"
#include "IConfigLoader.hpp"
#include "ITargetProvider.hpp"

namespace miltech::simulation {

// ============================================================
// Стани дрона (enum)
// ============================================================
enum class DroneState : uint8_t {
    STOPPED = 0,
    ACCELERATING = 1,
    DECELERATING = 2,
    TURNING = 3,
    MOVING = 4,
};

// ============================================================
// SimStep - Один крок симуляції для виведення.
// ============================================================
struct SimStep {
    Coord pos;              // позиція дрона
    float direction;        // напрямок (рад)
    DroneState state;       // стан автомата (0-4)
    int targetIdx;          // індекс поточної цілі
    Coord dropPoint;        // точка скиду (куди летить дрон)
    Coord aimPoint;         // куди впаде бомба (якщо скинути зараз)
    Coord predictedTarget;  // прогнозована позиція цілі

    // float time; // час
    // int num; // крок
};

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class MissionProcessor {
public:
    ~MissionProcessor()
    {
        if (steps_ != nullptr) {
            delete[] steps_;
            steps_ = nullptr;
        }
    }

    explicit MissionProcessor(const uint16_t maxSteps = 0)
        : maxSteps_(maxSteps)
    {
        if (maxSteps_ == 0) {
            throw std::invalid_argument("MissionProcessor::MissionProcessor(): maxSteps must be greater than 0");
        }
    }

    // Завантажити конфіг, підготувати дані для ітерації
    void init(IConfigLoader *&configLoader, const ITargetProvider *&targets)
    {
        if (isInitialized_) {
            throw std::logic_error("MissionProcessor::init(): Mission already initialized");
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

    void changeSolver(IBallisticSolver *&solver)  // Підмінити solver на льоту
    {
        solver_ = solver;
    }

    uint16_t getCurrentStep() const { return currentStep_; }

private:
    bool isInitialized_{false};
    uint16_t maxSteps_{0};
    SimStep *steps_ = nullptr;
    float currentTime_{0.0f};
    uint16_t currentStep_{0};

    const Config *config_ = nullptr;
    const AmmoParams *ammoParams_ = nullptr;
    const ITargetProvider *targets_ = nullptr;
    const IBallisticSolver *solver_ = nullptr;

    void requireInit() const
    {
        if (!isInitialized_) {
            throw std::logic_error("MissionProcessor: Mission not initialized");
        }

        if (solver_ == nullptr) {
            throw std::logic_error("MissionProcessor: Solver is not set");
        }
    }

    void doInit(IConfigLoader *&configLoader, const ITargetProvider *&targets);
    void doReset();
    bool doHasNext() const;
    bool doStep();
};

}  // namespace miltech::simulation

#endif  // DRONE_SIMULATION_MISSIONPROCESSOR_HPP
