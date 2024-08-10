#ifndef GAME_ENGINE_UNIFORM_BUFFER_BLOCK_DX12_H
#define GAME_ENGINE_UNIFORM_BUFFER_BLOCK_DX12_H

#include "gfx/rhi/i_uniform_buffer_block.h"
#include "gfx/rhi/dx12/buffer_dx12.h"
#include "gfx/rhi/dx12/ring_buffer_dx12.h"

namespace game_engine {

struct jUniformBufferBlock_DX12 : public IUniformBufferBlock {
  friend struct jPlacedResourcePool;

  using IUniformBufferBlock::IUniformBufferBlock;
  using IUniformBufferBlock::UpdateBufferData;
  virtual ~jUniformBufferBlock_DX12();

  virtual void Init(size_t size) override;

  virtual void Release() override;
  virtual void UpdateBufferData(const void* InData, size_t InSize) override;

  virtual void ClearBuffer(int32_t clearValue) override;

  virtual void* GetLowLevelResource() const override;
  virtual void* GetLowLevelMemory() const override;

  virtual size_t GetBufferSize() const override;

  virtual size_t GetBufferOffset() const override { return 0; }

  const jDescriptor_DX12& GetCBV() const;
  uint64_t                  GetGPUAddress() const;

  private:
  jUniformBufferBlock_DX12(const jUniformBufferBlock_DX12&)            = delete;
  jUniformBufferBlock_DX12& operator=(const jUniformBufferBlock_DX12&) = delete;

  std::shared_ptr<jBuffer_DX12> BufferPtr;

  jRingBuffer_DX12* RingBuffer              = nullptr;
  int64_t             RingBufferOffset        = 0;
  uint8_t*            RingBufferDestAddress   = nullptr;
  size_t            RingBufferAllocatedSize = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_UNIFORM_BUFFER_BLOCK_DX12_H