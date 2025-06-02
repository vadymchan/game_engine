#ifndef ARISE_CONFIG_MANAGER_H
#define ARISE_CONFIG_MANAGER_H

#include "config/config.h"

namespace arise {

class ConfigManager {
  public:
  static constexpr std::string_view s_configExtension = ".json";

  void loadAllConfigsFromDirectory(const std::filesystem::path& dirPath);

  void addConfig(const std::filesystem::path& filePath);

  Config* getConfig(const std::filesystem::path& filePath);

  void saveAllConfigs();

  bool unloadConfig(const std::filesystem::path& filePath);

  void unloadAllConfigs();

  private:
  std::unordered_map<std::filesystem::path, std::unique_ptr<Config>> configs_;
};

}  // namespace arise

#endif  // ARISE_CONFIG_MANAGER_H
