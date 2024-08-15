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

  VkPipelineShaderStageCreateInfo ShaderStage{};
};

// old version
// class Shader {
//  public:
//  Shader(VkShaderStageFlagBits        stage,
//           const std::filesystem::path& shaderFilePath,
//           const std::string&           entryPoint = "main")
//      : m_stage(stage)
//      , m_entryPoint(entryPoint) {
//    auto spirvCode = SpirvUtil::compileHlslFileToSpirv(
//        shaderFilePath, determineShaderKind(stage).value(), entryPoint);
//    createShaderModule(spirvCode);
//  }
//
//  ~Shader() {
//    if (m_shaderModule != VK_NULL_HANDLE) {
//      vkDestroyShaderModule(g_rhi_vk->m_device_, m_shaderModule, nullptr);
//    }
//  }
//
//  VkShaderStageFlagBits getStage() const { return m_stage; }
//
//  VkShaderModule getShaderModule() const { return m_shaderModule; }
//
//  const std::string& getEntryPoint() const { return m_entryPoint; }
//
//  VkPipelineShaderStageCreateInfo getShaderStageInfo() const {
//    return m_shaderStageInfo;
//  }
//
//  private:
//  void createShaderModule(const std::vector<uint32_t>& spirvCode) {
//    VkShaderModuleCreateInfo createInfo{};
//    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
//    createInfo.codeSize = spirvCode.size() * sizeof(uint32_t);
//    createInfo.pCode    = spirvCode.data();
//
//    if (vkCreateShaderModule(
//            g_rhi_vk->m_device_, &createInfo, nullptr, &m_shaderModule)
//        != VK_SUCCESS) {
//      // TODO: log error
//    }
//  }
//
//  static std::optional<shaderc_shader_kind> determineShaderKind(
//      VkShaderStageFlagBits stage) {
//    switch (stage) {
//      case VK_SHADER_STAGE_VERTEX_BIT:
//        return shaderc_vertex_shader;
//      case VK_SHADER_STAGE_FRAGMENT_BIT:
//        return shaderc_fragment_shader;
//      case VK_SHADER_STAGE_GEOMETRY_BIT:
//        return shaderc_geometry_shader;
//      case VK_SHADER_STAGE_COMPUTE_BIT:
//        return shaderc_compute_shader;
//      case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
//        return shaderc_raygen_shader;
//      case VK_SHADER_STAGE_MISS_BIT_KHR:
//        return shaderc_miss_shader;
//      case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
//        return shaderc_closesthit_shader;
//      case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
//        return shaderc_anyhit_shader;
//      default:
//        // Log error or handle unsupported stages
//        return std::nullopt;
//    }
//  }
//
//  VkShaderStageFlagBits m_stage;
//
//  std::string    m_entryPoint;
//  VkShaderModule m_shaderModule = VK_NULL_HANDLE;
//
//  // Compiler shader
//  VkPipelineShaderStageCreateInfo m_shaderStageInfo{};
//};

}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_VK_H
