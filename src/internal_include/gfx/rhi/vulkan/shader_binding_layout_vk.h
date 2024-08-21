#ifndef GAME_ENGINE_SHADER_BINDING_LAYOUT_VK_H
#define GAME_ENGINE_SHADER_BINDING_LAYOUT_VK_H

#include "gfx/rhi/i_uniform_buffer_block.h"
#include "gfx/rhi/lock.h"
#include "gfx/rhi/resource_container.h"
#include "gfx/rhi/shader_binding_layout.h"
#include "gfx/rhi/vulkan/buffer_vk.h"
// #include "gfx/rhi/vulkan/pipeline_state_info_vk.h" // circular dependency
#include "gfx/rhi/pipeline_state_info.h" // circular dependency
#include "gfx/rhi/vulkan/texture_vk.h"
#include "gfx/rhi/vulkan/uniform_buffer_object_vk.h"
#include "utils/third_party/xxhash_util.h"

#include <vulkan/vulkan.h>

#include <cassert>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace game_engine {

struct WriteDescriptorInfo {
  WriteDescriptorInfo() = default;

  WriteDescriptorInfo(VkDescriptorBufferInfo bufferInfo)
      : m_bufferInfo_(bufferInfo) {}

  WriteDescriptorInfo(VkDescriptorImageInfo imageInfo)
      : m_imageInfo_(imageInfo) {}

  // Raytracing (WIP)
  // WriteDescriptorInfo(
  //     VkWriteDescriptorSetAccelerationStructureKHR
  //     accelerationStructureInfo) :
  //     AccelerationStructureInfo(accelerationStructureInfo) {}

  VkDescriptorBufferInfo m_bufferInfo_{};
  VkDescriptorImageInfo  m_imageInfo_{};
  // Raytracing (WIP)
  // VkWriteDescriptorSetAccelerationStructureKHR AccelerationStructureInfo{};
};

struct WriteDescriptorSet {
  void reset();

  void setWriteDescriptorInfo(int32_t               index,
                              const ShaderBinding* shaderBinding);

  bool                              m_isInitialized_ = false;
  std::vector<WriteDescriptorInfo>  m_writeDescriptorInfos_;
  // This is the final result, generated using WriteDescriptorInfos
  std::vector<VkWriteDescriptorSet> m_descriptorWrites_;
  std::vector<uint32_t>             m_dynamicOffsets_;
};

// ----------------------

// TODO: move to Vulkan folder
struct ShaderBindingInstanceVk : public ShaderBindingInstance {
  virtual ~ShaderBindingInstanceVk() {}

  const struct ShaderBindingLayoutVk* m_shaderBindingsLayouts_ = nullptr;

  static void s_createWriteDescriptorSet(
      WriteDescriptorSet&        descriptorWrites,
      const VkDescriptorSet      descriptorSet,
      const ShaderBindingArray& shaderBindingArray);

  static void s_updateWriteDescriptorSet(
      WriteDescriptorSet&        descriptorWrites,
      const ShaderBindingArray& shaderBindingArray);

  virtual void initialize(
      const ShaderBindingArray& shaderBindingArray) override;

  virtual void updateShaderBindings(
      const ShaderBindingArray& shaderBindingArray) override;

  virtual void* getHandle() const override { return m_descriptorSet_; }

  virtual const std::vector<uint32_t>* getDynamicOffsets() const override {
    return &m_writeDescriptorSet_.m_dynamicOffsets_;
  }

  virtual void free() override;

  virtual ShaderBindingInstanceType getType() const { return m_type_; }

  virtual void setType(const ShaderBindingInstanceType type) {
    m_type_ = type;
  }

  private:
  ShaderBindingInstanceType m_type_ = ShaderBindingInstanceType::SingleFrame;

  public:
  // When the DescriptorPool is released, everything can be handled, so it is
  // not separately destroyed
  VkDescriptorSet    m_descriptorSet_ = nullptr;
  WriteDescriptorSet m_writeDescriptorSet_;
};

struct ShaderBindingLayoutVk;

struct ShaderBindingLayoutVk : public ShaderBindingLayout {
  virtual ~ShaderBindingLayoutVk() { release(); }

  virtual bool initialize(
      const ShaderBindingArray& shaderBindingArray) override;

  virtual std::shared_ptr<ShaderBindingInstance> createShaderBindingInstance(
      const ShaderBindingArray&       shaderBindingArray,
      const ShaderBindingInstanceType type) const override;

  virtual size_t getHash() const override;

  virtual const ShaderBindingArray& getShaderBindingsLayout() const {
    return m_shaderBindingArray_;
  }

  virtual void* getHandle() const override { return m_descriptorSetLayout_; }

  void release();

  // TODO: not used
  std::vector<VkDescriptorPoolSize> getDescriptorPoolSizeArray(
      uint32_t maxAllocations) const;

  mutable size_t m_hash_ = 0;

  protected:
  ShaderBindingArray m_shaderBindingArray_;

  public:
  static VkDescriptorSetLayout s_createDescriptorSetLayout(
      const ShaderBindingArray& shaderBindingArray);

  static VkPipelineLayout s_createPipelineLayout(
      const ShaderBindingLayoutArray& shaderBindingLayoutArray,
      const PushConstant*             pushConstant);

  static void s_clearPipelineLayout();

  VkDescriptorSetLayout m_descriptorSetLayout_ = nullptr;

  static MutexRWLock s_descriptorLayoutPoolLock;
  static std::unordered_map<size_t, VkDescriptorSetLayout> s_descriptorLayoutPool;

  static MutexRWLock                                  s_pipelineLayoutPoolLock;
  static std::unordered_map<size_t, VkPipelineLayout> s_pipelineLayoutPool;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_BINDING_LAYOUT_VK_H