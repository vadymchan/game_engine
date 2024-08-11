#ifndef GAME_ENGINE_COMMAND_BUFFER_MANAGER_H
#define GAME_ENGINE_COMMAND_BUFFER_MANAGER_H

#include "gfx/rhi/fence_manager.h"

namespace game_engine {

class CommandBuffer {
  public:
  virtual ~CommandBuffer() {}

  virtual void* GetNativeHandle() const { return nullptr; }

  virtual void* GetFenceHandle() const { return nullptr; }

  virtual void SetFence(void* fence) {}

  virtual bool Begin() const { return false; }

  virtual bool End() const { return false; }

  virtual void Reset() const {}

  virtual Fence* GetFence() const { return nullptr; }
};

class CommandBufferManager {
  public:
  virtual ~CommandBufferManager() {}

  virtual void Release() = 0;

  virtual CommandBuffer* GetOrCreateCommandBuffer()              = 0;
  virtual void ReturnCommandBuffer(CommandBuffer* commandBuffer) = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_COMMAND_BUFFER_MANAGER_H