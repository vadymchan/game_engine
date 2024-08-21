#ifndef GAME_ENGINE_UNIFORM_BUFFER_BLOCK_DX12_H
#define GAME_ENGINE_UNIFORM_BUFFER_BLOCK_DX12_H

#include "gfx/rhi/dx12/buffer_dx12.h"
#include "gfx/rhi/dx12/ring_buffer_dx12.h"
#include "gfx/rhi/i_uniform_buffer_block.h"

namespace game_engine {

struct UniformBufferBlockDx12 : public IUniformBufferBlock {
  friend struct PlacedResourcePool;

  using IUniformBufferBlock::IUniformBufferBlock;
  using IUniformBufferBlock::updateBufferData;
  virtual ~UniformBufferBlockDx12();

  virtual void init(size_t size) override;

  virtual void release() override;
  virtual void updateBufferData(const void* data, size_t size) override;

  virtual void clearBuffer(int32_t clearValue) override;

  virtual void* getLowLevelResource() const override;
  virtual void* getLowLevelMemory() const override;

  virtual size_t getBufferSize() const override;

  virtual size_t getBufferOffset() const override { return 0; }

  // TODO: consider if the naming is correct
  const DescriptorDx12& getCBV() const;
  uint64_t              getGPUAddress() const;

  private:
  UniformBufferBlockDx12(const UniformBufferBlockDx12&)            = delete;
  UniformBufferBlockDx12& operator=(const UniformBufferBlockDx12&) = delete;

  std::shared_ptr<BufferDx12> m_bufferPtr_;

  RingBufferDx12* m_ringBuffer_              = nullptr;
  int64_t         m_ringBufferOffset_        = 0;
  uint8_t*        m_ringBufferDestAddress_   = nullptr;
  size_t          m_ringBufferAllocatedSize_ = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_UNIFORM_BUFFER_BLOCK_DX12_H