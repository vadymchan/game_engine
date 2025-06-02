#ifndef ARISE_SYNCHRONIZATION_VK_H
#define ARISE_SYNCHRONIZATION_VK_H

#include "gfx/rhi/interface/synchronization.h"

#include <vulkan/vulkan.h>

namespace arise {
namespace gfx {
namespace rhi {

class DeviceVk;

class FenceVk : public Fence {
  public:
  FenceVk(const FenceDesc& desc, DeviceVk* device);
  ~FenceVk() override;

  void reset() override;
  bool wait(uint64_t timeout = UINT64_MAX) override;
  bool isSignaled() override;

  // Vulkan-specific methods
  VkFence getFence() const { return m_fence_; }

  void signal(VkQueue queue);

  private:
  DeviceVk* m_device_ = nullptr;
  VkFence   m_fence_  = VK_NULL_HANDLE;
};

class SemaphoreVk : public Semaphore {
  public:
  SemaphoreVk(DeviceVk* device);
  ~SemaphoreVk() override;

  // Vulkan-specific methods
  VkSemaphore getSemaphore() const { return m_semaphore_; }

  /**
   * @note In Vulkan, semaphores are signaled/waited through queue operations. This method is utility
   *	   functions that may be used in simple cases. but normally semaphores are signaled through command buffer
   *       submissions or swapchain operations
   * @todo Consider remove if there is no need
   */
  void signal(VkQueue queue);

  private:
  DeviceVk*   m_device_    = nullptr;
  VkSemaphore m_semaphore_ = VK_NULL_HANDLE;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace arise

#endif  // ARISE_SYNCHRONIZATION_VK_H