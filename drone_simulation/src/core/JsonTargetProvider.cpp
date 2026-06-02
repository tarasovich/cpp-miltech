// To create better diagnostics messages, each JSON value needs a pointer to its parent value such that a global context (i.e., a path from
// the root value to the value that led to the exception) can be created. They can, however, be enabled by defining the preprocessor symbol
// JSON_DIAGNOSTICS to 1 before including json.hpp.
#define JSON_DIAGNOSTICS 1  // NOLINT (consider using a 'constexpr' constant )
#include "json.hpp"
#include "JsonTargetProvider.hpp"
#include <fstream>

using json = nlohmann::json;

bool JsonTargetProvider::load()
{
    std::ifstream fstream(this->jsonPath);

    if (!fstream.is_open()) {
        throw std::invalid_argument("JsonTargetProvider::load(): Failed to open " + this->jsonPath.string());
    }

    json jc{};
    try {
        fstream >> jc;
    }
    catch (const json::exception &e) {
        throw std::invalid_argument("JsonTargetProvider::load(): Failed to parse " + this->jsonPath.string() + ":\n" + e.what());
    }

    try {
        this->targetCount = jc.at("targetCount");
        const uint8_t timeSteps = jc.at("timeSteps");
        this->targets = new Target *[this->targetCount];  // NOLINT(cppcoreguidelines-owning-memory)
        for (uint8_t i = 0; i < this->targetCount; i++) {
            this->targets[i] = new Target[timeSteps];  // NOLINT(cppcoreguidelines-owning-memory)
            for (uint8_t j = 0; j < timeSteps; j++) {
                this->targets[i][j].pos = Coord{.x = jc.at("targets").at(i).at("positions").at(j).at("x"),
                                                .y = jc.at("targets").at(i).at("positions").at(j).at("y")};
            }
        }
    }
    catch (json::exception &e) {
        throw std::logic_error("JsonTargetProvider::load(): Failed to parse " + this->jsonPath.string() + ":\n" + e.what());
    }

    return true;
}