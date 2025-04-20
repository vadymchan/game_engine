#ifndef GAME_ENGINE_BUFFER_H
#define GAME_ENGINE_BUFFER_H

#include "gfx/rhi/common/rhi_enums.h"
#include "gfx/rhi/common/rhi_types.h"

namespace game_engine {
namespace gfx {
namespace rhi {

/**
 * Represents a GPU buffer resource
 *
 * A buffer is a linear region of memory used for various purposes:
 * - Vertex buffers: Store vertex data
 * - Index buffers: Store index data
 * - Uniform/constant buffers: Store shader parameters
 * - Storage buffers: General purpose GPU-accessible memory
 */
class Buffer {
  public:
  Buffer(const BufferDesc& desc)
      : m_desc_(desc) {}

  virtual ~Buffer() = default;

  uint64_t getSize() const { return m_desc_.size; }

  BufferType getType() const { return m_desc_.type; }

  BufferCreateFlag getCreateFlags() const { return m_desc_.createFlags; }

  const BufferDesc& getDesc() const { return m_desc_; }

  protected:
  BufferDesc m_desc_;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_BUFFER_H