#ifndef GAME_ENGINE_UNIFORM_BUFFER_BLOCK_DX12_H
#define GAME_ENGINE_UNIFORM_BUFFER_BLOCK_DX12_H

#include "gfx/rhi/dx12/buffer_dx12.h"
#include "gfx/rhi/dx12/ring_buffer_dx12.h"
#include "gfx/rhi/i_uniform_buffer_block.h"

namespace game_engine {

struct UniformBufferBlockDx12 : public IUniformBufferBlock {
  friend struct PlacedResourcePool;

  using IUniformBufferBlock::IUniformBufferBlock;
  using IUniformBufferBlock::UpdateBufferData;
  virtual ~UniformBufferBlockDx12();

  virtual void Init(size_t size) override;

  virtual void Release() override;
  virtual void UpdateBufferData(const void* data, size_t size) override;

  virtual void ClearBuffer(int32_t clearValue) override;

  virtual void* GetLowLevelResource() const override;
  virtual void* GetLowLevelMemory() const override;

  virtual size_t GetBufferSize() const override;

  virtual size_t GetBufferOffset() const override { return 0; }

  const DescriptorDx12& GetCBV() const;
  uint64_t              GetGPUAddress() const;

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