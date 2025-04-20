#ifndef GAME_ENGINE_SYNCHRONIZATION_H
#define GAME_ENGINE_SYNCHRONIZATION_H

#include "gfx/rhi/common/rhi_types.h"

#include <cstdint>

namespace game_engine {
namespace gfx {
namespace rhi {

/**
 * A Fence is a synchronization primitive that can be used to insert a dependency
 * from a queue to the host. It has two states - signaled and unsignaled.
 * Used for CPU-GPU synchronization.
 */
class Fence {
  public:
  Fence(const FenceDesc& desc)
      : m_signaled_(desc.signaled) {}

  virtual ~Fence() = default;

  virtual void reset() = 0;

  virtual bool wait(uint64_t timeout = UINT64_MAX) = 0;

  virtual bool isSignaled() = 0;

  protected:
  bool m_signaled_ = false;
};

/**
 * A Semaphore is a synchronization primitive that can be used to insert a dependency
 * between queue operations (GPU-GPU synchronization) or between a queue and the swap chain.
 */
class Semaphore {
  public:
  Semaphore()          = default;
  virtual ~Semaphore() = default;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_SYNCHRONIZATION_H