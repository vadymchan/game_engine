
#include "gfx/rhi/vulkan/descriptor_pool_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

DescriptorPoolVk::~DescriptorPoolVk() {
  if (DescriptorPool) {
    vkDestroyDescriptorPool(g_rhi_vk->m_device_, DescriptorPool, nullptr);
    DescriptorPool = nullptr;
  }
  DeallocateMultiframeShaderBindingInstance.FreeDelegate = nullptr;
}

void DescriptorPoolVk::Create(uint32_t InMaxDescriptorSets) {
  if (DescriptorPool) {
    vkDestroyDescriptorPool(g_rhi_vk->m_device_, DescriptorPool, nullptr);
    DescriptorPool = nullptr;

#if !USE_RESET_DESCRIPTOR_POOL
    ScopedLock s(&DescriptorPoolLock);

    PendingDescriptorSets.clear();
    assert(AllocatedDescriptorSets.size() <= 0);
#endif
  }

  assert(!DescriptorPool);

  MaxDescriptorSets = InMaxDescriptorSets;
  
  // TODO: remove (old version)
  // constexpr int32_t    NumOfPoolSize = std::size(DefaultPoolSizes);
  // VkDescriptorPoolSize Types[NumOfPoolSize];
  // memset(Types, 0, sizeof(Types));

  // TODO: correct the logic
  // for (uint32_t i = 0; i < NumOfPoolSize; ++i) {
  //  EShaderBindingType DescriptorType
  //      = static_cast<EShaderBindingType>(VK_DESCRIPTOR_TYPE_SAMPLER + i);

  //  PoolSizes[i] = static_cast<uint32_t>(
  //      std::max(DefaultPoolSizes[i] * InMaxDescriptorSets, 4.0f));

  //  Types[i].type            = GetVulkanShaderBindingType(DescriptorType);
  //  Types[i].descriptorCount = PoolSizes[i];
  //}

  std::vector<VkDescriptorPoolSize> Types(DefaultPoolSizes.size());

  for (uint32_t i = 0; const auto& descriptorTypePair : DefaultPoolSizes) {
    uint32_t poolSize = static_cast<uint32_t>(
        std::max(descriptorTypePair.second * InMaxDescriptorSets, 4.0f));

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
  PoolInfo.maxSets       = InMaxDescriptorSets;
  PoolInfo.flags
      = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;  // for bindless
                                                          // resources

  assert(VK_SUCCESS
         == vkCreateDescriptorPool(
             g_rhi_vk->m_device_, &PoolInfo, nullptr, &DescriptorPool));

  DeallocateMultiframeShaderBindingInstance.FreeDelegate = std::bind(
      &DescriptorPoolVk::FreedFromPendingDelegate, this, std::placeholders::_1);

}

void DescriptorPoolVk::Reset() {
  {
#if USE_RESET_DESCRIPTOR_POOL
    verify(VK_SUCCESS
           == vkResetDescriptorPool(g_rhi_vk->m_device_, DescriptorPool, 0));
#else
    ScopedLock s(&DescriptorPoolLock);
    PendingDescriptorSets = AllocatedDescriptorSets;
#endif
  }
}

std::shared_ptr<ShaderBindingInstance> DescriptorPoolVk::AllocateDescriptorSet(
    VkDescriptorSetLayout InLayout) {
  ScopedLock s(&DescriptorPoolLock);
#if !USE_RESET_DESCRIPTOR_POOL
  const auto it_find = PendingDescriptorSets.find(InLayout);
  if (it_find != PendingDescriptorSets.end()) {
    ShaderBindingInstancePtrArray& pendingPools = it_find->second;
    if (pendingPools.size()) {
      std::shared_ptr<ShaderBindingInstance> descriptorSet
          = *pendingPools.rbegin();
      // pendingPools.PopBack();
      pendingPools.resize(pendingPools.size() - 1);
      return descriptorSet;
    }
  }
#endif

  VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo{};
  DescriptorSetAllocateInfo.sType
      = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  DescriptorSetAllocateInfo.descriptorPool     = DescriptorPool;
  DescriptorSetAllocateInfo.descriptorSetCount = 1;
  DescriptorSetAllocateInfo.pSetLayouts        = &InLayout;

  VkDescriptorSet NewDescriptorSet = nullptr;

  if (vkAllocateDescriptorSets(
          g_rhi_vk->m_device_, &DescriptorSetAllocateInfo, &NewDescriptorSet)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to allocate descriptor set");
    return nullptr;
  }

  auto NewShaderBindingInstanceVk           = new ShaderBindingInstanceVk();
  NewShaderBindingInstanceVk->DescriptorSet = NewDescriptorSet;
  std::shared_ptr<ShaderBindingInstanceVk> NewCachedDescriptorSet
      = std::shared_ptr<ShaderBindingInstanceVk>(NewShaderBindingInstanceVk);

#if !USE_RESET_DESCRIPTOR_POOL
  AllocatedDescriptorSets[InLayout].push_back(NewCachedDescriptorSet);
#endif

  return NewCachedDescriptorSet;
}

void DescriptorPoolVk::Free(
    std::shared_ptr<ShaderBindingInstance> InShaderBindingInstance) {
  assert(InShaderBindingInstance);
  ScopedLock s(&DescriptorPoolLock);

  DeallocateMultiframeShaderBindingInstance.Free(InShaderBindingInstance);

  // const int32_t CurrentFrameNumber = g_rhi_vk->GetCurrentFrameNumber();
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
  //                (VkDescriptorSetLayout)PendingFreeShaderBindingInstance.m_shaderBindingInstance->ShaderBindingsLayouts->GetHandle();
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
  // InShaderBindingInstance));
}

void DescriptorPoolVk::Release() {
  if (DescriptorPool) {
    vkDestroyDescriptorPool(g_rhi_vk->m_device_, DescriptorPool, nullptr);
    DescriptorPool = nullptr;
  }

  {
    ScopedLock s(&DescriptorPoolLock);

    // for (auto& iter : AllocatedDescriptorSets)
    //{
    //     ShaderBindingInstanceVulkanArray& instances = iter.second;
    //     for (int32_t i = 0; i < instances.size(); ++i)
    //     {
    //         delete instances[i];
    //     }
    // }
    AllocatedDescriptorSets.clear();
  }
}

// This will be called from 'm_deallocatorMultiFrameShaderBindingInstance'
void DescriptorPoolVk::FreedFromPendingDelegate(
    std::shared_ptr<ShaderBindingInstance> InShaderBindingInstance) {
  ShaderBindingInstanceVk* shaderBindingInstanceVk
      = (ShaderBindingInstanceVk*)InShaderBindingInstance.get();
  assert(shaderBindingInstanceVk);

  const VkDescriptorSetLayout DescriptorSetLayout
      = (VkDescriptorSetLayout)
            shaderBindingInstanceVk->ShaderBindingsLayouts->GetHandle();
  PendingDescriptorSets[DescriptorSetLayout].push_back(InShaderBindingInstance);
}

}  // namespace game_engine
