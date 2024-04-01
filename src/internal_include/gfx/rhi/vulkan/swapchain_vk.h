#ifndef GAME_ENGINE_SWAPCHAIN_VK_H
#define GAME_ENGINE_SWAPCHAIN_VK_H

#include "gfx/rhi/vulkan/texture_vk.h"
#include "gfx/rhi/vulkan/utils_vk.h"
#include "platform/common/window.h"
#include "gfx/rhi/vulkan/semaphore_vk.h"

#include <math_library/dimension.h>

#include <algorithm>

#include <vulkan/vulkan.hpp>

namespace game_engine {

class SwapchainImageVk {
  public:
  virtual ~SwapchainImageVk() { ReleaseInternal(); }

  virtual void Release() { ReleaseInternal(); }

  void ReleaseInternal();

  VkFence CommandBufferFence = nullptr;   // signal command buffer completion.

  SemaphoreVk* Available      = nullptr;
  SemaphoreVk* RenderFinished = nullptr;  // signal that rendering
                                          // is finished and the image is ready
                                          // for presentation.
  SemaphoreVk* RenderFinishedAfterShadow   = nullptr;
  SemaphoreVk* RenderFinishedAfterBasePass = nullptr;

  std::shared_ptr<TextureVk>
      m_texture_;  // TODO: sampler component not needed in swapchain image
};

class SwapchainVk {
  public:
  bool Create(const std::shared_ptr<Window>& window, VkSurfaceKHR surface);

  void Release() { ReleaseInternal(); }

  void ReleaseInternal();

  void* GetHandle() const { return m_swapChain_; }

  ETextureFormat GetFormat() const { return Format; }

  const math::Dimension2Di& GetExtent() const { return Extent; }

  SwapchainImageVk* GetSwapchainImage(int32_t index) const {
    // Ensure index is within range
    assert(Images.size() > index);
    return Images[index];
  }

  int32_t GetNumOfSwapchainImages() const {
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
