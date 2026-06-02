#ifndef DRONE_SIMULATION_JSONTARGETPROVIDER_HPP
#define DRONE_SIMULATION_JSONTARGETPROVIDER_HPP
#include <filesystem>
#include "ITargetProvider.hpp"
#include <iostream>
#include <utility>

namespace fs = std::filesystem;

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class JsonTargetProvider : public ITargetProvider {
public:
    explicit JsonTargetProvider(fs::path jsonPath = "targets.json")
        : jsonPath(std::move(jsonPath))
    {
        this->load();
    }

    fs::path getJsonPath() const { return jsonPath; }

    uint8_t getTargetCount() const override { return targetCount; }

    Target getTarget(const uint8_t index) const override {
        if (index >= targetCount) {
            throw std::out_of_range("JsonTargetProvider::getTarget(): Index out of bounds");
        }

        return *targets[index];
    }

    ~JsonTargetProvider() override {
        std::cout << "JsonTargetProvider destroy\n";

        if (targetCount > 0) {
            for (uint8_t i = 0; i < targetCount; i++) {
                delete[] targets[i];
            }
            delete[] targets;
        }

        targets = nullptr;
    }

private:
    fs::path jsonPath;

    uint8_t targetCount = 0;
    Target **targets = nullptr;

    bool isLoaded = false;

    bool load();
};

#endif  // DRONE_SIMULATION_JSONTARGETPROVIDER_HPP
