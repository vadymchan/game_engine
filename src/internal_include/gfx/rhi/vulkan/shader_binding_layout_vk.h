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

  WriteDescriptorInfo(VkDescriptorImageInfo InImageInfo)
      : m_imageInfo_(InImageInfo) {}

  // Raytracing (WIP)
  // WriteDescriptorInfo(
  //     VkWriteDescriptorSetAccelerationStructureKHR
  //     InAccelerationStructureInfo) :
  //     AccelerationStructureInfo(InAccelerationStructureInfo) {}

  VkDescriptorBufferInfo m_bufferInfo_{};
  VkDescriptorImageInfo  m_imageInfo_{};
  // Raytracing (WIP)
  // VkWriteDescriptorSetAccelerationStructureKHR AccelerationStructureInfo{};
};

struct WriteDescriptorSet {
  void Reset();

  void SetWriteDescriptorInfo(int32_t               index,
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

  const struct ShaderBindingLayoutVk* ShaderBindingsLayouts = nullptr;

  static void CreateWriteDescriptorSet(
      WriteDescriptorSet&        OutDescriptorWrites,
      const VkDescriptorSet      InDescriptorSet,
      const ShaderBindingArray& shaderBindingArray);

  static void UpdateWriteDescriptorSet(
      WriteDescriptorSet&        OutDescriptorWrites,
      const ShaderBindingArray& shaderBindingArray);

  virtual void Initialize(
      const ShaderBindingArray& shaderBindingArray) override;

  virtual void UpdateShaderBindings(
      const ShaderBindingArray& shaderBindingArray) override;

  virtual void* GetHandle() const override { return m_descriptorSet_; }

  virtual const std::vector<uint32_t>* GetDynamicOffsets() const override {
    return &m_writeDescriptorSet_.m_dynamicOffsets_;
  }

  virtual void Free() override;

  virtual ShaderBindingInstanceType GetType() const { return m_type_; }

  virtual void SetType(const ShaderBindingInstanceType type) {
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
  virtual ~ShaderBindingLayoutVk() { Release(); }

  virtual bool Initialize(
      const ShaderBindingArray& shaderBindingArray) override;

  virtual std::shared_ptr<ShaderBindingInstance> CreateShaderBindingInstance(
      const ShaderBindingArray&       shaderBindingArray,
      const ShaderBindingInstanceType type) const override;

  virtual size_t GetHash() const override;

  virtual const ShaderBindingArray& GetShaderBindingsLayout() const {
    return m_shaderBindingArray_;
  }

  virtual void* GetHandle() const override { return m_descriptorSetLayout_; }

  void Release();

  std::vector<VkDescriptorPoolSize> GetDescriptorPoolSizeArray(
      uint32_t maxAllocations) const;

  mutable size_t m_hash_ = 0;

  protected:
  ShaderBindingArray m_shaderBindingArray_;

  public:
  static VkDescriptorSetLayout CreateDescriptorSetLayout(
      const ShaderBindingArray& shaderBindingArray);

  static VkPipelineLayout CreatePipelineLayout(
      const ShaderBindingLayoutArray& shaderBindingLayoutArray,
      const PushConstant*             pushConstant);

  static void ClearPipelineLayout();

  VkDescriptorSetLayout m_descriptorSetLayout_ = nullptr;

  static MutexRWLock s_descriptorLayoutPoolLock;
  static std::unordered_map<size_t, VkDescriptorSetLayout> s_descriptorLayoutPool;

  static MutexRWLock                                  s_pipelineLayoutPoolLock;
  static std::unordered_map<size_t, VkPipelineLayout> s_pipelineLayoutPool;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_BINDING_LAYOUT_VK_H