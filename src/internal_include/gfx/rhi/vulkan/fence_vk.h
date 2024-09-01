#ifndef GAME_ENGINE_FENCE_VK_H
#define GAME_ENGINE_FENCE_VK_H

#include "gfx/rhi/fence_manager.h"

#include <vulkan/vulkan.h>

#include <cassert>
#include <unordered_set>

namespace game_engine {

// TODO: override IsComplete methods
class FenceVk : public IFence {
  public:
  // ======= BEGIN: public constructors =======================================

  FenceVk()
      : m_fence_(VK_NULL_HANDLE) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~FenceVk() override;

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  void waitForFence(uint64_t timeout = UINT64_MAX) override;

  virtual void* getHandle() const override {
    return reinterpret_cast<void*>(m_fence_);
  }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc methods =======================================

  // TODO: not used
  void resetFence() const;

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  VkFence m_fence_;

  // ======= END: public misc fields   ========================================
};

class FenceManagerVk : public IFenceManager {
  public:
  // ======= BEGIN: public destructor =========================================

  ~FenceManagerVk() { release(); }

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  void returnFence(IFence* fence) override;

  void release() override;

  IFence* getOrCreateFence() override;

  // ======= END: public overridden methods   =================================

  private:
  // ======= BEGIN: private misc fields =======================================

  std::unordered_set<IFence*> m_usingFences_;
  std::unordered_set<IFence*> m_pendingFences_;

  // ======= END: private misc fields   =======================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_FENCE_VK_H