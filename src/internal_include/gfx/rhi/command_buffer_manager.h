#ifndef GAME_ENGINE_COMMAND_BUFFER_MANAGER_H
#define GAME_ENGINE_COMMAND_BUFFER_MANAGER_H

#include "gfx/rhi/fence_manager.h"

#include <memory>

namespace game_engine {

class CommandBuffer {
  public:
  // ======= BEGIN: public destructor =========================================

  virtual ~CommandBuffer() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual bool begin() const { return false; }

  virtual bool end() const { return false; }

  virtual void reset() const {}

  virtual IFence* getFence() const { return nullptr; }

  virtual void* getNativeHandle() const { return nullptr; }

  virtual void* getFenceHandle() const { return nullptr; }

  virtual void setFence(void* fence) {}

  // ======= END: public overridden methods   =================================
};

class ICommandBufferManager {
  public:
  // ======= BEGIN: public destructor =========================================

  virtual ~ICommandBufferManager() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void release() = 0;

  virtual void returnCommandBuffer(std::shared_ptr<CommandBuffer> commandBuffer)
      = 0;
  virtual std::shared_ptr<CommandBuffer> getOrCreateCommandBuffer() = 0;

  // ======= END: public overridden methods   =================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_COMMAND_BUFFER_MANAGER_H
