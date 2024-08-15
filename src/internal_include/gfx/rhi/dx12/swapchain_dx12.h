#ifndef GAME_ENGINE_SWAPCHAIN_DX12_H
#define GAME_ENGINE_SWAPCHAIN_DX12_H

#include "gfx/rhi/swapchain.h"
#include "platform/windows/windows_platform_setup.h"

#include <cassert>

namespace game_engine {

	class SwapchainImageDx12 : public SwapchainImage {
  public:
  virtual ~SwapchainImageDx12() { ReleaseInternal(); }

  void         ReleaseInternal();
  virtual void Release() override;

  uint64_t FenceValue = 0;
};

// Swapchain
class SwapchainDx12 : public Swapchain {
  public:
  virtual ~SwapchainDx12() { ReleaseInternal(); }

  void ReleaseInternal();

  virtual bool Create(const std::shared_ptr<Window>& window) override;
  virtual void Release() override;

  virtual void* GetHandle() const override { return SwapChain.Get(); }

  virtual ETextureFormat GetFormat() const override { return Format; }

  virtual const math::Dimension2Di& GetExtent() const override {
    return Extent;
  }

  virtual SwapchainImage* GetSwapchainImage(int32_t index) const override {
    assert(Images.size() > index);
    return Images[index];
  }

  virtual int32_t GetNumOfSwapchainImages() const override {
    return (int32_t)Images.size();
  }

  bool Resize(int32_t InWidth, int32_t InHeight);

  uint32_t GetCurrentBackBufferIndex() const {
    return SwapChain->GetCurrentBackBufferIndex();
  }

  SwapchainImage* GetCurrentSwapchainImage() const {
    return Images[GetCurrentBackBufferIndex()];
  }

  ComPtr<IDXGISwapChain3>            SwapChain;
  ETextureFormat                     Format = ETextureFormat::RGB8;
  math::Dimension2Di                 Extent;
  std::vector<SwapchainImageDx12*> Images;
};

} // namespace game_engine

#endif // GAME_ENGINE_SWAPCHAIN_DX12_H