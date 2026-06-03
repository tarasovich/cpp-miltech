#include "types.hpp"
#include "BallisticSolverFactory.hpp"
#include "ConfigLoaderFactory.hpp"
#include "FileConfigLoaderOptions.hpp"
#include "IBallisticSolver.hpp"
#include "ITargetProvider.hpp"
#include "MissionProcessor.hpp"
#include "TargetProviderFactory.hpp"
// To create better diagnostics messages, each JSON value needs a pointer to its parent value such that a global context (i.e., a path from
// the root value to the value that led to the exception) can be created. They can, however, be enabled by defining the preprocessor symbol
// JSON_DIAGNOSTICS to 1 before including json.hpp.
#define JSON_DIAGNOSTICS 1  // NOLINT (consider using a 'constexpr' constant )
#include "json.hpp"
#include <fstream>
#include <cstring>

using json = nlohmann::json;

using namespace miltech::simulation;

namespace miltech::simulation {
void to_json(json &j, const Coord &c)
{
    j = json{{"x", c.x}, {"y", c.y}};
}

void to_json(json &j, const SimStep &step)
{
    j = json{{"position", step.pos},
             {"direction", step.direction},
             {"state", step.state},
             {"targetIndex", step.targetIdx},
             {"dropPoint", step.ballistic.fireCoords},
             {"aimPoint", step.aimPoint},
             {"predictedTarget", step.predictedTarget}};
}
}  // namespace miltech::simulation

// ============================================================
// Usage
// ============================================================
void usage()
{
    std::cout << "Usage:\n"
              << "  drone_simulation [data_dir] [config_file] [ammo_file] [targets_file] [output_file] [max_steps]\n\n"

              << "Arguments:\n"

              << "  data_dir      Directory containing configuration files.\n"
              << "                Default: drone_simulation/data/test01_base\n\n"

              << "  config_file   Main configuration file name.\n"
              << "                Resolved relative to data_dir.\n"
              << "                Default: config.json\n\n"

              << "  ammo_file     Ammo configuration file name.\n"
              << "                Resolved relative to data_dir.\n"
              << "                Default: ammo.json\n\n"

              << "  targets_file  Targets configuration file name.\n"
              << "                Resolved relative to data_dir.\n"
              << "                Default: targets.json\n\n"

              << "  output_file   Simulation output JSON file path.\n"
              << "                This path is independent of data_dir.\n"
              << "                Can be absolute or relative.\n"
              << "                Default: simulation.json\n\n"

              << "  max_steps     Maximum simulation steps count.\n"
              << "                Default: 10000\n\n"

              << "Special value:\n"
              << "  _             Use default value for this argument.\n\n"

              << "Examples:\n"
              << "  drone_simulation\n"
              << "  drone_simulation ./data\n"
              << "  drone_simulation ./data config.json ammo.json targets.json simulation.json 1000\n"
              << "  drone_simulation _ _ _ custom_targets.json _ 5000\n"
              << "  drone_simulation ./data _ _ _ ./output/result.json\n"
              << "  drone_simulation /etc/drone custom_config.json custom_ammo.json custom_targets.json "
              << "/tmp/result.json 10000\n\n"

              << "Options:\n"
              << "  -h, --help, help   Show this help message.\n";
}

// ============================================================
// Програма
// ============================================================
int main(const int argc, char *argv[])
{
    if (argc > 1) {
        if (std::strcmp(argv[1], "help") == 0 || std::strcmp(argv[1], "--help") == 0 || std::strcmp(argv[1], "-h") == 0) {
            usage();
            return 0;
        }
    }

    const std::string dataDir = (argc > 1 && std::strcmp(argv[1], "_") != 0) ? argv[1] : "drone_simulation/data/test01_base";
    const std::string configFilename = (argc > 2 && std::strcmp(argv[2], "_") != 0) ? argv[2] : "config.json";
    const std::string ammoFilename = (argc > 3 && std::strcmp(argv[3], "_") != 0) ? argv[3] : "ammo.json";
    const std::string targetsFilename = (argc > 4 && std::strcmp(argv[4], "_") != 0) ? argv[4] : "targets.json";
    const std::string outputFilename = (argc > 5 && std::strcmp(argv[5], "_") != 0) ? argv[5] : "simulation.json";
    uint16_t maxSteps = 10000;
    if (argc > 6 && std::strcmp(argv[6], "_") != 0) {
        try {
            maxSteps = std::stoi(argv[6]);
        }
        catch (const std::exception &e) {
            std::cerr << "Error parsing max steps: " << e.what() << '\n';
            return 1;
        }
    }

    const IConfigLoaderOptions *configOptions = new FileConfigLoaderOptions(dataDir, configFilename, ammoFilename);

    IConfigLoader *configLoader = nullptr;
    try {
        configLoader = ConfigLoaderFactory::create(*configOptions);
    }
    catch (const std::exception &e) {
        std::cerr << "Error creating config loader: " << e.what() << '\n';
        delete configOptions;  // NOLINT(cppcoreguidelines-owning-memory)
        return 1;
    }

    const ITargetProvider *targets = nullptr;
    try {
        targets = TargetProviderFactory::create(ProviderType::JSON, dataDir + "/" + targetsFilename);
    }
    catch (const std::exception &e) {
        std::cerr << "Error loading targets: " << e.what() << '\n';
        delete configOptions;  // NOLINT(cppcoreguidelines-owning-memory)
        delete configLoader;   // NOLINT(cppcoreguidelines-owning-memory)
        delete targets;        // NOLINT(cppcoreguidelines-owning-memory)
        return 1;
    }

    IBallisticSolver *solver = nullptr;
    try {
        solver = BallisticSolverFactory::create(SolverType::ANALYTICAL);
    }
    catch (const std::exception &e) {
        std::cerr << "Error creating ballistic solver: " << e.what() << '\n';
        delete configOptions;  // NOLINT(cppcoreguidelines-owning-memory)
        delete configLoader;   // NOLINT(cppcoreguidelines-owning-memory)
        delete targets;        // NOLINT(cppcoreguidelines-owning-memory)
        return 1;
    }

    int result = 0;

    MissionProcessor *missionProcessor = nullptr;
    try {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        missionProcessor = new MissionProcessor(maxSteps);
        missionProcessor->init(configLoader, targets);
        missionProcessor->changeSolver(solver);

        while (missionProcessor->hasNext()) {
            if (!missionProcessor->step()) {
                throw std::runtime_error("mission step failed");
            }
        }
    }
    catch (const std::exception &e) {
        std::cerr << "Error during mission processor: " << e.what() << '\n';
        result = 1;
    }

    std::cout << "Steps count: " << missionProcessor->getStepsCount() << '\n';

    // ============================================================
    // simulation.json (вихідний файл) - Результат симуляції.
    // ============================================================
    if (result < 1) {
        std::ofstream outFile(outputFilename);
        if (!outFile) {
            std::cerr << "Error opening output file: " << outputFilename << '\n';
            result = 1;
        }
        else {
            try {
                auto jo = json::object();
                jo.emplace("totalSteps", missionProcessor->getStepsCount());
                jo.emplace("steps", json::array());
                for (uint16_t i = 0; i < missionProcessor->getStepsCount(); i++) {
                    jo.at("steps").push_back(missionProcessor->getSteps()[i]);
                }
                outFile << jo.dump(2) << '\n';
                outFile.close();
            }
            catch (const std::exception &e) {
                std::cerr << "Error writing output file: " << e.what() << '\n';
                result = 1;
            }
        }
    }

    std::cout << '\n';

    delete configOptions;     // NOLINT(cppcoreguidelines-owning-memory)
    delete configLoader;      // NOLINT(cppcoreguidelines-owning-memory)
    delete targets;           // NOLINT(cppcoreguidelines-owning-memory)
    delete solver;            // NOLINT(cppcoreguidelines-owning-memory)
    delete missionProcessor;  // NOLINT(cppcoreguidelines-owning-memory)

    return result;
}