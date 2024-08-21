
//#include "gfx/rhi/vulkan/render_target_vk.h"
//
//#include "gfx/rhi/vulkan/render_target_pool_vk.h"
//#include "gfx/rhi/vulkan/rhi_vk.h"
//
//namespace game_engine {
//
//const RTClearValueVk RTClearValueVk::Invalid = RTClearValueVk();
//
//// ==================== RenderTargetVk =====================
//
//void RenderTargetVk::returnRt() {
//  if (CreatedFromRenderTargetPool) {
//    RenderTargetPoolVk::s_seturnRenderTarget(this);
//  }
//}

// ==================== SceneRenderTarget ( for Renderer) =====================

//void SceneRenderTarget::s_create(std::shared_ptr<Window> window,
//                               const SwapchainImageVk* InSwapchain) {
//  constexpr EMSAASamples MsaaSamples         = EMSAASamples::COUNT_1;
//  constexpr uint32_t     layerCount          = 1;
//  constexpr bool         IsGenerateMipmap    = false;
//  constexpr bool         IsUseAsSubpassInput = false;
//  constexpr bool         IsMemoryless        = false;
//
//  RenderTargetInfoVk ColorRTInfo = {
//    ETextureType::TEXTURE_2D,
//    ETextureFormat::R11G11B10F,  // VK_FORMAT_B10G11R11_UFLOAT_PACK32 for HDR
//    window->getSize(),
//    layerCount,
//    IsGenerateMipmap,
//    MsaaSamples,  // SampleCount
//    IsUseAsSubpassInput,
//    IsMemoryless,
//    RTClearValueVk(0.0f, 0.0f, 0.0f, 1.0f)
//    //, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT  // TODO: remove
//  };
//  // ColorRTInfo.ResourceName = Text("ColorPtr");
//  ColorPtr = RenderTargetPoolVk::s_getRenderTarget(ColorRTInfo);
//
//  RenderTargetInfoVk DepthRTInfo = {
//    ETextureType::TEXTURE_2D,
//    ETextureFormat::D24_S8,  // Assuming D24_S8 format
//    window->getSize(),
//    layerCount,
//    IsGenerateMipmap,
//    MsaaSamples,  // SampleCount
//    IsUseAsSubpassInput,
//    IsMemoryless,
//    RTClearValueVk(1.0f, 0)
//    //, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT  // TODO: remove
//  };
//  // DepthRTInfo.ResourceName = Text("DepthPtr");
//  DepthPtr = RenderTargetPoolVk::s_getRenderTarget(DepthRTInfo);
//
//  if ((int32_t)MsaaSamples > 1) {
//    assert(InSwapchain);
//    ResolvePtr = RenderTargetVk::s_createFromTexture(InSwapchain->m_texture_);
//  }
//
//  if (!FinalColorPtr) {
//    FinalColorPtr = RenderTargetVk::s_createFromTexture(InSwapchain->m_texture_);
//  }
//}
//
//void SceneRenderTarget::returnRt() {
//  if (ColorPtr) {
//    ColorPtr->returnRt();
//  }
//  if (DepthPtr) {
//    DepthPtr->returnRt();
//  }
//  if (ResolvePtr) {
//    ResolvePtr->returnRt();
//  }
//}

// }  // namespace game_engine
