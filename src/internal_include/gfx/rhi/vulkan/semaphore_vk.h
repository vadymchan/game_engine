#ifndef GAME_ENGINE_SEMAPHORE_VK_H
#define GAME_ENGINE_SEMAPHORE_VK_H

#include "gfx/rhi/semaphore_manager.h"

#include <vulkan/vulkan.h>

#include <cassert>
#include <unordered_set>

namespace game_engine {

class SemaphoreVk : public Semaphore {
  public:
  SemaphoreVk()
      : m_semaphore_(VK_NULL_HANDLE) {}

  virtual ~SemaphoreVk();

  virtual void* GetHandle() const { return reinterpret_cast<void*>(m_semaphore_); }

  // TODO
  // private:

  VkSemaphore m_semaphore_;
};

class SemaphoreManagerVk : public SemaphoreManager {
  public:
  ~SemaphoreManagerVk() { Release(); }

  Semaphore* GetOrCreateSemaphore() override;

  void ReturnSemaphore(Semaphore* semaphore) override;

  void Release();

  private:
  std::unordered_set<Semaphore*> UsingSemaphores;
  std::unordered_set<Semaphore*> PendingSemaphores;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SEMAPHORE_VK_H
