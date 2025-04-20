#ifndef GAME_ENGINE_FILE_SYSTEM_MANAGER_H
#define GAME_ENGINE_FILE_SYSTEM_MANAGER_H

#include <filesystem>
#include <optional>

namespace game_engine {

class FileSystemManager {
  public:
  static std::optional<std::string> readFile(
      const std::filesystem::path& filePath);

  static bool writeFile(const std::filesystem::path& filePath,
                        const std::string&           content);

  static bool fileExists(const std::filesystem::path& filePath);

  static std::vector<std::filesystem::path> getAllFilesInDirectory(
      const std::filesystem::path& dirPath);

  static bool createDirectory(const std::filesystem::path& dirPath);

  static bool remove(const std::filesystem::path& filePath);
};

}  // namespace game_engine

#endif  // GAME_ENGINE_FILE_SYSTEM_MANAGER_H
