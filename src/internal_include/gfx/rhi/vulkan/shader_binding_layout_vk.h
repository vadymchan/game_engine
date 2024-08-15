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

  WriteDescriptorInfo(VkDescriptorBufferInfo InBufferInfo)
      : BufferInfo(InBufferInfo) {}

  WriteDescriptorInfo(VkDescriptorImageInfo InImageInfo)
      : ImageInfo(InImageInfo) {}

  // Raytracing (WIP)
  // WriteDescriptorInfo(
  //     VkWriteDescriptorSetAccelerationStructureKHR
  //     InAccelerationStructureInfo) :
  //     AccelerationStructureInfo(InAccelerationStructureInfo) {}

  VkDescriptorBufferInfo BufferInfo{};
  VkDescriptorImageInfo  ImageInfo{};
  // Raytracing (WIP)
  // VkWriteDescriptorSetAccelerationStructureKHR AccelerationStructureInfo{};
};

struct WriteDescriptorSet {
  void Reset();

  void SetWriteDescriptorInfo(int32_t               InIndex,
                              const ShaderBinding* InShaderBinding);

  bool                              IsInitialized = false;
  std::vector<WriteDescriptorInfo>  WriteDescriptorInfos;
  // This is the final result, generated using WriteDescriptorInfos
  std::vector<VkWriteDescriptorSet> DescriptorWrites;
  std::vector<uint32_t>             DynamicOffsets;
};

// ----------------------

// TODO: move to Vulkan folder
struct ShaderBindingInstanceVk : public ShaderBindingInstance {
  virtual ~ShaderBindingInstanceVk() {}

  const struct ShaderBindingLayoutVk* ShaderBindingsLayouts = nullptr;

  static void CreateWriteDescriptorSet(
      WriteDescriptorSet&        OutDescriptorWrites,
      const VkDescriptorSet      InDescriptorSet,
      const ShaderBindingArray& InShaderBindingArray);

  static void UpdateWriteDescriptorSet(
      WriteDescriptorSet&        OutDescriptorWrites,
      const ShaderBindingArray& InShaderBindingArray);

  virtual void Initialize(
      const ShaderBindingArray& InShaderBindingArray) override;

  virtual void UpdateShaderBindings(
      const ShaderBindingArray& InShaderBindingArray) override;

  virtual void* GetHandle() const override { return DescriptorSet; }

  virtual const std::vector<uint32_t>* GetDynamicOffsets() const override {
    return &writeDescriptorSet.DynamicOffsets;
  }

  virtual void Free() override;

  virtual ShaderBindingInstanceType GetType() const { return Type; }

  virtual void SetType(const ShaderBindingInstanceType InType) {
    Type = InType;
  }

  private:
  ShaderBindingInstanceType Type = ShaderBindingInstanceType::SingleFrame;

  public:
  // When the DescriptorPool is released, everything can be handled, so it is
  // not separately destroyed
  VkDescriptorSet    DescriptorSet = nullptr;
  WriteDescriptorSet writeDescriptorSet;
};

struct ShaderBindingLayoutVk;

struct ShaderBindingLayoutVk : public ShaderBindingLayout {
  virtual ~ShaderBindingLayoutVk() { Release(); }

  virtual bool Initialize(
      const ShaderBindingArray& InShaderBindingArray) override;

  virtual std::shared_ptr<ShaderBindingInstance> CreateShaderBindingInstance(
      const ShaderBindingArray&       InShaderBindingArray,
      const ShaderBindingInstanceType InType) const override;

  virtual size_t GetHash() const override;

  virtual const ShaderBindingArray& GetShaderBindingsLayout() const {
    return shaderBindingArray;
  }

  virtual void* GetHandle() const override { return DescriptorSetLayout; }

  void Release();

  std::vector<VkDescriptorPoolSize> GetDescriptorPoolSizeArray(
      uint32_t maxAllocations) const;

  mutable size_t Hash = 0;

  protected:
  ShaderBindingArray shaderBindingArray;

  public:
  VkDescriptorSetLayout DescriptorSetLayout = nullptr;

  static VkDescriptorSetLayout CreateDescriptorSetLayout(
      const ShaderBindingArray& InShaderBindingArray);

  static VkPipelineLayout CreatePipelineLayout(
      const ShaderBindingLayoutArray& InShaderBindingLayoutArray,
      const PushConstant*             pushConstant);

  static void ClearPipelineLayout();

  static MutexRWLock DescriptorLayoutPoolLock;
  static std::unordered_map<size_t, VkDescriptorSetLayout> DescriptorLayoutPool;

  static MutexRWLock                                  PipelineLayoutPoolLock;
  static std::unordered_map<size_t, VkPipelineLayout> PipelineLayoutPool;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_BINDING_LAYOUT_VK_H