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

  std::vector<std::shared_ptr<Material>> getMaterials(
      const std::filesystem::path& filepath) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto                        it = materialCache_.find(filepath);
    if (it != materialCache_.end()) {
      return it->second;
    }

    auto materialLoaderManager = ServiceLocator::s_get<MaterialLoaderManager>();
    if (!materialLoaderManager) {
      // TODO: Logging if needed
      return {};
    }

    auto materials = materialLoaderManager->loadMaterials(filepath);
    if (!materials.empty()) {
      materialCache_[filepath] = materials;
    }
    return materials;
  }

  private:
  std::unordered_map<std::filesystem::path,
                     std::vector<std::shared_ptr<Material>>>
             materialCache_;
  std::mutex mutex_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_MATERIAL_MANAGER_H
