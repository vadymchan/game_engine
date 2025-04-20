#ifndef GAME_ENGINE_MATERIAL_MANAGER_H
#define GAME_ENGINE_MATERIAL_MANAGER_H

#include "utils/material/material_loader_manager.h"
#include "utils/service/service_locator.h"

#include <filesystem>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace game_engine {

class MaterialManager {
  public:
  MaterialManager() = default;

  std::vector<Material*> getMaterials(const std::filesystem::path& filepath) {
    std::lock_guard<std::mutex> lock(mutex_);

    // Check if materials are already loaded
    auto it = materialCache_.find(filepath);
    if (it != materialCache_.end()) {
      std::vector<Material*> result;
      for (const auto& material : it->second) {
        result.push_back(material.get());
      }
      return result;
    }

    // Get loader manager from service locator
    auto materialLoaderManager = ServiceLocator::s_get<MaterialLoaderManager>();
    if (!materialLoaderManager) {
      GlobalLogger::Log(LogLevel::Error, "MaterialLoaderManager not available in ServiceLocator.");
      return {};
    }

    // Load materials
    auto materials = materialLoaderManager->loadMaterials(filepath);
    if (!materials.empty()) {
      std::vector<Material*> result;

      auto& cacheEntry = materialCache_[filepath];

      for (auto& material : materials) {
        result.push_back(material.get());

        cacheEntry.push_back(std::move(material));
      }

      return result;
    }

    GlobalLogger::Log(LogLevel::Warning, "Failed to load materials from: " + filepath.string());
    return {};
  }


  private:
  std::unordered_map<std::filesystem::path, std::vector<std::unique_ptr<Material>>> materialCache_;
  std::mutex                                                                        mutex_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_MATERIAL_MANAGER_H