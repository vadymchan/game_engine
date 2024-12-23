#include "config/config_manager.h"

#include "file_loader/file_system_manager.h"

#include <ranges>

namespace game_engine {

void ConfigManager::loadAllConfigsFromDirectory(
    const std::filesystem::path& dirPath) {
  auto files = FileSystemManager::getAllFilesInDirectory(dirPath);

  for (const auto& file : files) {
    if (file.extension() == s_configExtension) {
      addConfig(file);
    }
    // TODO: use logger
    std::cerr << "Skipping non-config file: " << file << std::endl;
  }
}

void ConfigManager::addConfig(const std::filesystem::path& filePath) {
  if (configs_.find(filePath) != configs_.end()) {
    // TODO: use logger
    std::cerr << "Config already exists with name: " << filePath << std::endl;
    return;
  }

  auto config = std::make_shared<Config>();
  config->loadFromFileAsync(filePath);

  configs_[filePath] = config;
}

std::weak_ptr<Config> ConfigManager::getConfig(
    const std::filesystem::path& filePath) {
  if (configs_.find(filePath) != configs_.end()) {
    return configs_[filePath];
  } else {
    // TODO: use logger
    std::cerr << "Config not found: " << filePath << std::endl;
    return std::weak_ptr<Config>();
  }
}

void ConfigManager::saveAllConfigs() {
  for (const auto& config : configs_ | std::views::values) {
    FileSystemManager::writeFile(config->getFilename(), config->toString());
  }
}

bool ConfigManager::unloadConfig(const std::filesystem::path& filePath) {
  if (configs_.erase(filePath)) {
    // TODO: use logger
    std::cout << "Config " << filePath << " unloaded from memory." << std::endl;
    return true;
  } else {
    // TODO: use logger
    std::cerr << "Config " << filePath << " not found." << std::endl;
    return false;
  }
}

void ConfigManager::unloadAllConfigs() {
  configs_.clear();
  // TODO: use logger
  std::cout << "All configs have been unloaded from memory." << std::endl;
}

}  // namespace game_engine
