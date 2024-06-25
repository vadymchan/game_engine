#include "gfx/rhi/render_target.h"
#include "gfx/rhi/render_target_pool.h"

namespace game_engine {

void jRenderTarget::Return() {
  if (bCreatedFromRenderTargetPool) {
    jRenderTargetPool::ReturnRenderTarget(this);
  }
}


void SceneRenderTarget::Create(std::shared_ptr<Window> window,
                               const jSwapchainImage* InSwapchain) {
  constexpr EMSAASamples MsaaSamples         = EMSAASamples::COUNT_1;
  constexpr uint32_t     layerCount          = 1;
  constexpr bool         IsGenerateMipmap    = false;
  constexpr bool         IsUseAsSubpassInput = false;
  constexpr bool         IsMemoryless        = false;

  jRenderTargetInfo ColorRTInfo = {
    ETextureType::TEXTURE_2D,
    ETextureFormat::R11G11B10F,  // VK_FORMAT_B10G11R11_UFLOAT_PACK32 for HDR
    window->getSize(),
    layerCount,
    IsGenerateMipmap,
    MsaaSamples,  // SampleCount
    IsUseAsSubpassInput,
    IsMemoryless,
    jRTClearValue(0.0f, 0.0f, 0.0f, 1.0f)
    //, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT  // TODO: remove
  };
  // ColorRTInfo.ResourceName = TEXT("ColorPtr");
  ColorPtr = jRenderTargetPool::GetRenderTarget(ColorRTInfo);

  jRenderTargetInfo DepthRTInfo = {
    ETextureType::TEXTURE_2D,
    ETextureFormat::D24_S8,  // Assuming D24_S8 format
    window->getSize(),
    layerCount,
    IsGenerateMipmap,
    MsaaSamples,  // SampleCount
    IsUseAsSubpassInput,
    IsMemoryless,
    jRTClearValue(1.0f, 0)
    //, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT  // TODO: remove
  };
  // DepthRTInfo.ResourceName = TEXT("DepthPtr");
  DepthPtr = jRenderTargetPool::GetRenderTarget(DepthRTInfo);

  if ((int32_t)MsaaSamples > 1) {
    assert(InSwapchain);
    ResolvePtr = jRenderTarget::CreateFromTexture(InSwapchain->TexturePtr);
  }

  if (!FinalColorPtr) {
    FinalColorPtr = jRenderTarget::CreateFromTexture(InSwapchain->TexturePtr);
  }
}

void SceneRenderTarget::Return() {
  if (ColorPtr) {
    ColorPtr->Return();
  }
  if (DepthPtr) {
    DepthPtr->Return();
  }
  if (ResolvePtr) {
    ResolvePtr->Return();
  }
}

}  // namespace game_engine