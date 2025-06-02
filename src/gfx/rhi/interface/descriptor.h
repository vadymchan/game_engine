#ifndef ARISE_DESCRIPTOR_H
#define ARISE_DESCRIPTOR_H

#include "gfx/rhi/common/rhi_enums.h"
#include "gfx/rhi/common/rhi_types.h"

namespace arise {
namespace gfx {
namespace rhi {

class Buffer;
class Texture;
class Sampler;

/**
 * DescriptorSetLayout defines the layout of resources that can be accessed by shaders
 *
 * This class describes what types of resources (textures, buffers, etc.) are bound
 * at which binding points, and in which shader stages they are accessible.
 */
class DescriptorSetLayout {
  public:
  DescriptorSetLayout(const DescriptorSetLayoutDesc& desc)
      : m_desc_(desc) {}

  virtual ~DescriptorSetLayout() = default;

  const DescriptorSetLayoutDesc& getDesc() const { return m_desc_; }

  protected:
  DescriptorSetLayoutDesc m_desc_;
};

/**
 * DescriptorSet contains the actual resource bindings for shaders
 *
 * This class holds references to the resources (textures, buffers, samplers)
 * that are bound to specific binding points defined by the DescriptorSetLayout.
 */
class DescriptorSet {
  public:
  DescriptorSet()          = default;
  virtual ~DescriptorSet() = default;

  // clang-format off

  // bind resources to binding points
  virtual void setUniformBuffer(uint32_t binding, Buffer* buffer, uint64_t offset = 0, uint64_t range = 0) = 0;
  virtual void setTextureSampler(uint32_t binding, Texture* texture, Sampler* sampler) = 0;
  virtual void setTexture(uint32_t binding, Texture* texture, ResourceLayout layout = ResourceLayout::ShaderReadOnly) = 0;
  virtual void setSampler(uint32_t binding, Sampler* sampler) = 0;
  virtual void setStorageBuffer(uint32_t binding, Buffer* buffer, uint64_t offset = 0, uint64_t range = 0) = 0;

  // clang-format on

  virtual const DescriptorSetLayout* getLayout() const = 0;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace arise

#endif  // ARISE_DESCRIPTOR_H