#ifndef DRONE_SIMULATION_TARGETPROVIDERFACTORY_HPP
#define DRONE_SIMULATION_TARGETPROVIDERFACTORY_HPP
#include "ITargetProvider.hpp"
#include "JsonTargetProvider.hpp"

enum class ProviderType : std::uint8_t { JSON };

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class TargetProviderFactory {
public:
    ~TargetProviderFactory() = default;

    static ITargetProvider* create(const ProviderType type, const std::string& jsonPath)
    {
        switch (type) {
            case ProviderType::JSON:
                // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
                return new JsonTargetProvider(jsonPath);
            default:
                throw std::invalid_argument("TargetProviderFactory::create(): Unsupported provider type");
        }
    }
};

#endif  // DRONE_SIMULATION_TARGETPROVIDERFACTORY_HPP
