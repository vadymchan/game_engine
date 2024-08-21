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
  virtual ~SwapchainImageVk() { releaseInternal(); }

  virtual void release() override { releaseInternal(); }

  void releaseInternal();

  VkFence m_commandBufferFence_ = nullptr;  // signal command buffer completion.

  // Semaphores are used to synchronize between GPUs. They allow multiple frames
  // to be created simultaneously.
  // Semaphores are designed for synchronizing commands within a command queue
  // or between queues.
  Semaphore* m_available_
      = nullptr;  // This is signaled (lock is released) when an image is
                  // acquired and rendering preparation is complete.
  Semaphore* m_renderFinished_
      = nullptr;  // This is signaled when rendering is finished and the image
                  // is ready for presentation.
  Semaphore* m_renderFinishedAfterShadow_
      = nullptr;  // This is signaled when rendering is finished after the
                  // shadow pass and the image is ready for presentation.
  Semaphore* m_renderFinishedAfterBasePass_
      = nullptr;  // This is signaled when rendering is finished after the base
                  // pass and the image is ready for presentation.

  // TODO: remove from texture
  /*std::shared_ptr<TextureVk> m_texture_;*/
};

class SwapchainVk : public Swapchain {
  public:
  bool create(const std::shared_ptr<Window>& window) override;

  void release() override { releaseInternal(); }

  void releaseInternal();

  void* getHandle() const override { return m_swapChain_; }

  ETextureFormat getFormat() const override { return m_format_; }

  const math::Dimension2Di& getExtent() const override { return m_extent_; }

  SwapchainImageVk* getSwapchainImage(int32_t index) const override {
    // Ensure index is within range
    assert(m_images_.size() > index);
    return m_images_[index];
  }

  int32_t getNumOfSwapchainImages() const override {
    return static_cast<int32_t>(m_images_.size());
  }

  private:
  VkSwapchainKHR                 m_swapChain_ = nullptr;
  ETextureFormat                 m_format_       = ETextureFormat::RGB8;
  math::Dimension2Di             m_extent_;
  std::vector<SwapchainImageVk*> m_images_;

  bool m_isVSyncEnabled_ = {false};
};
}  // namespace game_engine

#endif  // GAME_ENGINE_SWAPCHAIN_VK_H
