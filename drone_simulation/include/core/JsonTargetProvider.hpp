#ifndef DRONE_SIMULATION_JSONTARGETPROVIDER_HPP
#define DRONE_SIMULATION_JSONTARGETPROVIDER_HPP
#include <filesystem>
#include "ITargetProvider.hpp"
#include <iostream>

namespace fs = std::filesystem;

class JsonTargetProvider : public ITargetProvider {
public:
    explicit JsonTargetProvider(const fs::path &jsonPath = "targets.json")
        : jsonPath(jsonPath)
    {
        this->load();
    }

    fs::path getJsonPath() const { return jsonPath; }

    uint8_t getTargetCount() const override { return targetCount; }

    Target getTarget(const uint8_t index) const override {
        if (index < 0 || index >= targetCount) {
            throw std::out_of_range("Index out of bounds");
        }

        return *targets[index];
    }

    ~JsonTargetProvider() override {;
        if (targets) {
            for (int i = 0; i < targetCount; i++) {
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
