// To create better diagnostics messages, each JSON value needs a pointer to its parent value such that a global context (i.e., a path from
// the root value to the value that led to the exception) can be created. They can, however, be enabled by defining the preprocessor symbol
// JSON_DIAGNOSTICS to 1 before including json.hpp.
#define JSON_DIAGNOSTICS 1  // NOLINT (consider using a 'constexpr' constant )
#include "json.hpp"
#include "JsonConfigLoader.hpp"
#include <fstream>

using json = nlohmann::json;

void from_json(const json &j, Config &config)
{
    j.at("drone").at("position").at("x").get_to(config.startPos.x);
    j.at("drone").at("position").at("y").get_to(config.startPos.y);
    j.at("drone").at("altitude").get_to(config.altitude);
    j.at("drone").at("initialDirection").get_to(config.initialDir);
    j.at("drone").at("attackSpeed").get_to(config.attackSpeed);
    j.at("drone").at("accelerationPath").get_to(config.accelPath);
    j.at("drone").at("angularSpeed").get_to(config.angularSpeed);
    j.at("drone").at("turnThreshold").get_to(config.turnThreshold);
    j.at("simulation").at("timeStep").get_to(config.simTimeStep);
    j.at("simulation").at("hitRadius").get_to(config.hitRadius);
    j.at("targetArrayTimeStep").get_to(config.arrayTimeStep);
    j.at("ammo").get_to(config.ammoName);
}

void from_json(const json &j, AmmoParams &ammo)
{
    j.at("name").get_to(ammo.name);
    j.at("mass").get_to(ammo.mass);
    j.at("drag").get_to(ammo.drag);
    j.at("lift").get_to(ammo.lift);
}

bool JsonConfigLoader::parse()
{
    // ------------------------
    // Parse main config.json
    // ------------------------
    const fs::path &mainPath = this->getMainPath();
    auto *resultConfig = initResultConfig();

    try {
        std::ifstream fstream(mainPath);

        if (!fstream.is_open()) {
            throw std::invalid_argument("JsonConfigLoader::parse(): Failed to open " + mainPath.string());
        }

        json jc{};
        fstream >> jc;
        jc.get_to(*resultConfig);
    }
    catch (const json::exception &e) {
        throw std::invalid_argument("JsonConfigLoader::parse(): Failed to parse " + mainPath.string() + ":\n" + e.what());
    }

    // ------------------------
    // Parse ammo.json
    // ------------------------
    const fs::path &ammoPath = this->getAmmoPath();
    json jc{};

    try {
        std::ifstream fstream(ammoPath);

        if (!fstream.is_open()) {
            throw std::invalid_argument("Could not open file");
        }

        fstream >> jc;
    }
    catch (const std::exception &e) {
        throw std::invalid_argument("JsonConfigLoader::parse(): Failed to parse " + ammoPath.string() + ":\n" + e.what());
    }

    const uint8_t ammoCount{static_cast<uint8_t>(jc.size())};
    if (ammoCount < 1) {
        throw std::invalid_argument("JsonConfigLoader::parse(): No ammos in " + ammoPath.string());
    }

    auto *resultAmmo = this->initResultAmmoParams();
    bool isAmmoFound{false};

    try {
        for (uint8_t i = 0; i < ammoCount; i++) {
            const auto &ammo{jc.at(i)};
            if (ammo.at("name") == resultConfig->ammoName) {
                ammo.get_to(*resultAmmo);
                isAmmoFound = true;
                break;
            }
        }
    }
    catch (const json::exception &e) {
        throw std::invalid_argument("JsonConfigLoader::parse(): Failed to parse " + ammoPath.string() + ":\n" + e.what());
    }

    if (!isAmmoFound) {
        throw std::invalid_argument("JsonConfigLoader::parse(): No ammo with name \"" + resultConfig->ammoName + "\" in " +
                                    ammoPath.string());
    }

    return true;
}
