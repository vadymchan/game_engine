#ifndef GAME_ENGINE_UNIFORM_BUFFER_BLOCK_DX12_H
#define GAME_ENGINE_UNIFORM_BUFFER_BLOCK_DX12_H

#include "gfx/rhi/dx12/buffer_dx12.h"
#include "gfx/rhi/dx12/ring_buffer_dx12.h"
#include "gfx/rhi/i_uniform_buffer_block.h"

namespace game_engine {

struct UniformBufferBlockDx12 : public IUniformBufferBlock {
  // ======= BEGIN: public aliases ============================================

  using IUniformBufferBlock::IUniformBufferBlock;
  using IUniformBufferBlock::updateBufferData;

  // ======= END: public aliases   ============================================

  // ======= BEGIN: public destructor =========================================

  virtual ~UniformBufferBlockDx12();

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void init(size_t size) override;

  virtual void release() override;
  virtual void updateBufferData(const void* data, size_t size) override;

  virtual void clearBuffer(int32_t clearValue) override;

  virtual void* getLowLevelResource() const override;
  virtual void* getLowLevelMemory() const override;

  virtual size_t getBufferSize() const override;

  virtual size_t getBufferOffset() const override { return 0; }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public getters ============================================

  // TODO: consider if the naming is correct
  const DescriptorDx12& getCBV() const;
  uint64_t              getGPUAddress() const;

  // ======= END: public getters   ============================================

  private:
  // ======= BEGIN: private friend classes ====================================

  friend struct PlacedResourcePool;

  // ======= END: private friend classes   ====================================

  // ======= BEGIN: private constructors ======================================

  UniformBufferBlockDx12(const UniformBufferBlockDx12&) = delete;

  // ======= END: private constructors   ======================================

  // ======= BEGIN: private overloaded operators ==============================

  UniformBufferBlockDx12& operator=(const UniformBufferBlockDx12&) = delete;

  // ======= END: private overloaded operators   ==============================

  // ======= BEGIN: private misc fields =======================================

  std::shared_ptr<BufferDx12> m_bufferPtr_;

  RingBufferDx12* m_ringBuffer_              = nullptr;
  int64_t         m_ringBufferOffset_        = 0;
  uint8_t*        m_ringBufferDestAddress_   = nullptr;
  size_t          m_ringBufferAllocatedSize_ = 0;

  // ======= END: private misc fields   =======================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_UNIFORM_BUFFER_BLOCK_DX12_H