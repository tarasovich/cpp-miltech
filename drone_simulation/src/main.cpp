#include "ConfigLoaderFactory.hpp"
#include "FileConfigLoaderOptions.hpp"
#include "ITargetProvider.hpp"
#include "JsonConfigLoader.hpp"
#include "TargetProviderFactory.hpp"
#include <cstring>
// ============================================================
// Usage
// ============================================================
void usage()
{
    std::cout << "Usage:\n"
              << "  drone_simulation [data_dir] [config_file] [ammo_file] [targets_file]\n\n"
              << "Arguments:\n"
              << "  data_dir     Directory containing configuration files.\n"
              << "               Default: drone_simulation/data\n"
              << "  config_file  Main configuration file name.\n"
              << "               Default: config.json\n"
              << "  ammo_file    Ammo configuration file name.\n"
              << "               Default: ammo.json\n\n"
              << "  targets_file Targets configuration file name.\n"
              << "               Default: targets.json\n\n"
              << "Examples:\n"
              << "  drone_simulation\n"
              << "  drone_simulation ./data\n"
              << "  drone_simulation ./data config.json ammo.json targets.json"
              << "  drone_simulation /etc/drone custom_config.json custom_ammo.json custom_targets.json\n\n"
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

    const IConfigLoaderOptions *configOptions = new FileConfigLoaderOptions(dataDir, configFilename, ammoFilename);
    IConfigLoader *configLoader = ConfigLoaderFactory::create(*configOptions);

    const ITargetProvider *targets = TargetProviderFactory::create(ProviderType::JSON, dataDir + "/" + targetsFilename);

    if (!configLoader->load()) {
        return 1;
    }

    const auto config = configLoader->getConfig();
    const auto ammoParams = configLoader->getAmmoParams();

    std::cout << *config << '\n';
    std::cout << *ammoParams << '\n';
    std::cout << *targets << '\n';

    delete configLoader;
    delete configOptions;

    return 0;
}
