// TODO:
// - move to more appropriate folder
// - consider moving scene folder to a more appropriate location (gfx is not the
// very best place for it)

#include "gfx/scene/view.h"

#include "gfx/rhi/rhi.h"

namespace game_engine {

void View::PrepareViewUniformBufferShaderBindingInstance() {
  // Prepare & Get ViewUniformBuffer
  struct ViewUniformBuffer {
    math::Matrix4f  m_view;
    math::Matrix4f  m_projection;
    math::Matrix4f  m_VP;
    math::Vector3Df m_eyeWorld;
    float           m_padding0;
  };

  ViewUniformBuffer ubo;
  ubo.m_projection = m_camera_->m_projection_;
  ubo.m_view       = m_camera_->m_view_;
  ubo.m_VP         = m_camera_->m_projection_ * m_camera_->m_view_;
  ubo.m_eyeWorld   = m_camera_->m_position_;

  m_viewUniformBufferPtr_ = std::shared_ptr<IUniformBufferBlock>(
      g_rhi->CreateUniformBufferBlock(NameStatic("ViewUniformParameters"),
                                      LifeTimeType::OneFrame,
                                      sizeof(ubo)));
  m_viewUniformBufferPtr_->UpdateBufferData(&ubo, sizeof(ubo));

  int32_t                              BindingPoint = 0;
  ShaderBindingArray                   shaderBindingArray;
  ShaderBindingResourceInlineAllocator ResourceInlineAllactor;

  shaderBindingArray.Add(
      ShaderBinding(BindingPoint++,
                    1,
                    EShaderBindingType::UNIFORMBUFFER_DYNAMIC,
                    false,
                    EShaderAccessStageFlag::ALL_GRAPHICS,
                    ResourceInlineAllactor.Alloc<UniformBufferResource>(
                        m_viewUniformBufferPtr_.get())));

  m_viewUniformBufferShaderBindingInstance_
      = g_rhi->CreateShaderBindingInstance(
          shaderBindingArray, ShaderBindingInstanceType::SingleFrame);
}

void View::GetShaderBindingInstance(
    ShaderBindingInstanceArray& shaderBindingInstanceArray,
    bool                        isForwardRenderer) const {
  shaderBindingInstanceArray.Add(
      m_viewUniformBufferShaderBindingInstance_.get());

  // Get light uniform buffers
  // if (isForwardRenderer) {
  //  for (int32_t i = 0; i < lights.size(); ++i) {
  //    const ViewLight& viewLight = lights[i];
  //    if (viewLight.Light) {
  //      assert(viewLight.m_shaderBindingInstance);
  //      shaderBindingInstanceArray.Add(
  //          viewLight.m_shaderBindingInstance.get());
  //    }
  //  }
  //}
}

// TODO: currently not used
void View::GetShaderBindingLayout(
    ShaderBindingLayoutArray& shaderBindingsLayoutArray,
    bool                      isForwardRenderer) const {
  shaderBindingsLayoutArray.Add(
      m_viewUniformBufferShaderBindingInstance_->m_shaderBindingsLayouts_);

  // Get light uniform buffers
  // if (isForwardRenderer) {
  //  for (int32_t i = 0; i < lights.size(); ++i) {
  //    const ViewLight& viewLight = lights[i];
  //    if (viewLight.Light) {
  //      assert(viewLight.m_shaderBindingInstance);
  //      shaderBindingsLayoutArray.Add(
  //          viewLight.m_shaderBindingInstance->m_shaderBindingsLayouts_);
  //    }
  //  }
  //}
}

}  // namespace game_engine
