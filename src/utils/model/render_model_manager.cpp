#include "utils/model/render_model_manager.h"
#include "utils/model/render_mesh_manager.h"

namespace game_engine {
RenderModel* RenderModelManager::getRenderModel(const std::filesystem::path& filepath, std::optional<Model*> outModel) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = renderModelCache_.find(filepath);
  if (it != renderModelCache_.end()) {
    return it->second.get();
  }

  auto renderModelLoaderManager = ServiceLocator::s_get<RenderModelLoaderManager>();
  if (!renderModelLoaderManager) {
    GlobalLogger::Log(LogLevel::Error, "RenderModelLoaderManager not available in ServiceLocator.");
    return nullptr;
  }

  auto renderModel = renderModelLoaderManager->loadRenderModel(filepath, outModel);
  if (renderModel) {
    RenderModel* modelPtr       = renderModel.get();
    renderModelCache_[filepath] = std::move(renderModel);
    return modelPtr;
  }

  GlobalLogger::Log(LogLevel::Warning, "Failed to load render model: " + filepath.string());
  return nullptr;
}

bool RenderModelManager::removeRenderModel(RenderModel* renderModel) {
  if (!renderModel) {
    GlobalLogger::Log(LogLevel::Error, "Cannot remove null render model");
    return false;
  }

  GlobalLogger::Log(LogLevel::Info, "Removing render model: " + renderModel->filePath.string());

  auto renderMeshManager = ServiceLocator::s_get<RenderMeshManager>();

  if (renderMeshManager) {
    for (auto* renderMesh : renderModel->renderMeshes) {
      if (renderMesh) {
        renderMeshManager->removeRenderMesh(renderMesh);
      }
    }
  }

  std::lock_guard<std::mutex> lock(mutex_);

  auto it = std::find_if(renderModelCache_.begin(), renderModelCache_.end(), [renderModel](const auto& pair) {
    return pair.second.get() == renderModel;
  });

  if (it != renderModelCache_.end()) {
    renderModelCache_.erase(it);
    return true;
  }

  GlobalLogger::Log(LogLevel::Warning, "Render model not found in manager");
  return false;
}

}  // namespace game_engine