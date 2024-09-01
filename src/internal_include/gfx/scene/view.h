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
  // ======= BEGIN: public constructors =======================================

  View() = default;

  View(const Camera* camera /*, const std::vector<Light*>& lights*/)
      : m_camera_(camera) {
    assert(camera);
    /*lights.resize(lights.size());
    for (int32_t i = 0; i < (int32_t)lights.size(); ++i) {
      new (&lights[i]) ViewLight(lights[i]);
    }*/
  }

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public getters ============================================

  void getShaderBindingInstance(
      ShaderBindingInstanceArray& shaderBindingInstanceArray,
      bool                        isForwardRenderer = false) const;

  // TODO: currently not used
  void getShaderBindingLayout(
      ShaderBindingLayoutArray& shaderBindingsLayoutArray,
      bool                      isForwardRenderer = false) const;

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc methods =======================================

  void prepareViewUniformBufferShaderBindingInstance();

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  const Camera*                        m_camera_ = nullptr;
  // std::vector<ViewLight>            m_lights_;
  // std::vector<ViewLight>            m_shadowCasterLights_;
  std::shared_ptr<IUniformBufferBlock> m_viewUniformBufferPtr_;
  std::shared_ptr<ShaderBindingInstance>
      m_viewUniformBufferShaderBindingInstance_;

  // ======= END: public misc fields   ========================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_VIEW_H