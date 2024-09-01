#ifndef GAME_ENGINE_SHADER_BINDING_COMBINER_H
#define GAME_ENGINE_SHADER_BINDING_COMBINER_H

// TODO:
// - cosider renaming this file
// - cosider move this file to a more appropriate location

#include "gfx/rhi/resource_container.h"
#include "gfx/rhi/shader_binding_layout.h"

namespace game_engine {

struct ShaderBindingInstanceCombiner {
  // ======= BEGIN: public constants ==========================================

  const ShaderBindingInstanceArray* m_shaderBindingInstanceArray = nullptr;

  // ======= END: public constants   ==========================================

  // ======= BEGIN: public misc fields ========================================

  ResourceContainer<void*>    m_descriptorSetHandles_;
  ResourceContainer<uint32_t> m_dynamicOffsets_;

  // ======= END: public misc fields   ========================================
};
}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_BINDING_COMBINER_H