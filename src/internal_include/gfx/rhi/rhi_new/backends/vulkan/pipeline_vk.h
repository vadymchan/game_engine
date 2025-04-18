#ifndef GAME_ENGINE_PIPELINE_VK_H
#define GAME_ENGINE_PIPELINE_VK_H

#include "gfx/rhi/rhi_new/interface/pipeline.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace game_engine {
namespace gfx {
namespace rhi {

class DeviceVk;
class ShaderVk;
class DescriptorSetLayoutVk;
class RenderPassVk;

class GraphicsPipelineVk : public GraphicsPipeline {
  public:
  GraphicsPipelineVk(const GraphicsPipelineDesc& desc, DeviceVk* device);
  ~GraphicsPipelineVk() override;

  GraphicsPipelineVk(const GraphicsPipelineVk&)            = delete;
  GraphicsPipelineVk& operator=(const GraphicsPipelineVk&) = delete;

  // Vulkan-specific methods
  VkPipeline getPipeline() const { return m_pipeline_; }

  VkPipelineLayout getPipelineLayout() const { return m_pipelineLayout_; }

  private:
  bool initialize_();

  bool createShaderStages_(std::vector<VkPipelineShaderStageCreateInfo>& shaderStages);
  bool createVertexInputState_(VkPipelineVertexInputStateCreateInfo& vertexInputInfo);
  bool createInputAssemblyState_(VkPipelineInputAssemblyStateCreateInfo& inputAssembly);
  bool createRasterizationState_(VkPipelineRasterizationStateCreateInfo& rasterizer);
  bool createMultisampleState_(VkPipelineMultisampleStateCreateInfo& multisampling);
  bool createDepthStencilState_(VkPipelineDepthStencilStateCreateInfo& depthStencil);
  bool createColorBlendState_(VkPipelineColorBlendStateCreateInfo& colorBlending);
  bool createPipelineLayout_();

  DeviceVk* m_device_;

  VkPipeline       m_pipeline_;
  VkPipelineLayout m_pipelineLayout_;

  // Native Vulkan descriptor set layouts for quick access during pipeline creation
  std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts_;

  // Resources owned by this pipeline
  std::vector<std::unique_ptr<DescriptorSetLayout>> m_ownedDescriptorSetLayouts_;
  std::vector<std::unique_ptr<Shader>>              m_ownedShaders_;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_PIPELINE_VK_H