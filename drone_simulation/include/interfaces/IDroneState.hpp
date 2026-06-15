#pragma once

#include <memory>

namespace miltech::simulation {

struct DroneContext;

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class IDroneState {
public:
    virtual ~IDroneState() = default;

    // Виконати логіку стану, повернути наступний стан.
    // Якщо стан не змінився — повернути nullptr
    // (головний цикл залишить поточний).
    virtual std::unique_ptr<IDroneState> execute(DroneContext& ctx) = 0;

    virtual std::string name() const = 0;
    virtual uint8_t idx() const = 0;
};

}  // namespace miltech::simulation