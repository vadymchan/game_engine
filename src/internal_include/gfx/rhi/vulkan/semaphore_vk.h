#ifndef GAME_ENGINE_SEMAPHORE_VK_H
#define GAME_ENGINE_SEMAPHORE_VK_H

#include <vulkan/vulkan.h>

#include <cassert>
#include <unordered_set>

namespace game_engine {

class SemaphoreVk {
  public:
  SemaphoreVk()
      : Semaphore(VK_NULL_HANDLE) {}

  virtual ~SemaphoreVk();

  virtual void* GetHandle() const { return reinterpret_cast<void*>(Semaphore); }

  // TODO
  // private:

  VkSemaphore Semaphore;
};

class SemaphoreManagerVk {
  public:
  ~SemaphoreManagerVk() { Release(); }

  SemaphoreVk* GetOrCreateSemaphore();

  void ReturnSemaphore(SemaphoreVk* semaphore);

  void Release();

  private:
  std::unordered_set<SemaphoreVk*> UsingSemaphores;
  std::unordered_set<SemaphoreVk*> PendingSemaphores;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SEMAPHORE_VK_H
