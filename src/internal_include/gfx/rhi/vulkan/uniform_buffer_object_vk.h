#ifndef GAME_ENGINE_UNIFORM_BUFFER_OBJECT_VK_H
#define GAME_ENGINE_UNIFORM_BUFFER_OBJECT_VK_H

#include "gfx/rhi/i_uniform_buffer_block.h"
#include "gfx/rhi/vulkan/buffer_vk.h"

#include <cstdint>

namespace game_engine {

// TODO: add interface IUniformBufferBlock
struct UniformBufferBlockVk : public IUniformBufferBlock {
  using IUniformBufferBlock::IUniformBufferBlock;
  using IUniformBufferBlock::UpdateBufferData;

  virtual ~UniformBufferBlockVk() {}

  virtual void Init(size_t size) override;

  void AllocBufferFromGlobalMemory(size_t size);

  virtual void Release() override { Buffer.Release(); }

  virtual void UpdateBufferData(const void* InData, size_t InSize) override;

  virtual void ClearBuffer(int32_t clearValue) override;

  virtual void* GetLowLevelResource() const override { return Buffer.m_buffer; }

  virtual void* GetLowLevelMemory() const override { return Buffer.m_memory; }

  virtual size_t GetBufferSize() const override { return Buffer.AllocatedSize; }

  virtual size_t GetBufferOffset() const override { return Buffer.Offset; }

  BufferVk Buffer;

  private:
  UniformBufferBlockVk(const UniformBufferBlockVk&)            = delete;
  UniformBufferBlockVk& operator=(const UniformBufferBlockVk&) = delete;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_UNIFORM_BUFFER_OBJECT_VK_H