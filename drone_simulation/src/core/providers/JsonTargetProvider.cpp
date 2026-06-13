// To create better diagnostics messages, each JSON value needs a pointer to its parent value such that a global context (i.e., a path from
// the root value to the value that led to the exception) can be created. They can, however, be enabled by defining the preprocessor symbol
// JSON_DIAGNOSTICS to 1 before including json.hpp.
#define JSON_DIAGNOSTICS 1  // NOLINT (consider using a 'constexpr' constant )
#include "json.hpp"
#include "Target.hpp"
#include "JsonTargetProvider.hpp"
#include <fstream>

using json = nlohmann::json;

namespace miltech::simulation {

bool JsonTargetProvider::load()
{
    std::ifstream fstream(this->jsonPath_);

    if (!fstream.is_open()) {
        throw std::invalid_argument("JsonTargetProvider::load(): Failed to open " + this->jsonPath_.string());
    }

    json jc{};
    try {
        fstream >> jc;
    }
    catch (const json::exception &e) {
        throw std::invalid_argument("JsonTargetProvider::load(): Failed to parse " + this->jsonPath_.string() + ":\n" + e.what());
    }

    try {
        const auto targetCount = jc.at("targetCount").get<std::size_t>();
        const auto timeSteps = jc.at("timeSteps").get<std::size_t>();

        targets_.resize(targetCount);

        for (size_t i = 0; i < targetCount; i++) {
            targets_.at(i).positions.resize(timeSteps);

            for (size_t j = 0; j < timeSteps; j++) {
                targets_.at(i).positions.at(j) = Coord{.x = jc.at("targets").at(i).at("positions").at(j).at("x"),
                                                       .y = jc.at("targets").at(i).at("positions").at(j).at("y")};
            }
        }
    }
    catch (json::exception &e) {
        throw std::logic_error("JsonTargetProvider::load(): Failed to parse " + this->jsonPath_.string() + ":\n" + e.what());
    }

    return true;
}

}  // namespace miltech::simulation
