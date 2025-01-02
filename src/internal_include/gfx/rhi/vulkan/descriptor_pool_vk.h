#ifndef GAME_ENGINE_DESCRIPTOR_POOL_VK_H
#define GAME_ENGINE_DESCRIPTOR_POOL_VK_H

#include "gfx/rhi/lock.h"
#include "gfx/rhi/vulkan/shader_binding_layout_vk.h"

#include <vulkan/vulkan.h>

#include <functional>
#include <map>
#include <unordered_map>

#define USE_RESET_DESCRIPTOR_POOL 1

namespace game_engine {

// TODO: currently multiframe descriptor sets are not supported (need to add
// correct handler)

const std::unordered_map<VkDescriptorType, float> g_kDefaultPoolSizes = {
  {               VK_DESCRIPTOR_TYPE_SAMPLER,       2},
  {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,       2},
  {         VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,       2},
  {         VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 / 8.0},
  {  VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1 / 2.0},
  {  VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1 / 8.0},
  {        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 / 4.0},
  {        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 / 8.0},
  {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,       4},
  {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1 / 8.0},
  {      VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1 / 8.0},
  {  VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, 1 / 8.0},
  //{VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1 / 8.0},
};

struct DescriptorPoolVk {
  // ======= BEGIN: public constructors =======================================

  DescriptorPoolVk() = default;

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~DescriptorPoolVk();

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void create(uint32_t maxDescriptorSets = 128);

  virtual void reset();

  virtual std::shared_ptr<ShaderBindingInstance> allocateDescriptorSet(
      VkDescriptorSetLayout layout);

  virtual void free(
      std::shared_ptr<ShaderBindingInstance> shaderBindingInstance);

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc methods =======================================

  void release();

  // This will be called from 'DeallocatorMultiFrameShaderBindingInstance'
  void freedFromPendingDelegate(
      std::shared_ptr<ShaderBindingInstance> shaderBindingInstance);

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  std::map<VkDescriptorSetLayout, ShaderBindingInstancePtrArray>
      m_pendingDescriptorSets_;
  std::map<VkDescriptorSetLayout, ShaderBindingInstancePtrArray>
      m_allocatedDescriptorSets_;

  uint32_t         m_maxDescriptorSets_ = 128;
  // TODO: check that std::size works as _countof
  // uint32_t         PoolSizes[std::size(g_kDefaultPoolSizes)];
  VkDescriptorPool m_descriptorPool_ = nullptr;
  ;
  mutable MutexLock m_descriptorPoolLock_;

  DeallocatorMultiFrameShaderBindingInstance
      m_deallocateMultiframeShaderBindingInstance_;

  // ======= END: public misc fields   ========================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_DESCRIPTOR_POOL_VK_H
