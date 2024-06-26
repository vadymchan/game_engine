#ifndef GAME_ENGINE_SHADER_BINDING_COMBINER_H
#define GAME_ENGINE_SHADER_BINDING_COMBINER_H

// TODO:
// - cosider renaming this file
// - cosider move this file to a more appropriate location 

#include "gfx/rhi/resource_container.h"
#include "gfx/rhi/vulkan/shader_binding_layout_vk.h"

namespace game_engine {

struct ShaderBindingInstanceCombiner {
  const ShaderBindingInstanceArray* shaderBindingInstanceArray = nullptr;

  ResourceContainer<void*>    DescriptorSetHandles;
  ResourceContainer<uint32_t> DynamicOffsets;
};
}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_BINDING_COMBINER_H