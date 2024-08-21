#include "gfx/rhi/render_target.h"

#include "gfx/rhi/render_target_pool.h"

namespace game_engine {

void RenderTarget::returnRt() {
  if (m_isCreatedFromRenderTargetPool_) {
    RenderTargetPool::s_seturnRenderTarget(this);
  }
}

void SceneRenderTarget::create(std::shared_ptr<Window> window,
                               const SwapchainImage*   swapchain) {
  constexpr EMSAASamples MsaaSamples         = EMSAASamples::COUNT_1;
  constexpr uint32_t     layerCount          = 1;
  constexpr bool         IsGenerateMipmap    = false;
  constexpr bool         IsUseAsSubpassInput = false;
  constexpr bool         IsMemoryless        = false;

  RenderTargetInfo ColorRTInfo = {
    ETextureType::TEXTURE_2D,
    ETextureFormat::R11G11B10F,  // VK_FORMAT_B10G11R11_UFLOAT_PACK32 for HDR
    window->getSize(),
    layerCount,
    IsGenerateMipmap,
    MsaaSamples,  // SampleCount
    IsUseAsSubpassInput,
    IsMemoryless,
    RTClearValue(0.0f, 0.0f, 0.0f, 1.0f)
    //, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT  // TODO: remove
  };
  // ColorRTInfo.ResourceName = Text("ColorPtr");
  m_colorPtr_ = RenderTargetPool::s_getRenderTarget(ColorRTInfo);

  RenderTargetInfo DepthRTInfo = {
    ETextureType::TEXTURE_2D,
    ETextureFormat::D24_S8,  // Assuming D24_S8 format
    window->getSize(),
    layerCount,
    IsGenerateMipmap,
    MsaaSamples,  // SampleCount
    IsUseAsSubpassInput,
    IsMemoryless,
    RTClearValue(1.0f, 0)
    //, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT  // TODO: remove
  };
  // DepthRTInfo.ResourceName = Text("DepthPtr");
  m_depthPtr_ = RenderTargetPool::s_getRenderTarget(DepthRTInfo);

  if ((int32_t)MsaaSamples > 1) {
    assert(swapchain);
    m_resolvePtr_ = RenderTarget::s_createFromTexture(swapchain->m_TexturePtr_);
  }

  if (!m_finalColorPtr_) {
    m_finalColorPtr_
        = RenderTarget::s_createFromTexture(swapchain->m_TexturePtr_);
  }
}

void SceneRenderTarget::returnRt() {
  if (m_colorPtr_) {
    m_colorPtr_->returnRt();
  }
  if (m_depthPtr_) {
    m_depthPtr_->returnRt();
  }
  if (m_resolvePtr_) {
    m_resolvePtr_->returnRt();
  }
}

}  // namespace game_engine