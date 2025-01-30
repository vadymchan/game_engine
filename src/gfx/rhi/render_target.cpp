#include "gfx/rhi/render_target.h"

#include "gfx/rhi/render_target_pool.h"

namespace game_engine {

void RenderTarget::returnRt() {
  if (m_isCreatedFromRenderTargetPool_) {
    RenderTargetPool::s_seturnRenderTarget(this);
  }
}

void SceneRenderTarget::create(
    // std::shared_ptr<Window>                 window,
    math::Dimension2Di                      dimensions,
    const std::shared_ptr<ISwapchainImage>& swapchain) {
  constexpr EMSAASamples MsaaSamples         = EMSAASamples::COUNT_1;
  constexpr uint32_t     layerCount          = 1;
  constexpr bool         IsGenerateMipmap    = false;
  constexpr bool         IsUseAsSubpassInput = false;
  constexpr bool         IsMemoryless        = false;

  RenderTargetInfo ColorRTInfo;
  ColorRTInfo.m_rype_                = ETextureType::TEXTURE_2D;
  ColorRTInfo.m_format_              = ETextureFormat::RGBA8;
  ColorRTInfo.m_extent_              = dimensions;
  ColorRTInfo.m_layerCount_          = layerCount;
  ColorRTInfo.m_isGenerateMipmap_    = IsGenerateMipmap;
  ColorRTInfo.m_sampleCount_         = MsaaSamples;
  ColorRTInfo.m_isUseAsSubpassInput_ = IsUseAsSubpassInput;
  ColorRTInfo.m_isMemoryless_        = IsMemoryless;
  ColorRTInfo.m_rtClearValue         = RtClearValue(0.0f, 0.0f, 0.0f, 1.0f);
  ColorRTInfo.m_textureCreateFlag_
      = ETextureCreateFlag::TransferSrc | ETextureCreateFlag::TransferDst;
  // ColorRTInfo.ResourceName = Text("ColorPtr");
  m_colorBuffer_ = RenderTargetPool::s_getRenderTarget(ColorRTInfo);

  RenderTargetInfo DepthRTInfo;
  DepthRTInfo.m_rype_                = ETextureType::TEXTURE_2D;
  DepthRTInfo.m_format_              = ETextureFormat::D24_S8;
  DepthRTInfo.m_extent_              = dimensions;
  DepthRTInfo.m_layerCount_          = layerCount;
  DepthRTInfo.m_isGenerateMipmap_    = IsGenerateMipmap;
  DepthRTInfo.m_sampleCount_         = MsaaSamples;
  DepthRTInfo.m_isUseAsSubpassInput_ = IsUseAsSubpassInput;
  DepthRTInfo.m_isMemoryless_        = IsMemoryless;
  DepthRTInfo.m_rtClearValue         = RtClearValue(1.0f, 0);
  DepthRTInfo.m_textureCreateFlag_
      = ETextureCreateFlag::TransferSrc | ETextureCreateFlag::TransferDst;
  // DepthRTInfo.ResourceName = Text("DepthPtr");
  m_depthBuffer_ = RenderTargetPool::s_getRenderTarget(DepthRTInfo);

  // if ((int32_t)MsaaSamples > 1) {
  //   assert(swapchain);
  //   m_resolvePtr_ =
  //   RenderTarget::s_createFromTexture(swapchain->m_texture_);
  // }

  if (!m_backBuffer_) {
    m_backBuffer_ = RenderTarget::s_createFromTexture(swapchain->m_texture_);
  }
}

void SceneRenderTarget::returnRt() {
  if (m_colorBuffer_) {
    m_colorBuffer_->returnRt();
  }
  if (m_depthBuffer_) {
    m_depthBuffer_->returnRt();
  }
  // if (m_resolvePtr_) {
  //   m_resolvePtr_->returnRt();
  // }
}

}  // namespace game_engine
