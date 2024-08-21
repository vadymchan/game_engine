#ifndef GAME_ENGINE_SWAPCHAIN_DX12_H
#define GAME_ENGINE_SWAPCHAIN_DX12_H

#include "gfx/rhi/swapchain.h"
#include "platform/windows/windows_platform_setup.h"

#include <cassert>

namespace game_engine {

class SwapchainImageDx12 : public SwapchainImage {
  public:
  virtual ~SwapchainImageDx12() { releaseInternal(); }

  void         releaseInternal();
  virtual void release() override;

  uint64_t m_fenceValue_ = 0;
};

// Swapchain
class SwapchainDx12 : public Swapchain {
  public:
  virtual ~SwapchainDx12() { releaseInternal(); }

  void releaseInternal();

  virtual bool create(const std::shared_ptr<Window>& window) override;
  virtual void release() override;

  virtual void* getHandle() const override { return m_swapChain_.Get(); }

  virtual ETextureFormat getFormat() const override { return m_format_; }

  virtual const math::Dimension2Di& getExtent() const override {
    return m_extent_;
  }

  virtual SwapchainImage* getSwapchainImage(int32_t index) const override {
    assert(m_images_.size() > index);
    return m_images_[index];
  }

  virtual int32_t getNumOfSwapchainImages() const override {
    return (int32_t)m_images_.size();
  }

  bool resize(int32_t witdh, int32_t height);

  uint32_t getCurrentBackBufferIndex() const {
    return m_swapChain_->GetCurrentBackBufferIndex();
  }

  SwapchainImage* getCurrentSwapchainImage() const {
    return m_images_[getCurrentBackBufferIndex()];
  }

  ComPtr<IDXGISwapChain3>          m_swapChain_;
  ETextureFormat                   m_format_ = ETextureFormat::RGB8;
  math::Dimension2Di               m_extent_;
  std::vector<SwapchainImageDx12*> m_images_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SWAPCHAIN_DX12_H