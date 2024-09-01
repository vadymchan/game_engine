#ifndef GAME_ENGINE_UNIFORM_BUFFER_OBJECT_VK_H
#define GAME_ENGINE_UNIFORM_BUFFER_OBJECT_VK_H

#include "gfx/rhi/i_uniform_buffer_block.h"
#include "gfx/rhi/vulkan/buffer_vk.h"

#include <cstdint>

namespace game_engine {

// TODO: add interface IUniformBufferBlock
struct UniformBufferBlockVk : public IUniformBufferBlock {
  // ======= BEGIN: public aliases ============================================

  // TODO: consider whether this can be treated as alias
  using IUniformBufferBlock::IUniformBufferBlock;
  using IUniformBufferBlock::updateBufferData;

  // ======= END: public aliases   ============================================

  // ======= BEGIN: public destructor =========================================

  virtual ~UniformBufferBlockVk() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void init(size_t size) override;

  void allocBufferFromGlobalMemory(size_t size);

  virtual void release() override { m_buffer_.release(); }

  virtual void updateBufferData(const void* data, size_t size) override;

  virtual void clearBuffer(int32_t clearValue) override;

  virtual void* getLowLevelResource() const override {
    return m_buffer_.m_buffer_;
  }

  virtual void* getLowLevelMemory() const override {
    return m_buffer_.m_deviceMemory_;
  }

  virtual size_t getBufferSize() const override {
    return m_buffer_.m_allocatedSize_;
  }

  virtual size_t getBufferOffset() const override {
    return m_buffer_.m_offset_;
  }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  BufferVk m_buffer_;

  // ======= END: public misc fields   ========================================

  private:
  // ======= BEGIN: public constructors =======================================

  // TODO: this is probably need to be placed in public
  UniformBufferBlockVk(const UniformBufferBlockVk&) = delete;

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public overloaded operators ===============================

  UniformBufferBlockVk& operator=(const UniformBufferBlockVk&) = delete;

  // ======= END: public overloaded operators   ===============================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_UNIFORM_BUFFER_OBJECT_VK_H