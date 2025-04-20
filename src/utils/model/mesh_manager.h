#ifndef GAME_ENGINE_MESH_MANAGER_H
#define GAME_ENGINE_MESH_MANAGER_H

#include "ecs/components/mesh.h"

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace game_engine {


class MeshManager {
  public:
  MeshManager()  = default;
  ~MeshManager() = default;

  /**
   * @param sourcePath Source file path (used for identification)
   */
  Mesh* addMesh(std::unique_ptr<Mesh> mesh, const std::filesystem::path& sourcePath);

  Mesh* getMesh(const std::filesystem::path& sourcePath, const std::string& meshName);

  private:
  // Map of meshes keyed by unique identifier (path + name)
  std::unordered_map<std::string, std::unique_ptr<Mesh>> m_meshes;
  mutable std::mutex                                     m_mutex;

  std::string createKey(const std::filesystem::path& path, const std::string& name) const;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_MESH_MANAGER_H