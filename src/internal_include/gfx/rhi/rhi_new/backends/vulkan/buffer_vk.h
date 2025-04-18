#ifndef GAME_ENGINE_BUFFER_VK_H
#define GAME_ENGINE_BUFFER_VK_H

#include "gfx/rhi/rhi_new/interface/buffer.h"

#include <vulkan/vulkan.h>

namespace game_engine {
namespace gfx {
namespace rhi {

class DeviceVk;

class BufferVk : public Buffer {
  public:
  BufferVk(const BufferDesc& desc, DeviceVk* device);
  ~BufferVk() override;

  VkBuffer getBuffer() const { return m_buffer_; }

  uint32_t getStride() const { return m_stride_; }

  void setStride(uint32_t stride) { m_stride_ = stride; }

  VkDeviceMemory getDeviceMemory() const { return m_memory_; }

  bool isMapped() const { return m_isMapped_; }

  void* getMappedData() const { return m_mappedData_; }

  private:
  friend class DeviceVk;
  // These methods are called from DeviceVk
  bool update_(const void* data, size_t size, size_t offset);
  bool map_(void** ppData);
  void unmap_();

  bool createBuffer_(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

  VkBufferUsageFlags getBufferUsageFlags_() const;

  VkMemoryPropertyFlags getMemoryPropertyFlags_() const;

  uint32_t findMemoryType_(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

  DeviceVk*      m_device_     = nullptr;
  VkBuffer       m_buffer_     = VK_NULL_HANDLE;
  VkDeviceMemory m_memory_     = VK_NULL_HANDLE;
  uint32_t       m_stride_     = 0;        // For vertex buffers
  void*          m_mappedData_ = nullptr;  // For persistent mapping
  bool           m_isMapped_   = false;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_BUFFER_VK_H