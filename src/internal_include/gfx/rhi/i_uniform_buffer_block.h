#ifndef GAME_ENGINE_I_UNIFORM_BUFFER_BLOCK_H
#define GAME_ENGINE_I_UNIFORM_BUFFER_BLOCK_H

#include "gfx/rhi/shader_bindable_resource.h"
#include "gfx/rhi/name.h"
#include "gfx/rhi/shader.h"

#include <cstdint>

namespace game_engine {

enum class LifeTimeType : uint8_t {
  OneFrame = 0,
  MultiFrame,
  MAX
};

struct IUniformBufferBlock : public ShaderBindableResource {
  IUniformBufferBlock() = default;

  IUniformBufferBlock(const Name& InName, LifeTimeType InLifeType)
      : ShaderBindableResource(InName)
      , LifeType(InLifeType) {}

  virtual ~IUniformBufferBlock() {}

  virtual bool IsUseRingBuffer() const {
    return LifeType == LifeTimeType::OneFrame;
  }

  virtual size_t GetBufferSize() const { return 0; }

  virtual size_t GetBufferOffset() const { return 0; }

  virtual void Init(size_t size) = 0;
  virtual void Release()         = 0;

  virtual void Bind(const Shader* shader) const {}

  virtual void UpdateBufferData(const void* newData, size_t size) = 0;
  virtual void ClearBuffer(int32_t clearValue = 0)                  = 0;

  virtual void* GetLowLevelResource() const { return nullptr; }

  virtual void* GetLowLevelMemory() const { return nullptr; }  // Vulkan only

  const LifeTimeType LifeType = LifeTimeType::MultiFrame;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_I_UNIFORM_BUFFER_BLOCK_H