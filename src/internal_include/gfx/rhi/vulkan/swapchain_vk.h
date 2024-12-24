#ifndef GAME_ENGINE_SWAPCHAIN_VK_H
#define GAME_ENGINE_SWAPCHAIN_VK_H

#include "gfx/rhi/swapchain.h"
#include "gfx/rhi/vulkan/semaphore_vk.h"
#include "gfx/rhi/vulkan/texture_vk.h"
#include "gfx/rhi/vulkan/utils_vk.h"

#include <math_library/dimension.h>

#include <algorithm>

// #include <vulkan/vulkan.hpp>

namespace game_engine {

class SwapchainImageVk : public ISwapchainImage {
  public:
  // ======= BEGIN: public destructor =========================================

  virtual ~SwapchainImageVk() { releaseInternal(); }

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void release() override { releaseInternal(); }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc methods =======================================

  void releaseInternal();

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  VkFence m_commandBufferFence_ = nullptr;  // signal command buffer completion.

  // Semaphores are used to synchronize between GPUs. They allow multiple frames
  // to be created simultaneously.
  // Semaphores are designed for synchronizing commands within a command queue
  // or between queues.
  ISemaphore* m_available_
      = nullptr;  // This is signaled (lock is released) when an image is
                  // acquired and rendering preparation is complete.
  ISemaphore* m_renderFinished_
      = nullptr;  // This is signaled when rendering is finished and the image
                  // is ready for presentation.
  ISemaphore* m_renderFinishedAfterShadow_
      = nullptr;  // This is signaled when rendering is finished after the
                  // shadow pass and the image is ready for presentation.
  ISemaphore* m_renderFinishedAfterBasePass_
      = nullptr;  // This is signaled when rendering is finished after the base
                  // pass and the image is ready for presentation.

  // TODO: remove from texture
  /*std::shared_ptr<TextureVk> m_texture_;*/

  // ======= END: public misc fields   ========================================
};

class SwapchainVk : public ISwapchain {
  public:
  // ======= BEGIN: public overridden methods =================================

  bool create(const std::shared_ptr<Window>& window) override;

  void release() override { releaseInternal(); }

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

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc methods =======================================

  void releaseInternal();

  // ======= END: public misc methods   =======================================

  private:
  // ======= BEGIN: private misc fields =======================================

  VkSwapchainKHR                 m_swapChain_ = nullptr;
  ETextureFormat                 m_format_    = ETextureFormat::RGB8;
  math::Dimension2Di             m_extent_;
  std::vector<SwapchainImageVk*> m_images_;

  bool m_isVSyncEnabled_ = {false};

  // ======= END: private misc fields   =======================================
};
}  // namespace game_engine

#endif  // GAME_ENGINE_SWAPCHAIN_VK_H
