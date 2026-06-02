#include "BallisticSolverFactory.hpp"
#include "ConfigLoaderFactory.hpp"
#include "FileConfigLoaderOptions.hpp"
#include "IBallisticSolver.hpp"
#include "ITargetProvider.hpp"
#include "JsonConfigLoader.hpp"
#include "MissionProcessor.hpp"
#include "TargetProviderFactory.hpp"
#include <cstring>
// ============================================================
// Usage
// ============================================================
void usage()
{
    std::cout << "Usage:\n"
              << "  drone_simulation [data_dir] [config_file] [ammo_file] [targets_file] [max_steps]\n\n"
              << "Arguments:\n"
              << "  data_dir     Directory containing configuration files.\n"
              << "               Default: drone_simulation/data\n"
              << "  config_file  Main configuration file name.\n"
              << "               Default: config.json\n"
              << "  ammo_file    Ammo configuration file name.\n"
              << "               Default: ammo.json\n\n"
              << "  targets_file Targets configuration file name.\n"
              << "               Default: targets.json\n\n"
              << "  max_steps    Maximum simulation steps count\n"
              << "               Default: 10000\n\n"
              << "Examples:\n"
              << "  drone_simulation\n"
              << "  drone_simulation ./data\n"
              << "  drone_simulation ./data config.json ammo.json targets.json 1000\n"
              << "  drone_simulation /etc/drone custom_config.json custom_ammo.json custom_targets.json 10000\n\n"
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

    const std::string dataDir = (argc > 1) ? argv[1] : "drone_simulation/data";
    const std::string configFilename = (argc > 2) ? argv[2] : "config.json";
    const std::string ammoFilename = (argc > 3) ? argv[3] : "ammo.json";
    const std::string targetsFilename = (argc > 4) ? argv[4] : "targets.json";
    uint16_t maxSteps = 10000;
    if (argc > 5) {
        try {
            maxSteps = std::stoi(argv[5]);
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
        delete configLoader;   // NOLINT(cppcoreguidelines-owning-memory)
        delete configOptions;  // NOLINT(cppcoreguidelines-owning-memory)
        delete targets;        // NOLINT(cppcoreguidelines-owning-memory)
        return 1;
    }

    IBallisticSolver *solver = nullptr;
    try {
        solver = BallisticSolverFactory::create(SolverType::ANALYTICAL);
    }
    catch (const std::exception &e) {
        std::cerr << "Error creating ballistic solver: " << e.what() << '\n';
        delete configLoader;   // NOLINT(cppcoreguidelines-owning-memory)
        delete configOptions;  // NOLINT(cppcoreguidelines-owning-memory)
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

    std::cout << "Steps count: " << missionProcessor->getCurrentStep() << '\n';

    delete configLoader;      // NOLINT(cppcoreguidelines-owning-memory)
    delete configOptions;     // NOLINT(cppcoreguidelines-owning-memory)
    delete targets;           // NOLINT(cppcoreguidelines-owning-memory)
    delete solver;            // NOLINT(cppcoreguidelines-owning-memory)
    delete missionProcessor;  // NOLINT(cppcoreguidelines-owning-memory)

    return result;
}