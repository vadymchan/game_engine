#include "file_loader/file_system_manager.h"

#include <fstream>
#include <iostream>

namespace game_engine {
std::optional<std::string> FileSystemManager::readFile(
    const std::filesystem::path& filePath) {
  std::ifstream file(filePath, std::ios::in | std::ios::binary);
  if (!file) {
    // TODO: use logger
    std::cerr << "Failed to open file: " << filePath << std::endl;
    return std::nullopt;
  }

  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();
  return content;
}

bool FileSystemManager::writeFile(const std::filesystem::path& filePath,
                                  const std::string&           content) {
  std::ofstream file(filePath, std::ios::out | std::ios::binary);
  if (!file) {
    // TODO: use logger
    std::cerr << "Failed to open file for writing: " << filePath << std::endl;
    return false;
  }

  file << content;
  file.close();
  // TODO: use logger
  std::cout << "File written successfully: " << filePath << std::endl;
  return true;
}

bool FileSystemManager::fileExists(const std::filesystem::path& filePath) {
  return std::filesystem::exists(filePath);
}

std::vector<std::filesystem::path> FileSystemManager::getAllFilesInDirectory(
    const std::filesystem::path& dirPath) {
  std::vector<std::filesystem::path> filePaths;

  if (std::filesystem::exists(dirPath)
      && std::filesystem::is_directory(dirPath)) {
    for (const auto& entry :
         std::filesystem::recursive_directory_iterator(dirPath)) {
      if (entry.is_regular_file()) {
        filePaths.push_back(entry.path());
      }
    }
  } else {
    // TODO: use logger
    std::cerr << "Directory does not exist or is not a directory: " << dirPath
              << std::endl;
  }

  return filePaths;
}

bool FileSystemManager::createDirectory(const std::filesystem::path& dirPath) {
  if (!std::filesystem::exists(dirPath)) {
    if (std::filesystem::create_directory(dirPath)) {
      // TODO: use logger
      std::cout << "Directory created successfully: " << dirPath << std::endl;
      return true;
    } else {
      // TODO: use logger
      std::cerr << "Failed to create directory: " << dirPath << std::endl;
      return false;
    }
  } else {
    // TODO: use logger
    std::cerr << "Directory already exists: " << dirPath << std::endl;
    return false;
  }
}

bool FileSystemManager::remove(const std::filesystem::path& filePath) {
  if (std::filesystem::exists(filePath)) {
    if (std::filesystem::remove_all(filePath) > 0) {
      // TODO: use logger
      std::cout << "File or directory removed successfully: " << filePath
                << std::endl;
      return true;
    } else {
      // TODO: use logger
      std::cerr << "Failed to remove file or directory: " << filePath
                << std::endl;
      return false;
    }
  } else {
    // TODO: use logger
    std::cerr << "File or directory does not exist: " << filePath << std::endl;
    return false;
  }
}
}  // namespace game_engine
