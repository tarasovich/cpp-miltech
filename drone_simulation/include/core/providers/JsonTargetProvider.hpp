#pragma once

#include "types.hpp"
#include "ITargetProvider.hpp"
#include <cstdint>
#include <filesystem>

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

    uint8_t getTargetCount() const override { return targets_.size(); }

    Target getTarget(const uint8_t index) const override { return targets_.at(index); }

    ~JsonTargetProvider() override = default;

private:
    fs::path jsonPath_;

    std::vector<Target> targets_;

    bool isLoaded_{false};

    bool load();
};

}  // namespace miltech::simulation
