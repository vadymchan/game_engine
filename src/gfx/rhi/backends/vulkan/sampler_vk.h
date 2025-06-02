#ifndef ARISE_SAMPLER_VK_H
#define ARISE_SAMPLER_VK_H

#include "gfx/rhi/interface/sampler.h"

#include <vulkan/vulkan.h>

namespace arise {
namespace gfx {
namespace rhi {

class DeviceVk;

class SamplerVk : public Sampler {
  public:
  SamplerVk(const SamplerDesc& desc, DeviceVk* device);
  ~SamplerVk() override;

  SamplerVk(const SamplerVk&)            = delete;
  SamplerVk& operator=(const SamplerVk&) = delete;

  // Vulkan-specific methods
  VkSampler getSampler() const { return m_sampler_; }

  private:
  DeviceVk* m_device_;
  VkSampler m_sampler_ = VK_NULL_HANDLE;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace arise

#endif  // ARISE_SAMPLER_VK_H