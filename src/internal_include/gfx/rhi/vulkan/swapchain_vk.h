#ifndef GAME_ENGINE_SWAPCHAIN_VK_H
#define GAME_ENGINE_SWAPCHAIN_VK_H

#include "gfx/rhi/swapchain.h"
#include "gfx/rhi/vulkan/semaphore_vk.h"
#include "gfx/rhi/vulkan/texture_vk.h"
#include "gfx/rhi/vulkan/utils_vk.h"

#include <math_library/dimension.h>

#include <algorithm>

#include <vulkan/vulkan.hpp>

namespace game_engine {

class SwapchainImageVk : public SwapchainImage {
  public:
  virtual ~SwapchainImageVk() { ReleaseInternal(); }

  virtual void Release() override { ReleaseInternal(); }

  void ReleaseInternal();

  VkFence CommandBufferFence = nullptr;  // signal command buffer completion.

  // Semaphores are used to synchronize between GPUs. They allow multiple frames
  // to be created simultaneously.
  // Semaphores are designed for synchronizing commands within a command queue
  // or between queues.
  Semaphore* Available
      = nullptr;  // This is signaled (lock is released) when an image is
                  // acquired and rendering preparation is complete.
  Semaphore* RenderFinished
      = nullptr;  // This is signaled when rendering is finished and the image
                  // is ready for presentation.
  Semaphore* RenderFinishedAfterShadow
      = nullptr;  // This is signaled when rendering is finished after the
                  // shadow pass and the image is ready for presentation.
  Semaphore* RenderFinishedAfterBasePass
      = nullptr;  // This is signaled when rendering is finished after the base
                  // pass and the image is ready for presentation.

  // TODO: remove from texture
  /*std::shared_ptr<TextureVk> m_texture_;*/
};

class SwapchainVk : public Swapchain {
  public:
  bool Create(const std::shared_ptr<Window>& window) override;

  void Release() override { ReleaseInternal(); }

  void ReleaseInternal();

  void* GetHandle() const override { return m_swapChain_; }

  ETextureFormat GetFormat() const override { return Format; }

  const math::Dimension2Di& GetExtent() const override { return Extent; }

  SwapchainImageVk* GetSwapchainImage(int32_t index) const override {
    // Ensure index is within range
    assert(Images.size() > index);
    return Images[index];
  }

  int32_t GetNumOfSwapchainImages() const override {
    return static_cast<int32_t>(Images.size());
  }

  private:
  VkSwapchainKHR                 m_swapChain_ = nullptr;
  ETextureFormat                 Format       = ETextureFormat::RGB8;
  math::Dimension2Di             Extent;
  std::vector<SwapchainImageVk*> Images;

  bool isVSyncEnabled = {false};
};
}  // namespace game_engine

#endif  // GAME_ENGINE_SWAPCHAIN_VK_H
