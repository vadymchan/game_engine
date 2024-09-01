#ifndef GAME_ENGINE_I_UNIFORM_BUFFER_BLOCK_H
#define GAME_ENGINE_I_UNIFORM_BUFFER_BLOCK_H

#include "gfx/rhi/name.h"
#include "gfx/rhi/shader.h"
#include "gfx/rhi/shader_bindable_resource.h"

#include <cstdint>

namespace game_engine {

enum class LifeTimeType : uint8_t {
  OneFrame = 0,
  MultiFrame,
  MAX
};

struct IUniformBufferBlock : public ShaderBindableResource {
  // ======= BEGIN: public constructors =======================================

  IUniformBufferBlock() = default;

  IUniformBufferBlock(const Name& name, LifeTimeType lifeType)
      : ShaderBindableResource(name)
      , kLifeType(lifeType) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~IUniformBufferBlock() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void init(size_t size) = 0;
  virtual void release()         = 0;

  virtual void bind(const Shader* shader) const {}

  virtual void updateBufferData(const void* newData, size_t size) = 0;
  virtual void clearBuffer(int32_t clearValue = 0)                = 0;

  virtual bool isUseRingBuffer() const {
    return kLifeType == LifeTimeType::OneFrame;
  }

  virtual size_t getBufferSize() const { return 0; }

  virtual size_t getBufferOffset() const { return 0; }

  virtual void* getLowLevelResource() const { return nullptr; }

  virtual void* getLowLevelMemory() const { return nullptr; }  // Vulkan only

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public constants ==========================================

  const LifeTimeType kLifeType = LifeTimeType::MultiFrame;

  // ======= END: public constants   ==========================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_I_UNIFORM_BUFFER_BLOCK_H