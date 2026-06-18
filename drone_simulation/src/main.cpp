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
#include "CLI11.hpp"
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
// CLI Options
// ============================================================
struct CliOptions {
    std::filesystem::path dataDir{"drone_simulation/data/test01_base"};
    std::string configFilename{"config.json"};
    std::string ammoFilename{"ammo_table.json"};
    std::string targetsFilename{"targets.json"};
    std::string tableFilename{"ballistic_table.txt"};
    SolverType solverType{SolverType::TABLE};
    std::uint16_t maxSteps{10000};
    std::filesystem::path outputFilename{"simulation.json"};
};

// ============================================================
// Програма
// ============================================================
int main(const int argc, char *argv[])
{
    CliOptions options{};
    std::unique_ptr<CLI::App> app;

    try {
        app = std::make_unique<CLI::App>("Drone simulation");
        app->add_option("--data-dir", options.dataDir, "Directory containing configuration files")->default_val(options.dataDir.string());
        app->add_option("--config", options.configFilename, "Main configuration file name")->default_val(options.configFilename);
        app->add_option("--ammo", options.ammoFilename, "Ammo configuration file name")->default_val(options.ammoFilename);
        app->add_option("--targets", options.targetsFilename, "Targets configuration file name")->default_val(options.targetsFilename);
        app->add_option("--table", options.tableFilename, "Ballistic table file name")->default_val(options.tableFilename);
        app->add_option("--solver", options.solverType, "Ballistic solver type: analytical or table")
            ->default_val(options.solverType)
            ->transform(CLI::CheckedTransformer(
                std::map<std::string, SolverType>{{"analytical", SolverType::ANALYTICAL}, {"table", SolverType::TABLE}}, CLI::ignore_case));
        app->add_option("--output", options.outputFilename, "Simulation output JSON file path")
            ->default_val(options.outputFilename.string());
        app->add_option("--max-steps", options.maxSteps, "Maximum simulation steps count")
            ->default_val(options.maxSteps)
            ->check(CLI::Range(1, 65535));
        app->parse(argc, argv);
    }
    catch (const CLI::ParseError &e) {
        return app->exit(e);
    }
    catch (const std::exception &e) {
        std::cerr << "Error during initialization: " << e.what() << '\n';
        return 1;
    }

    std::cout << "data dir: " << options.dataDir << '\n';
    std::cout << "config: " << options.configFilename << '\n';
    std::cout << "ammo: " << options.ammoFilename << '\n';
    std::cout << "targets: " << options.targetsFilename << '\n';
    std::cout << "table: " << options.tableFilename << '\n';
    std::cout << "solver: " << options.solverType << '\n';
    std::cout << "output: " << options.outputFilename << '\n';
    std::cout << "max steps: " << options.maxSteps << '\n';
    std::cout << '\n';

    // > а тут взагалі потрібно використовувати стек?
    const auto configOptions = std::make_unique<FileConfigLoaderOptions>(options.dataDir, options.configFilename, options.ammoFilename);

    std::unique_ptr<IConfigLoader> configLoader = nullptr;
    std::unique_ptr<ITargetProvider> targets = nullptr;
    std::unique_ptr<IBallisticSolver> solver = nullptr;
    try {
        configLoader = ConfigLoaderFactory::create(*configOptions);
        targets = TargetProviderFactory::create(ProviderType::JSON, options.dataDir.string() + "/" + options.targetsFilename);
        solver = BallisticSolverFactory::create(options.solverType, options.dataDir.string() + "/" + options.tableFilename);
    }
    catch (const std::exception &e) {
        std::cerr << "Error during initializing: " << e.what() << '\n';
        return 1;
    }

    int result = 0;

    std::unique_ptr<MissionProcessor> missionProcessor;
    try {
        missionProcessor = std::make_unique<MissionProcessor>(options.maxSteps);
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
        std::ofstream outFile(options.outputFilename);
        if (!outFile) {
            std::cerr << "Error opening output file: " << options.outputFilename << '\n';
            result = 1;
        }
        else {
            try {
                auto jo = json::object();
                jo.emplace("totalSteps", missionProcessor->getStepsCount());
                jo.emplace("steps", json::array());
                for (const auto &step : missionProcessor->getSteps()) {
                    jo.at("steps").push_back(step);
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

    return result;
}