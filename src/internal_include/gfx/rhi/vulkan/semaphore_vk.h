#ifndef GAME_ENGINE_SEMAPHORE_VK_H
#define GAME_ENGINE_SEMAPHORE_VK_H

#include "gfx/rhi/semaphore_manager.h"

#include <vulkan/vulkan.h>

#include <cassert>
#include <unordered_set>

namespace game_engine {

class SemaphoreVk : public ISemaphore {
  public:
  // ======= BEGIN: public constructors =======================================

  SemaphoreVk()
      : m_semaphore_(VK_NULL_HANDLE) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~SemaphoreVk();

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void* getHandle() const {
    return reinterpret_cast<void*>(m_semaphore_);
  }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  VkSemaphore m_semaphore_;

  // ======= END: public misc fields   ========================================
};

class SemaphoreManagerVk : public ISemaphoreManager {
  public:
  // ======= BEGIN: public destructor =========================================

  ~SemaphoreManagerVk() { release(); }

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  void        returnSemaphore(ISemaphore* semaphore) override;
  ISemaphore* getOrCreateSemaphore() override;

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc methods =======================================

  void release();

  // ======= END: public misc methods   =======================================

  private:
  // ======= BEGIN: private misc fields =======================================

  std::unordered_set<ISemaphore*> m_usingSemaphores_;
  std::unordered_set<ISemaphore*> m_pendingSemaphores_;

  // ======= END: private misc fields   =======================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SEMAPHORE_VK_H
