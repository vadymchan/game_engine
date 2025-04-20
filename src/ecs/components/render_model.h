#ifndef GAME_ENGINE_RENDER_MODEL_H
#define GAME_ENGINE_RENDER_MODEL_H

#include "render_mesh.h"

#include <filesystem>
#include <memory>
#include <vector>

namespace game_engine {

struct RenderModel {
  std::filesystem::path    filePath;
  std::vector<RenderMesh*> renderMeshes;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_MODEL_H
