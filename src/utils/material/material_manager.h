#ifndef ARISE_MATERIAL_MANAGER_H
#define ARISE_MATERIAL_MANAGER_H

#include "utils/material/material_loader_manager.h"
#include "utils/service/service_locator.h"
#include "utils/texture/texture_manager.h"

#include <filesystem>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace arise {

class MaterialManager {
  public:
  MaterialManager() = default;

  std::vector<Material*> getMaterials(const std::filesystem::path& filepath) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = materialCache_.find(filepath);
    if (it != materialCache_.end()) {
      std::vector<Material*> result;
      for (const auto& material : it->second) {
        result.push_back(material.get());
      }
      return result;
    }

    auto materialLoaderManager = ServiceLocator::s_get<MaterialLoaderManager>();
    if (!materialLoaderManager) {
      GlobalLogger::Log(LogLevel::Error, "MaterialLoaderManager not available in ServiceLocator.");
      return {};
    }

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

  bool removeMaterial(Material* material) {
    if (!material) {
      GlobalLogger::Log(LogLevel::Error, "Cannot remove null material");
      return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = materialCache_.begin(); it != materialCache_.end(); ++it) {
      auto& materialVec = it->second;
      auto  materialIt  = std::find_if(materialVec.begin(),
                                     materialVec.end(),
                                     [material](const std::unique_ptr<Material>& m) { return m.get() == material; });

      if (materialIt != materialVec.end()) {
        GlobalLogger::Log(LogLevel::Info, "Removing material: " + material->materialName);

        auto textureManager = ServiceLocator::s_get<TextureManager>();
        if (textureManager) {
          for (const auto& [textureName, texturePtr] : material->textures) {
            if (texturePtr) {
              GlobalLogger::Log(
                  LogLevel::Debug,
                  "Releasing texture '" + textureName + "' from material '" + material->materialName + "'");
              textureManager->removeTexture(texturePtr);
            }
          }
        } else {
          GlobalLogger::Log(LogLevel::Warning, "TextureManager not available, textures may not be properly released");
        }

        GlobalLogger::Log(LogLevel::Info, "Material '" + material->materialName + "' deleted");

        materialVec.erase(materialIt);
        if (materialVec.empty()) {
          materialCache_.erase(it);
        }
        return true;
      }
    }

    GlobalLogger::Log(LogLevel::Debug, "Material not found in manager (may have been removed already)");
    return false;
  }

  private:
  std::unordered_map<std::filesystem::path, std::vector<std::unique_ptr<Material>>> materialCache_;
  std::mutex                                                                        mutex_;
};

}  // namespace arise

#endif  // ARISE_MATERIAL_MANAGER_H