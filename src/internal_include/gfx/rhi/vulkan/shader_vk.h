#ifndef GAME_ENGINE_SHADER_VK_H
#define GAME_ENGINE_SHADER_VK_H

#include "gfx/rhi/instant_struct.h"
#include "gfx/rhi/lock.h"
#include "gfx/rhi/name.h"
#include "gfx/rhi/shader.h"
#include "gfx/rhi/vulkan/rhi_type_vk.h"
#include "gfx/rhi/vulkan/spirv_util.h"

#include <vulkan/vulkan.h>

#include <filesystem>
#include <iterator>
#include <map>
#include <string>
#include <thread>
#include <vector>

namespace game_engine {

struct CompiledShaderVk: public CompiledShader {
  virtual ~CompiledShaderVk();

  VkPipelineShaderStageCreateInfo m_shaderStage_{};
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_VK_H
