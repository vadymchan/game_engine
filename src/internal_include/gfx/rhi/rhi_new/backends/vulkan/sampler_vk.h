#ifndef GAME_ENGINE_SAMPLER_VK_H
#define GAME_ENGINE_SAMPLER_VK_H

#include "gfx/rhi/rhi_new/interface/sampler.h"

#include <vulkan/vulkan.h>

namespace game_engine {
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
}  // namespace game_engine

#endif  // GAME_ENGINE_SAMPLER_VK_H