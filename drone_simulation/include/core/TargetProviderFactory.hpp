#ifndef DRONE_SIMULATION_TARGETPROVIDERFACTORY_HPP
#define DRONE_SIMULATION_TARGETPROVIDERFACTORY_HPP
#include "ITargetProvider.hpp"
#include "JsonTargetProvider.hpp"

enum class ProviderType { JSON };

class TargetProviderFactory {
public:
    ~TargetProviderFactory() = default;

    static ITargetProvider* create(const ProviderType type, const std::string &jsonPath)
    {
        switch (type) {
            case ProviderType::JSON:
                return new JsonTargetProvider(jsonPath);
            default:
                throw std::invalid_argument("TargetProviderFactory::create(): Unsupported provider type");
        }
    }

};

#endif  // DRONE_SIMULATION_TARGETPROVIDERFACTORY_HPP
