#ifndef GAME_ENGINE_COMMAND_BUFFER_MANAGER_H
#define GAME_ENGINE_COMMAND_BUFFER_MANAGER_H

#include "gfx/rhi/fence_manager.h"

namespace game_engine {

class jCommandBuffer {
  public:
  virtual ~jCommandBuffer() {}

  virtual void* GetNativeHandle() const { return nullptr; }

  virtual void* GetFenceHandle() const { return nullptr; }

  virtual void SetFence(void* fence) {}

  virtual bool Begin() const { return false; }

  virtual bool End() const { return false; }

  virtual void Reset() const {}

  virtual jFence* GetFence() const { return nullptr; }
};

class jCommandBufferManager {
  public:
  virtual ~jCommandBufferManager() {}

  virtual void Release() = 0;

  virtual jCommandBuffer* GetOrCreateCommandBuffer()              = 0;
  virtual void ReturnCommandBuffer(jCommandBuffer* commandBuffer) = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_COMMAND_BUFFER_MANAGER_H