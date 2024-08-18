#include "gfx/rhi/render_target.h"
#include "gfx/rhi/render_target_pool.h"

namespace game_engine {

void RenderTarget::Return() {
  if (m_isCreatedFromRenderTargetPool_) {
    RenderTargetPool::ReturnRenderTarget(this);
  }
}


void SceneRenderTarget::Create(std::shared_ptr<Window> window,
                               const SwapchainImage* InSwapchain) {
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
  // ColorRTInfo.ResourceName = TEXT("ColorPtr");
  m_colorPtr_ = RenderTargetPool::GetRenderTarget(ColorRTInfo);

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
  // DepthRTInfo.ResourceName = TEXT("DepthPtr");
  m_depthPtr_ = RenderTargetPool::GetRenderTarget(DepthRTInfo);

  if ((int32_t)MsaaSamples > 1) {
    assert(InSwapchain);
    m_resolvePtr_ = RenderTarget::CreateFromTexture(InSwapchain->m_TexturePtr_);
  }

  if (!m_finalColorPtr_) {
    m_finalColorPtr_ = RenderTarget::CreateFromTexture(InSwapchain->m_TexturePtr_);
  }
}

void SceneRenderTarget::Return() {
  if (m_colorPtr_) {
    m_colorPtr_->Return();
  }
  if (m_depthPtr_) {
    m_depthPtr_->Return();
  }
  if (m_resolvePtr_) {
    m_resolvePtr_->Return();
  }
}

}  // namespace game_engine