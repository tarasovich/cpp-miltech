#include "FileConfigLoader.hpp"
#include <fstream>

bool FileConfigLoader::load()
{
    if (this->isLoaded) {
        throw std::logic_error("FileConfigLoader::load(): already loaded");
    }

    if (!this->parseMainConfig(this->getMainPath())) {
        throw std::invalid_argument("FileConfigLoader::load(): could not load " + this->getMainPath().string());
    }

    if (!this->parseAmmoConfig(this->getAmmoPath())) {
        throw std::invalid_argument("FileConfigLoader::load(): could not load " + this->getAmmoPath().string());
    }

    this->isLoaded = true;

    return this->isLoaded;
}
