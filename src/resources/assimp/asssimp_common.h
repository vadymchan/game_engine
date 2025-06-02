#ifndef ASSIMP_COMMON_H
#define ASSIMP_COMMON_H

#ifdef ARISE_USE_ASSIMP

#include "utils/logger/global_logger.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

#include <assimp/Importer.hpp>

namespace arise {

class AssimpSceneCache {
  public:
  static std::shared_ptr<const aiScene> getOrLoad(const std::filesystem::path& path) {
    auto absPath = std::filesystem::absolute(path).string();

    auto it = s_cache.find(absPath);
    if (it != s_cache.end()) {
      if (auto ptr = it->second.lock()) {
        return ptr;
      }
    }

    auto importer = new Assimp::Importer();
    auto flags    = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace;
    const aiScene* sceneRaw = importer->ReadFile(absPath, flags);

    if (!sceneRaw || sceneRaw->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !sceneRaw->mRootNode) {
      GlobalLogger::Log(LogLevel::Error,
                        "Assimp scene is invalid or incomplete for file \"" + path.string()
                            + "\". Error: " + importer->GetErrorString());
      delete importer;
      return nullptr;
    }

    std::shared_ptr<const aiScene> scenePtr(sceneRaw, [importer](const aiScene*) { delete importer; });
    s_cache.emplace(absPath, scenePtr);
    return scenePtr;
  }

  private:
  static inline std::unordered_map<std::string, std::weak_ptr<const aiScene>> s_cache;
};

}  // namespace arise

#endif // ARISE_USE_ASSIMP

#endif  // ASSIMP_COMMON_H
