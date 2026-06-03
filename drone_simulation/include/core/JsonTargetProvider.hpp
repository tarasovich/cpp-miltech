#pragma once

#include <filesystem>
#include "ITargetProvider.hpp"
#include <iostream>
#include <utility>
#include <cstdint>

namespace miltech::simulation {

namespace fs = std::filesystem;

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class JsonTargetProvider : public ITargetProvider {
public:
    explicit JsonTargetProvider(fs::path jsonPath = "targets.json")
        : jsonPath_(std::move(jsonPath))
    {
        this->load();
    }

    fs::path getJsonPath() const { return jsonPath_; }

    uint8_t getTargetCount() const override { return targetCount_; }

    Target getTarget(const uint8_t index) const override
    {
        if (index >= targetCount_) {
            throw std::out_of_range("JsonTargetProvider::getTarget(): Index out of bounds");
        }

        return targets_[index];
    }

    ~JsonTargetProvider() override
    {
        std::cout << "JsonTargetProvider::destructor\n";

        if (targetCount_ > 0) {
            for (uint8_t i = 0; i < targetCount_; i++) {
                delete[] targets_[i].positions;  // NOLINT(cppcoreguidelines-owning-memory)
            }
            delete[] targets_;
        }

        targets_ = nullptr;
    }

private:
    fs::path jsonPath_;

    uint8_t targetCount_{0};
    Target *targets_ = nullptr;

    bool isLoaded_{false};

    bool load();
};

}  // namespace miltech::simulation
