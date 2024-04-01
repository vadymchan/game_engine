// TODO:
// - move to more appropriate folder
// - consider moving scene folder to a more appropriate location (gfx is not the
// very best place for it)

#include "gfx/scene/view.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

void View::PrepareViewUniformBufferShaderBindingInstance() {
  // Prepare & Get ViewUniformBuffer
  struct ViewUniformBuffer {
    math::Matrix4f  V;
    math::Matrix4f  P;
    math::Matrix4f  VP;
    math::Vector3Df EyeWorld;
    float           padding0;
  };

  ViewUniformBuffer ubo;
  ubo.P        = Camera->Projection;
  ubo.V        = Camera->view;
  ubo.VP       = Camera->Projection * Camera->view;
  ubo.EyeWorld = Camera->Pos;

  ViewUniformBufferPtr = std::shared_ptr<IUniformBufferBlock>(
      g_rhi_vk->CreateUniformBufferBlock(NameStatic("ViewUniformParameters"),
                                         LifeTimeType::OneFrame,
                                         sizeof(ubo)));
  ViewUniformBufferPtr->UpdateBufferData(&ubo, sizeof(ubo));

  int32_t                              BindingPoint = 0;
  ShaderBindingArray                   ShaderBindingArray;
  ShaderBindingResourceInlineAllocator ResourceInlineAllactor;

  ShaderBindingArray.Add(
      ShaderBindingVk(BindingPoint++,
                      1,
                      EShaderBindingType::UNIFORMBUFFER_DYNAMIC,
                      EShaderAccessStageFlag::ALL_GRAPHICS,
                      ResourceInlineAllactor.Alloc<UniformBufferResource>(
                          ViewUniformBufferPtr.get())));

  ViewUniformBufferShaderBindingInstance
      = g_rhi_vk->CreateShaderBindingInstance(
          ShaderBindingArray, ShaderBindingInstanceType::SingleFrame);
}

void View::GetShaderBindingInstance(
    ShaderBindingInstanceArray& OutShaderBindingInstanceArray,
    bool                        InIsForwardRenderer ) const {
  OutShaderBindingInstanceArray.Add(
      ViewUniformBufferShaderBindingInstance.get());

  // Get light uniform buffers
  // if (InIsForwardRenderer) {
  //  for (int32_t i = 0; i < lights.size(); ++i) {
  //    const ViewLight& viewLight = lights[i];
  //    if (viewLight.Light) {
  //      assert(viewLight.ShaderBindingInstance);
  //      OutShaderBindingInstanceArray.Add(
  //          viewLight.ShaderBindingInstance.get());
  //    }
  //  }
  //}
}

// TODO: currently not used
void View::GetShaderBindingLayout(
    ShaderBindingLayoutArrayVk& OutShaderBindingsLayoutArray,
    bool                        InIsForwardRenderer ) const {
  OutShaderBindingsLayoutArray.Add(
      ViewUniformBufferShaderBindingInstance->ShaderBindingsLayouts);

  // Get light uniform buffers
  // if (InIsForwardRenderer) {
  //  for (int32_t i = 0; i < lights.size(); ++i) {
  //    const ViewLight& viewLight = lights[i];
  //    if (viewLight.Light) {
  //      assert(viewLight.ShaderBindingInstance);
  //      OutShaderBindingsLayoutArray.Add(
  //          viewLight.ShaderBindingInstance->ShaderBindingsLayouts);
  //    }
  //  }
  //}
}

}  // namespace game_engine
