#ifndef GAME_ENGINE_SEMAPHORE_VK_H
#define GAME_ENGINE_SEMAPHORE_VK_H

#include "gfx/rhi/semaphore_manager.h"

#include <vulkan/vulkan.h>

#include <cassert>
#include <unordered_set>

namespace game_engine {

class SemaphoreVk : public ISemaphore {
  public:
  SemaphoreVk()
      : m_semaphore_(VK_NULL_HANDLE) {}

  virtual ~SemaphoreVk();

  virtual void* getHandle() const { return reinterpret_cast<void*>(m_semaphore_); }

  // TODO
  // private:

  VkSemaphore m_semaphore_;
};

class SemaphoreManagerVk : public ISemaphoreManager {
  public:
  ~SemaphoreManagerVk() { release(); }

  ISemaphore* getOrCreateSemaphore() override;

  void returnSemaphore(ISemaphore* semaphore) override;

  void release();

  private:
  std::unordered_set<ISemaphore*> m_usingSemaphores_;
  std::unordered_set<ISemaphore*> m_pendingSemaphores_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SEMAPHORE_VK_H
