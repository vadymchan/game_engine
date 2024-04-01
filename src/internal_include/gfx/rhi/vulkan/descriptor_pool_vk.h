#ifndef GAME_ENGINE_DESCRIPTOR_POOL_VK_H
#define GAME_ENGINE_DESCRIPTOR_POOL_VK_H

#include "gfx/rhi/lock.h"
#include "gfx/rhi/vulkan/shader_binding_layout_vk.h"

#include <vulkan/vulkan.h>

#include <functional>
#include <map>
#include <unordered_map>

#define USE_RESET_DESCRIPTOR_POOL 0

namespace game_engine {

// TODO: check whether it works correctly
// const float DefaultPoolSizes[] = {
//  2,        // VK_DESCRIPTOR_TYPE_SAMPLER
//  2,        // VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
//  2,        // VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
//  1 / 8.0,  // VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
//  1 / 2.0,  // VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
//  1 / 8.0,  // VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
//  1 / 4.0,  // VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
//  1 / 8.0,  // VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
//  4,        // VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
//  1 / 8.0,  // VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
//  1 / 8.0,  // VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
//  1 / 8.0,  // VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK
//  1 / 8.0,  // VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR
//};

const std::unordered_map<VkDescriptorType, float> DefaultPoolSizes = {
  {                   VK_DESCRIPTOR_TYPE_SAMPLER,       2},
  {    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,       2},
  {             VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,       2},
  {             VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 / 8.0},
  {      VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1 / 2.0},
  {      VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1 / 8.0},
  {            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 / 4.0},
  {            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 / 8.0},
  {    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,       4},
  {    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1 / 8.0},
  {          VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1 / 8.0},
  {      VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, 1 / 8.0},
  //{VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1 / 8.0},
};

struct DescriptorPoolVk {
  std::map<VkDescriptorSetLayout, ShaderBindingInstancePtrArray>
      PendingDescriptorSets;
  std::map<VkDescriptorSetLayout, ShaderBindingInstancePtrArray>
      AllocatedDescriptorSets;

  DescriptorPoolVk() = default;

  virtual ~DescriptorPoolVk();

  virtual void Create(uint32_t InMaxDescriptorSets = 128);

  virtual void Reset();

  virtual std::shared_ptr<ShaderBindingInstance> AllocateDescriptorSet(
      VkDescriptorSetLayout InLayout);

  virtual void Free(
      std::shared_ptr<ShaderBindingInstance> InShaderBindingInstance);

  void Release();

  // This will be called from 'DeallocatorMultiFrameShaderBindingInstance'
  void FreedFromPendingDelegate(
      std::shared_ptr<ShaderBindingInstance> InShaderBindingInstance);

  uint32_t         MaxDescriptorSets = 128;
  // TODO: check that std::size works as _countof
  // uint32_t         PoolSizes[std::size(DefaultPoolSizes)];
  VkDescriptorPool DescriptorPool = nullptr;
  ;
  mutable MutexLock DescriptorPoolLock;

  DeallocatorMultiFrameShaderBindingInstance
      DeallocateMultiframeShaderBindingInstance;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_DESCRIPTOR_POOL_VK_H