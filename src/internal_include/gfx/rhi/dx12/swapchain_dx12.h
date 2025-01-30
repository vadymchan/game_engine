#ifndef GAME_ENGINE_SWAPCHAIN_DX12_H
#define GAME_ENGINE_SWAPCHAIN_DX12_H

#include "platform/windows/windows_platform_setup.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "gfx/rhi/swapchain.h"

#include <cassert>

namespace game_engine {

class SwapchainImageDx12 : public ISwapchainImage {
  public:
  virtual ~SwapchainImageDx12() { releaseInternal(); }

  virtual void release() override;

  void releaseInternal();

  uint64_t m_fenceValue_ = 0;
};

// Swapchain
class SwapchainDx12 : public ISwapchain {
  public:
  // ======= BEGIN: public destructor =========================================

  virtual ~SwapchainDx12() { releaseInternal(); }

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual bool create(const std::shared_ptr<Window>& window) override;
  virtual void release() override;

  virtual void* getHandle() const override { return m_swapChain_.Get(); }

  virtual ETextureFormat getFormat() const override { return m_format_; }

  virtual const math::Dimension2Di& getExtent() const override {
    return m_extent_;
  }

  virtual std::shared_ptr<ISwapchainImage> getSwapchainImage(
      int32_t index) const override {
    assert(m_images_.size() > index);
    return m_images_[index];
  }

  virtual int32_t getNumOfSwapchainImages() const override {
    return (int32_t)m_images_.size();
  }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public getters ============================================

  uint32_t getCurrentBackBufferIndex() const {
    return m_swapChain_->GetCurrentBackBufferIndex();
  }

  std::shared_ptr<ISwapchainImage> getCurrentSwapchainImage() const {
    return m_images_[getCurrentBackBufferIndex()];
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc methods =======================================

  void releaseInternal();

  bool resize(int32_t witdh, int32_t height);

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  ComPtr<IDXGISwapChain3> m_swapChain_;
  ETextureFormat          m_format_ = ETextureFormat::RGB8;
  math::Dimension2Di      m_extent_;
  std::vector<std::shared_ptr<SwapchainImageDx12>> m_images_;

  // ======= END: public misc fields   ========================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12

#endif  // GAME_ENGINE_SWAPCHAIN_DX12_H
