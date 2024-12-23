#include "ecs/systems/render_system.h"

#include "ecs/components/model.h"
#include "ecs/components/render_model.h"
#include "ecs/components/transform.h"

namespace game_engine {

// TODO: currently not implemented and not used (it's responsible for organizing
// and preparing entity data for rendering (different render passes), applying
// culling and LOD selection
void RenderSystem::update(const std::shared_ptr<Scene>& scene,
                          float                         deltaTime) {
  Registry& registry = scene->getEntityRegistry();

  // Handle entities with RenderModel and Transform components
  auto modelView = registry.view<Transform, RenderModel>();

  for (auto entity : modelView) {
    auto& renderModel = modelView.get<RenderModel>(entity);

    std::cout << "Entity " << static_cast<int>(entity)
              << " has RenderModel: " << renderModel.filePath << "\n";
  }

  // Handle entities with RenderMesh components
  auto meshView = registry.view<RenderMesh>();

  for (auto entity : meshView) {
    auto& renderMesh = meshView.get<RenderMesh>(entity);

    if (renderMesh.material) {
      std::cout << "Entity " << static_cast<int>(entity)
                << " has Material: " << renderMesh.material->materialName
                << "\n";
    }
  }
}

}  // namespace game_engine
