// TODO:
// - move to more appropriate folder
// - consider moving scene folder to a more appropriate location (gfx is not the
// very best place for it)

#ifndef GAME_ENGINE_VIEW_H
#define GAME_ENGINE_VIEW_H

#include "gfx/rhi/shader_binding_layout.h"
#include "gfx/rhi/i_uniform_buffer_block.h" 
#include "gfx/scene/camera.h"

#include <vector>

namespace game_engine {

class View {
  public:
  View() = default;

  View(const Camera* camera/*, const std::vector<Light*>& InLights*/)
      : Camera(camera) {
    assert(camera);
    /*lights.resize(InLights.size());
    for (int32_t i = 0; i < (int32_t)lights.size(); ++i) {
      new (&lights[i]) ViewLight(InLights[i]);
    }*/
  }

  void PrepareViewUniformBufferShaderBindingInstance();

  void GetShaderBindingInstance(
      ShaderBindingInstanceArray& OutShaderBindingInstanceArray,
      bool                        InIsForwardRenderer = false) const;

  // TODO: currently not used
  void GetShaderBindingLayout(
      ShaderBindingLayoutArray& OutShaderBindingsLayoutArray,
      bool                        InIsForwardRenderer = false) const;

  const Camera*                         Camera = nullptr;
  //std::vector<ViewLight>                lights;
  //std::vector<ViewLight>                ShadowCasterLights;
  std::shared_ptr<IUniformBufferBlock>   ViewUniformBufferPtr;
  std::shared_ptr<ShaderBindingInstance> ViewUniformBufferShaderBindingInstance;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_VIEW_H