
#include "gfx/rhi/vulkan/descriptor_pool_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

DescriptorPoolVk::~DescriptorPoolVk() {
  if (m_descriptorPool_) {
    vkDestroyDescriptorPool(g_rhiVk->m_device_, m_descriptorPool_, nullptr);
    m_descriptorPool_ = nullptr;
  }
  m_deallocateMultiframeShaderBindingInstance_.m_freeDelegate_ = nullptr;
}

void DescriptorPoolVk::create(uint32_t maxDescriptorSets) {
  if (m_descriptorPool_) {
    vkDestroyDescriptorPool(g_rhiVk->m_device_, m_descriptorPool_, nullptr);
    m_descriptorPool_ = nullptr;

#if !USE_RESET_DESCRIPTOR_POOL
    ScopedLock s(&m_descriptorPoolLock_);

    m_pendingDescriptorSets_.clear();
    assert(m_allocatedDescriptorSets_.size() <= 0);
#endif
  }

  assert(!m_descriptorPool_);

  m_maxDescriptorSets_ = maxDescriptorSets;

  // TODO: remove (old version)
  // constexpr int32_t    NumOfPoolSize = std::size(g_kDefaultPoolSizes);
  // VkDescriptorPoolSize Types[NumOfPoolSize];
  // memset(Types, 0, sizeof(Types));

  // TODO: correct the logic
  // for (uint32_t i = 0; i < NumOfPoolSize; ++i) {
  //  EShaderBindingType DescriptorType
  //      = static_cast<EShaderBindingType>(VK_DESCRIPTOR_TYPE_SAMPLER + i);

  //  PoolSizes[i] = static_cast<uint32_t>(
  //      std::max(g_kDefaultPoolSizes[i] * maxDescriptorSets, 4.0f));

  //  Types[i].type            = GetVulkanShaderBindingType(DescriptorType);
  //  Types[i].descriptorCount = PoolSizes[i];
  //}

  std::vector<VkDescriptorPoolSize> Types(g_kDefaultPoolSizes.size());

  for (uint32_t i = 0; const auto& descriptorTypePair : g_kDefaultPoolSizes) {
    uint32_t poolSize = static_cast<uint32_t>(
        std::max(descriptorTypePair.second * maxDescriptorSets, 4.0f));

    if (descriptorTypePair.first
        == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT) {
      poolSize = ((poolSize + 3) / 4) * 4;
    }

    Types[i].type            = descriptorTypePair.first;
    Types[i].descriptorCount = poolSize;
    i++;
  }

  VkDescriptorPoolCreateInfo PoolInfo{};
  PoolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  PoolInfo.poolSizeCount = Types.size();
  PoolInfo.pPoolSizes    = Types.data();
  PoolInfo.maxSets       = maxDescriptorSets;
  PoolInfo.flags
      = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;  // for bindless
                                                          // resources

  assert(VK_SUCCESS
         == vkCreateDescriptorPool(
             g_rhiVk->m_device_, &PoolInfo, nullptr, &m_descriptorPool_));

  m_deallocateMultiframeShaderBindingInstance_.m_freeDelegate_ = std::bind(
      &DescriptorPoolVk::freedFromPendingDelegate, this, std::placeholders::_1);
}

void DescriptorPoolVk::reset() {
  {
#if USE_RESET_DESCRIPTOR_POOL
    assert(VK_SUCCESS
           == vkResetDescriptorPool(g_rhiVk->m_device_, m_descriptorPool_, 0));
#else
    ScopedLock s(&m_descriptorPoolLock_);
    m_pendingDescriptorSets_ = m_allocatedDescriptorSets_;
#endif
  }
}

std::shared_ptr<ShaderBindingInstance> DescriptorPoolVk::allocateDescriptorSet(
    VkDescriptorSetLayout layout) {
  ScopedLock s(&m_descriptorPoolLock_);
#if !USE_RESET_DESCRIPTOR_POOL
  const auto it_find = m_pendingDescriptorSets_.find(layout);
  if (it_find != m_pendingDescriptorSets_.end()) {
    ShaderBindingInstancePtrArray& pendingPools = it_find->second;
    if (pendingPools.size()) {
      std::shared_ptr<ShaderBindingInstance> descriptorSet
          = *pendingPools.rbegin();
      // pendingPools.popBack();
      pendingPools.resize(pendingPools.size() - 1);
      return descriptorSet;
    }
  }
#endif

  VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo{};
  DescriptorSetAllocateInfo.sType
      = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  DescriptorSetAllocateInfo.descriptorPool     = m_descriptorPool_;
  DescriptorSetAllocateInfo.descriptorSetCount = 1;
  DescriptorSetAllocateInfo.pSetLayouts        = &layout;

  VkDescriptorSet NewDescriptorSet = nullptr;

  auto result = vkAllocateDescriptorSets(
      g_rhiVk->m_device_, &DescriptorSetAllocateInfo, &NewDescriptorSet);

  if (result != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to allocate descriptor set");
    return nullptr;
  }

  auto NewCachedDescriptorSet = std::make_shared<ShaderBindingInstanceVk>();
  NewCachedDescriptorSet->m_descriptorSet_ = NewDescriptorSet;

#if !USE_RESET_DESCRIPTOR_POOL
  m_allocatedDescriptorSets_[layout].push_back(NewCachedDescriptorSet);
#endif

  return NewCachedDescriptorSet;
}

void DescriptorPoolVk::free(
    std::shared_ptr<ShaderBindingInstance> shaderBindingInstance) {
  assert(shaderBindingInstance);
  ScopedLock s(&m_descriptorPoolLock_);

  m_deallocateMultiframeShaderBindingInstance_.free(shaderBindingInstance);

  // const int32_t CurrentFrameNumber = g_rhiVk->getCurrentFrameNumber();
  // const int32_t OldestFrameToKeep = CurrentFrameNumber -
  // NumOfFramesToWaitBeforeReleasing;

  //// ProcessPendingDescriptorPoolFree
  //{
  //    // Check it is too early
  //    if (CurrentFrameNumber >=
  //    CanReleasePendingFreeShaderBindingInstanceFrameNumber)
  //    {
  //        // Release pending memory
  //        int32_t i = 0;
  //        for (; i < PendingFree.size(); ++i)
  //        {
  //            PendingFreeShaderBindingInstance&
  //            PendingFreeShaderBindingInstance = PendingFree[i]; if
  //            (PendingFreeShaderBindingInstance.FrameIndex <
  //            OldestFrameToKeep)
  //            {
  //                // Return to pending descriptor set
  //                assert(PendingFreeShaderBindingInstance.m_shaderBindingInstance);
  //                const VkDescriptorSetLayout DescriptorSetLayout =
  //                (VkDescriptorSetLayout)PendingFreeShaderBindingInstance.m_shaderBindingInstance->m_shaderBindingsLayouts_->getHandle();
  //                PendingDescriptorSets[DescriptorSetLayout].push_back(PendingFreeShaderBindingInstance.m_shaderBindingInstance);
  //            }
  //            else
  //            {
  //                CanReleasePendingFreeShaderBindingInstanceFrameNumber =
  //                PendingFreeShaderBindingInstance.FrameIndex +
  //                NumOfFramesToWaitBeforeReleasing + 1; break;
  //            }
  //        }
  //        if (i > 0)
  //        {
  //            const size_t RemainingSize = (PendingFree.size() - i);
  //            if (RemainingSize > 0)
  //            {
  //                for (int32_t k = 0; k < RemainingSize; ++k)
  //                    PendingFree[k] = PendingFree[i + k];
  //                //memcpy(&PendingFree[0], &PendingFree[i],
  //                sizeof(PendingFreeShaderBindingInstance) *
  //                RemainingSize);
  //            }
  //            PendingFree.resize(RemainingSize);
  //        }
  //    }
  //}

  // PendingFree.emplace_back(PendingFreeShaderBindingInstance(CurrentFrameNumber,
  // shaderBindingInstance));
}

void DescriptorPoolVk::release() {
  if (m_descriptorPool_) {
    vkDestroyDescriptorPool(g_rhiVk->m_device_, m_descriptorPool_, nullptr);
    m_descriptorPool_ = nullptr;
  }

  {
    ScopedLock s(&m_descriptorPoolLock_);

    // for (auto& iter : AllocatedDescriptorSets)
    //{
    //     ShaderBindingInstanceVulkanArray& instances = iter.second;
    //     for (int32_t i = 0; i < instances.size(); ++i)
    //     {
    //         delete instances[i];
    //     }
    // }
    m_allocatedDescriptorSets_.clear();
  }
}

// This will be called from 'm_deallocatorMultiFrameShaderBindingInstance'
void DescriptorPoolVk::freedFromPendingDelegate(
    std::shared_ptr<ShaderBindingInstance> shaderBindingInstance) {
  ShaderBindingInstanceVk* shaderBindingInstanceVk
      = (ShaderBindingInstanceVk*)shaderBindingInstance.get();
  assert(shaderBindingInstanceVk);

  const VkDescriptorSetLayout DescriptorSetLayout
      = (VkDescriptorSetLayout)
            shaderBindingInstanceVk->m_shaderBindingsLayouts_->getHandle();
#if !USE_RESET_DESCRIPTOR_POOL
  m_pendingDescriptorSets_[DescriptorSetLayout].push_back(
      shaderBindingInstance);
#endif
}

}  // namespace game_engine
