#ifndef GAME_ENGINE_I_RENDER_MODEL_LOADER_H
#define GAME_ENGINE_I_RENDER_MODEL_LOADER_H

#include "ecs/components/model.h"
#include "ecs/components/render_model.h"

#include <optional>

namespace game_engine {

// we made RenderModelLoader friend to ModelLoader and MaterialLoader so we can
// access. As an alternative we can expose private methods to public.

// TODO: consider merge RenderModelLoader and ModelLoader (since this may
// confuse users)
class IRenderModelLoader {
  public:
  virtual ~IRenderModelLoader() = default;

  // Optionally returns CPU-side geometry model (outModel)
  virtual std::unique_ptr<RenderModel> loadRenderModel(const std::filesystem::path& filepath,
                                                       std::optional<Model*>        outModel = std::nullopt)
      = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_I_RENDER_MODEL_LOADER_H
