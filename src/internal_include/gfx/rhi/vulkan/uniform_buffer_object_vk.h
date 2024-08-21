#ifndef GAME_ENGINE_UNIFORM_BUFFER_OBJECT_VK_H
#define GAME_ENGINE_UNIFORM_BUFFER_OBJECT_VK_H

#include "gfx/rhi/i_uniform_buffer_block.h"
#include "gfx/rhi/vulkan/buffer_vk.h"

#include <cstdint>

namespace game_engine {

// TODO: add interface IUniformBufferBlock
struct UniformBufferBlockVk : public IUniformBufferBlock {
  using IUniformBufferBlock::IUniformBufferBlock;
  using IUniformBufferBlock::updateBufferData;

  virtual ~UniformBufferBlockVk() {}

  virtual void init(size_t size) override;

  void allocBufferFromGlobalMemory(size_t size);

  virtual void release() override { m_buffer_.release(); }

  virtual void updateBufferData(const void* data, size_t size) override;

  virtual void clearBuffer(int32_t clearValue) override;

  virtual void* getLowLevelResource() const override { return m_buffer_.m_buffer_; }

  virtual void* getLowLevelMemory() const override { return m_buffer_.m_deviceMemory_; }

  virtual size_t getBufferSize() const override { return m_buffer_.m_allocatedSize_; }

  virtual size_t getBufferOffset() const override { return m_buffer_.m_offset_; }

  BufferVk m_buffer_;

  private:
  UniformBufferBlockVk(const UniformBufferBlockVk&)            = delete;
  UniformBufferBlockVk& operator=(const UniformBufferBlockVk&) = delete;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_UNIFORM_BUFFER_OBJECT_VK_H