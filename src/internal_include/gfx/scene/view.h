// TODO:
// - move to more appropriate folder
// - consider moving scene folder to a more appropriate location (gfx is not the
// very best place for it)

#ifndef GAME_ENGINE_VIEW_H
#define GAME_ENGINE_VIEW_H

#include "gfx/rhi/i_uniform_buffer_block.h"
#include "gfx/rhi/shader_binding_layout.h"
#include "gfx/scene/camera.h"

#include <vector>

namespace game_engine {

class View {
  public:
  View() = default;

  View(const Camera* camera /*, const std::vector<Light*>& lights*/)
      : m_camera_(camera) {
    assert(camera);
    /*lights.resize(lights.size());
    for (int32_t i = 0; i < (int32_t)lights.size(); ++i) {
      new (&lights[i]) ViewLight(lights[i]);
    }*/
  }

  void prepareViewUniformBufferShaderBindingInstance();

  void getShaderBindingInstance(
      ShaderBindingInstanceArray& shaderBindingInstanceArray,
      bool                        isForwardRenderer = false) const;

  // TODO: currently not used
  void getShaderBindingLayout(
      ShaderBindingLayoutArray& shaderBindingsLayoutArray,
      bool                      isForwardRenderer = false) const;

  const Camera*                          m_camera_ = nullptr;
  // std::vector<ViewLight>              lights;
  // std::vector<ViewLight>              ShadowCasterLights;
  std::shared_ptr<IUniformBufferBlock>   m_viewUniformBufferPtr_;
  std::shared_ptr<ShaderBindingInstance> m_viewUniformBufferShaderBindingInstance_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_VIEW_H