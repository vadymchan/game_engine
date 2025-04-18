#ifndef GAME_ENGINE_DESCRIPTOR_H
#define GAME_ENGINE_DESCRIPTOR_H

#include "gfx/rhi/rhi_new/common/rhi_enums.h"
#include "gfx/rhi/rhi_new/common/rhi_types.h"

namespace game_engine {
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

  /**
   * Bind a uniform buffer to a binding point
   *
   * @param binding The binding point index
   * @param buffer The buffer to bind
   * @param offset Offset within the buffer
   * @param range Size of the buffer range to bind (0 = entire buffer)
   */
  virtual void setUniformBuffer(uint32_t binding, Buffer* buffer, uint64_t offset = 0, uint64_t range = 0) = 0;

  /**
   * Bind a texture with a sampler to a binding point
   *
   * @param binding The binding point index
   * @param texture The texture to bind
   * @param sampler The sampler to use with the texture
   */
  virtual void setTextureSampler(uint32_t binding, Texture* texture, Sampler* sampler) = 0;

  /**
   * Bind a texture to a binding point
   *
   * @param binding The binding point index
   * @param texture The texture to bind
   * @param layout The layout the texture should be in when accessed
   */
  virtual void setTexture(uint32_t binding, Texture* texture, ResourceLayout layout = ResourceLayout::ShaderReadOnly)
      = 0;

  /**
   * Bind a sampler to a binding point
   *
   * @param binding The binding point index
   * @param sampler The sampler to bind
   */
  virtual void setSampler(uint32_t binding, Sampler* sampler) = 0;

  /**
   * Bind a storage buffer to a binding point
   *
   * @param binding The binding point index
   * @param buffer The buffer to bind
   * @param offset Offset within the buffer
   * @param range Size of the buffer range to bind (0 = entire buffer)
   */
  virtual void setStorageBuffer(uint32_t binding, Buffer* buffer, uint64_t offset = 0, uint64_t range = 0) = 0;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_DESCRIPTOR_H