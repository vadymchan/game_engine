#include "config/config_manager.h"

#include "file_loader/file_system_manager.h"
#include "utils/logger/global_logger.h"

#include <ranges>

namespace game_engine {

void ConfigManager::loadAllConfigsFromDirectory(
    const std::filesystem::path& dirPath) {
  auto files = FileSystemManager::getAllFilesInDirectory(dirPath);

  for (const auto& file : files) {
    if (file.extension() == s_configExtension) {
      addConfig(file);
    }
    GlobalLogger::Log(LogLevel::Info,
                      "Skipping non-config file: " + file.string());
  }
}

void ConfigManager::addConfig(const std::filesystem::path& filePath) {
  if (configs_.find(filePath) != configs_.end()) {
    GlobalLogger::Log(LogLevel::Warning,
                      "Config already exists with name: " + filePath.string());
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
    GlobalLogger::Log(LogLevel::Error,
                      "Config not found: " + filePath.string());
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
    GlobalLogger::Log(LogLevel::Info,
                      "Config " + filePath.string() + " unloaded from memory.");
    return true;
  } else {
    GlobalLogger::Log(LogLevel::Error,
                      "Config " + filePath.string() + " not found.");
    return false;
  }
}

void ConfigManager::unloadAllConfigs() {
  configs_.clear();
  GlobalLogger::Log(LogLevel::Info,
                    "All configs have been unloaded from memory.");
}

}  // namespace game_engine
