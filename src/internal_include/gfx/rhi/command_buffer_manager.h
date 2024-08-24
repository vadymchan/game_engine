#ifndef GAME_ENGINE_COMMAND_BUFFER_MANAGER_H
#define GAME_ENGINE_COMMAND_BUFFER_MANAGER_H

#include "gfx/rhi/fence_manager.h"

namespace game_engine {

class CommandBuffer {
  public:
  virtual ~CommandBuffer() {}

  virtual void* getNativeHandle() const { return nullptr; }

  virtual void* getFenceHandle() const { return nullptr; }

  virtual void setFence(void* fence) {}

  virtual bool begin() const { return false; }

  virtual bool end() const { return false; }

  virtual void reset() const {}

  virtual IFence* getFence() const { return nullptr; }
};

class ICommandBufferManager {
  public:
  virtual ~ICommandBufferManager() {}

  virtual void release() = 0;

  virtual CommandBuffer* getOrCreateCommandBuffer()              = 0;
  virtual void returnCommandBuffer(CommandBuffer* commandBuffer) = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_COMMAND_BUFFER_MANAGER_H