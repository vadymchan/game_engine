#ifndef GAME_ENGINE_SEMAPHORE_VK_H
#define GAME_ENGINE_SEMAPHORE_VK_H

#include "gfx/rhi/semaphore_manager.h"

#include <vulkan/vulkan.h>

#include <cassert>
#include <unordered_set>

namespace game_engine {

class SemaphoreVk : public jSemaphore {
  public:
  SemaphoreVk()
      : Semaphore(VK_NULL_HANDLE) {}

  virtual ~SemaphoreVk();

  virtual void* GetHandle() const { return reinterpret_cast<void*>(Semaphore); }

  // TODO
  // private:

  VkSemaphore Semaphore;
};

class SemaphoreManagerVk : public jSemaphoreManager {
  public:
  ~SemaphoreManagerVk() { Release(); }

  jSemaphore* GetOrCreateSemaphore() override;

  void ReturnSemaphore(jSemaphore* semaphore) override;

  void Release();

  private:
  std::unordered_set<jSemaphore*> UsingSemaphores;
  std::unordered_set<jSemaphore*> PendingSemaphores;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SEMAPHORE_VK_H
