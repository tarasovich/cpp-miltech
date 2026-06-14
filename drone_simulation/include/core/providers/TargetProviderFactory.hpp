#pragma once

#include "JsonTargetProvider.hpp"
#include <cstdint>
#include <memory>

namespace miltech::simulation {

class ITargetProvider;

enum class ProviderType : std::uint8_t { JSON };

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class TargetProviderFactory {
public:
    ~TargetProviderFactory() = default;

    static std::unique_ptr<ITargetProvider> create(const ProviderType type, const std::string& jsonPath)
    {
        switch (type) {
            case ProviderType::JSON:
                return std::make_unique<JsonTargetProvider>(jsonPath);
            default:
                throw std::invalid_argument("TargetProviderFactory::create(): Unsupported provider type");
        }
    }
};

}  // namespace miltech::simulation
