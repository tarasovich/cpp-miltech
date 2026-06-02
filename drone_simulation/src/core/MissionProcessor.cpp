#include "MissionProcessor.hpp"

void MissionProcessor::doInit(IConfigLoader*& configLoader, const ITargetProvider*& targets)
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

    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    steps_ = new SimStep[maxSteps_];

    steps_[currentStep_].pos = config_->startPos;
    steps_[currentStep_].direction = config_->initialDir;
    steps_[currentStep_].state = DroneState::STOPPED;
    steps_[currentStep_].targetIdx = -1;
}

bool MissionProcessor::doHasNext() const
{
    return currentStep_ + 1 <= maxSteps_;
}

bool MissionProcessor::doStep()
{
    ++currentStep_;
    return true;
}