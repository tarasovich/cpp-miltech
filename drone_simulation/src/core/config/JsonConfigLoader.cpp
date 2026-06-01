
// To create better diagnostics messages, each JSON value needs a pointer to its parent value such that a global context (i.e., a path from
// the root value to the value that led to the exception) can be created. They can, however, be enabled by defining the preprocessor symbol
// JSON_DIAGNOSTICS to 1 before including json.hpp.
#define JSON_DIAGNOSTICS 1 // NOLINT (consider using a 'constexpr' constant )
#include "json.hpp"
#include "JsonConfigLoader.hpp"
#include <fstream>

using json = nlohmann::json;

void from_json(const json &j, Config &config)
{
    j["drone"]["position"]["x"].get_to(config.startPos.x);
    j["drone"]["position"]["y"].get_to(config.startPos.y);
    j["drone"]["altitude"].get_to(config.altitude);
    j["drone"]["initialDirection"].get_to(config.initialDir);
    j["drone"]["attackSpeed"].get_to(config.attackSpeed);
    j["drone"]["accelerationPath"].get_to(config.accelPath);
    j["drone"]["angularSpeed"].get_to(config.angularSpeed);
    j["drone"]["turnThreshold"].get_to(config.turnThreshold);
    j["simulation"]["timeStep"].get_to(config.simTimeStep);
    j["simulation"]["hitRadius"].get_to(config.hitRadius);
    j["targetArrayTimeStep"].get_to(config.arrayTimeStep);
    j["ammo"].get_to(config.ammoName);
}

void from_json(const json &j, AmmoParams &ammo)
{
    j.at("name").get_to(ammo.name);
    j.at("mass").get_to(ammo.mass);
    j.at("drag").get_to(ammo.drag);
    j.at("lift").get_to(ammo.lift);
}

bool JsonConfigLoader::parseMainConfig(const fs::path& filePath)
{
    std::ifstream fstream(filePath);

    if (!fstream.is_open()) {
        throw std::logic_error("JsonConfigLoader::parseMainConfig(): Failed to open " + filePath.string());
    }

    resultConfig = new Config;

    json jc{};
    try {
        fstream >> jc;
        jc.get_to(*resultConfig);
    } catch (const json::exception& e) {
        delete resultConfig;
        resultConfig = nullptr;

        throw std::logic_error(
            "JsonConfigLoader::parseMainConfig(): Failed to parse " + filePath.string() + ":\n" + e.what()
        );
    }

    return true;
}

bool JsonConfigLoader::parseAmmoConfig(const fs::path& filePath)
{
    std::ifstream fstream(filePath);

    if (!fstream.is_open()) {
        throw std::logic_error("JsonConfigLoader::parseAmmoConfig(): Failed to open " + filePath.string());
    }

    json jc{};
    try {
        fstream >> jc;
    } catch (const json::exception& e) {
        throw std::logic_error(
            "JsonConfigLoader::parseAmmoConfig(): Failed to parse " + filePath.string() + ":\n" + e.what()
        );
    }

    const int ammoCount{static_cast<int>(jc.size())};
    if (ammoCount < 1) {
        throw std::logic_error("JsonConfigLoader::parseAmmoConfig(): No ammos in " + filePath.string());
    }

    resultAmmoParams = new AmmoParams[ammoCount];

    try {
        for (int i = 0; i < ammoCount; i++) {
            resultAmmoParams[i] = jc[i].get<AmmoParams>();
        }
    } catch (const json::exception& e) {
        throw std::logic_error(
            "JsonConfigLoader::parseAmmoConfig(): Failed to parse " + filePath.string() + ":\n" + e.what()
        );
    }

    return true;
}
