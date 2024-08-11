
#include "gfx/rhi/vulkan/shader_binding_layout_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

MutexRWLock ShaderBindingLayoutVk::DescriptorLayoutPoolLock;
std::unordered_map<uint64_t, VkDescriptorSetLayout>
            ShaderBindingLayoutVk::DescriptorLayoutPool;
MutexRWLock ShaderBindingLayoutVk::PipelineLayoutPoolLock;
std::unordered_map<uint64_t, VkPipelineLayout>
    ShaderBindingLayoutVk::PipelineLayoutPool;

//// ShaderBindingVk
//// =========================================================
//size_t ShaderBindingVk::GetHash() const {
//  if (Hash) {
//    return Hash;
//  }
//
//  Hash = GETHASH_FROM_INSTANT_STRUCT(
//      IsInline, BindingPoint, NumOfDescriptors, BindingType, AccessStageFlags);
//  return Hash;
//}
//
//void ShaderBindingVk::CloneWithoutResource(ShaderBindingVk& OutReslut) const {
//  OutReslut.IsInline         = IsInline;
//  OutReslut.BindingPoint     = BindingPoint;
//  OutReslut.NumOfDescriptors = NumOfDescriptors;
//  OutReslut.BindingType      = BindingType;
//  OutReslut.AccessStageFlags = AccessStageFlags;
//  OutReslut.Hash             = Hash;
//}
//
//// =========================================================
//
//// ShaderBindingArray
//// =========================================================
//size_t ShaderBindingArray::GetHash() const {
//  size_t           Hash    = 0;
//  ShaderBindingVk* Address = (ShaderBindingVk*)&Data[0];
//  for (int32_t i = 0; i < NumOfData; ++i) {
//    Hash ^= ((Address + i)->GetHash() << i);
//  }
//  return Hash;
//}
//
//ShaderBindingArray& ShaderBindingArray::operator=(
//    const ShaderBindingArray& In) {
//  memcpy(&Data[0], &In.Data[0], sizeof(ShaderBindingVk) * In.NumOfData);
//  NumOfData = In.NumOfData;
//  return *this;
//}
//
//const ShaderBindingVk* ShaderBindingArray::operator[](int32_t InIndex) const {
//  assert(InIndex < NumOfData);
//  return (ShaderBindingVk*)(&Data[InIndex]);
//}
//
//void ShaderBindingArray::CloneWithoutResource(
//    ShaderBindingArray& OutResult) const {
//  memcpy(&OutResult.Data[0], &Data[0], sizeof(ShaderBindingVk) * NumOfData);
//
//  for (int32_t i = 0; i < NumOfData; ++i) {
//    ShaderBindingVk* SrcAddress = (ShaderBindingVk*)&Data[i];
//    ShaderBindingVk* DstAddress = (ShaderBindingVk*)&OutResult.Data[i];
//    SrcAddress->CloneWithoutResource(*DstAddress);
//  }
//  OutResult.NumOfData = NumOfData;
//}
//
//// =========================================================

// WriteDescriptorSet
// =========================================================

void WriteDescriptorSet::Reset() {
  WriteDescriptorInfos.clear();
  DescriptorWrites.clear();
  DynamicOffsets.clear();
  IsInitialized = false;
}

void WriteDescriptorSet::SetWriteDescriptorInfo(
    int32_t InIndex, const ShaderBinding* InShaderBinding) {
  assert(InShaderBinding);
  assert(InShaderBinding->Resource);
  const bool IsBindless = InShaderBinding->Resource->IsBindless();

  switch (InShaderBinding->BindingType) {
    case EShaderBindingType::UNIFORMBUFFER: {
      if (IsBindless) {
        const UniformBufferResourceBindless* ubor
            = reinterpret_cast<const UniformBufferResourceBindless*>(
                InShaderBinding->Resource);
        assert(ubor);

        if (ubor) {
          for (auto UniformBuffer : ubor->UniformBuffers) {
            assert(UniformBuffer);

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = (VkBuffer)UniformBuffer->GetLowLevelResource();
            bufferInfo.offset = UniformBuffer->GetBufferOffset();
            bufferInfo.range  = UniformBuffer->GetBufferSize();
            assert(bufferInfo.buffer);
            WriteDescriptorInfos.push_back(WriteDescriptorInfo(bufferInfo));
          }
        }
      } else {
        const UniformBufferResource* ubor
            = reinterpret_cast<const UniformBufferResource*>(
                InShaderBinding->Resource);
        assert(ubor && ubor->UniformBuffer);

        if (ubor && ubor->UniformBuffer) {
          VkDescriptorBufferInfo bufferInfo{};
          bufferInfo.buffer
              = (VkBuffer)ubor->UniformBuffer->GetLowLevelResource();
          bufferInfo.offset = ubor->UniformBuffer->GetBufferOffset();
          bufferInfo.range  = ubor->UniformBuffer->GetBufferSize();
          assert(bufferInfo.buffer);
          WriteDescriptorInfos.push_back(WriteDescriptorInfo(bufferInfo));
        }
      }
      break;
    }
    case EShaderBindingType::UNIFORMBUFFER_DYNAMIC: {
      if (IsBindless) {
        const UniformBufferResourceBindless* ubor
            = reinterpret_cast<const UniformBufferResourceBindless*>(
                InShaderBinding->Resource);
        assert(ubor);

        if (ubor) {
          for (auto UniformBuffer : ubor->UniformBuffers) {
            assert(UniformBuffer);

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = (VkBuffer)UniformBuffer->GetLowLevelResource();
            bufferInfo.offset = 0;  // TODO: Use DynamicOffset instead
            bufferInfo.range  = UniformBuffer->GetBufferSize();
            assert(bufferInfo.buffer);
            WriteDescriptorInfos.push_back(WriteDescriptorInfo(bufferInfo));
            DynamicOffsets.push_back(
                (uint32_t)UniformBuffer->GetBufferOffset());
          }
        }
      } else {
        const UniformBufferResource* ubor
            = reinterpret_cast<const UniformBufferResource*>(
                InShaderBinding->Resource);
        assert(ubor && ubor->UniformBuffer);

        if (ubor && ubor->UniformBuffer) {
          VkDescriptorBufferInfo bufferInfo{};
          bufferInfo.buffer
              = (VkBuffer)ubor->UniformBuffer->GetLowLevelResource();
          bufferInfo.offset = 0;  // TODO: Use DynamicOffset instead
          bufferInfo.range  = ubor->UniformBuffer->GetBufferSize();
          assert(bufferInfo.buffer);
          WriteDescriptorInfos.push_back(WriteDescriptorInfo(bufferInfo));
          DynamicOffsets.push_back(
              (uint32_t)ubor->UniformBuffer->GetBufferOffset());
        }
      }
      break;
    }
    case EShaderBindingType::TEXTURE_SAMPLER_SRV:
    case EShaderBindingType::TEXTURE_SRV: {
      if (IsBindless) {
        const TextureResourceBindless* tbor
            = reinterpret_cast<const TextureResourceBindless*>(
                InShaderBinding->Resource);
        assert(tbor);

        if (tbor) {
          for (auto TextureData : tbor->TextureBindDatas) {
            assert(TextureData.m_texture);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout
                = TextureData.m_texture->IsDepthFormat()
                    ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
                    : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView
                = ((const TextureVk*)TextureData.m_texture)->imageView;
            imageInfo.sampler
                = TextureData.SamplerState
                    ? (VkSampler)TextureData.SamplerState->GetHandle()
                    : nullptr;
            if (!imageInfo.sampler) {
              imageInfo.sampler
                  = TextureVk::CreateDefaultSamplerState();  // todo
            }
            assert(imageInfo.imageView);
            WriteDescriptorInfos.push_back(WriteDescriptorInfo(imageInfo));
          }
        }
      } else {
        const TextureResource* tbor = reinterpret_cast<const TextureResource*>(
            InShaderBinding->Resource);
        assert(tbor && tbor->m_texture);

        if (tbor && tbor->m_texture) {
          VkDescriptorImageInfo imageInfo{};
          imageInfo.imageLayout
              = tbor->m_texture->IsDepthFormat()
                  ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
                  : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
          imageInfo.imageView = ((const TextureVk*)tbor->m_texture)->imageView;
          imageInfo.sampler   = tbor->SamplerState
                                  ? (VkSampler)tbor->SamplerState->GetHandle()
                                  : nullptr;
          if (!imageInfo.sampler) {
            imageInfo.sampler = TextureVk::CreateDefaultSamplerState();  // todo
          }
          assert(imageInfo.imageView);
          WriteDescriptorInfos.push_back(WriteDescriptorInfo(imageInfo));
        }
      }
      break;
    }
    case EShaderBindingType::TEXTURE_ARRAY_SRV: {
      const TextureArrayResource* tbor
          = reinterpret_cast<const TextureArrayResource*>(
              InShaderBinding->Resource);
      assert(tbor && tbor->TextureArray);
      if (tbor && tbor->TextureArray) {
        // TODO: Implement
        assert(0);
      }
      break;
    }
    case EShaderBindingType::SUBPASS_INPUT_ATTACHMENT: {
      if (IsBindless) {
        const TextureResourceBindless* tbor
            = reinterpret_cast<const TextureResourceBindless*>(
                InShaderBinding->Resource);
        assert(tbor);

        if (tbor) {
          for (auto TextureData : tbor->TextureBindDatas) {
            assert(TextureData.m_texture);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout
                = TextureData.m_texture->IsDepthFormat()
                    ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
                    : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView
                = ((const TextureVk*)TextureData.m_texture)->imageView;
            imageInfo.sampler
                = TextureData.SamplerState
                    ? (VkSampler)TextureData.SamplerState->GetHandle()
                    : nullptr;
            if (!imageInfo.sampler) {
              imageInfo.sampler
                  = TextureVk::CreateDefaultSamplerState();  // todo
            }
            assert(imageInfo.imageView);
            WriteDescriptorInfos.push_back(WriteDescriptorInfo(imageInfo));
          }
        }
      } else {
        const TextureResource* tbor = reinterpret_cast<const TextureResource*>(
            InShaderBinding->Resource);
        assert(tbor && tbor->m_texture);

        if (tbor && tbor->m_texture) {
          VkDescriptorImageInfo imageInfo{};
          imageInfo.imageLayout
              = (tbor->m_texture->IsDepthFormat()
                     ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
                     : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
          imageInfo.imageView = ((const TextureVk*)tbor->m_texture)->imageView;
          assert(imageInfo.imageView);
          WriteDescriptorInfos.push_back(WriteDescriptorInfo(imageInfo));
        }
      }
      break;
    }
    // Texture UAV
    case EShaderBindingType::TEXTURE_UAV: {
      // TODO: log error
      assert(0);
      // TODO: WIP
      // if (IsBindless) {
      //  const TextureResourceBindless* tbor
      //      = reinterpret_cast<const TextureResourceBindless*>(
      //          InShaderBinding->Resource);
      //  assert(tbor);

      //  if (tbor) {
      //    for (auto TextureData : tbor->TextureBindDatas) {
      //      assert(TextureData.Texture);

      //      VkDescriptorImageInfo imageInfo{};
      //      imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

      //      const TextureVk* Texture = (const
      //      TextureVk*)TextureData.Texture; if (TextureData.MipLevel == 0) {
      //        imageInfo.imageView
      //            = Texture->ViewUAV ? Texture->ViewUAV :
      //            Texture->imageView;
      //        assert(imageInfo.imageView);
      //      } else {
      //        const auto& ViewForMipMap
      //            = (Texture->ViewUAVForMipMap.size() > 0)
      //                ? Texture->ViewUAVForMipMap
      //                : Texture->ViewForMipMap;
      //        auto it_find = ViewForMipMap.find(TextureData.MipLevel);
      //        if (it_find != ViewForMipMap.end() && it_find->second) {
      //          imageInfo.imageView = it_find->second;
      //        }

      //        if (!imageInfo.imageView) {
      //          imageInfo.imageView = Texture->ViewUAV;
      //          assert(imageInfo.imageView);
      //        }
      //      }

      //      if (TextureData.SamplerState) {
      //        imageInfo.sampler
      //            = (VkSampler)TextureData.SamplerState->GetHandle();
      //      }
      //      if (!imageInfo.sampler) {
      //        imageInfo.sampler
      //            = TextureVk::CreateDefaultSamplerState();  // todo
      //      }
      //      assert(imageInfo.imageView);
      //      WriteDescriptorInfos.push_back(WriteDescriptorInfo(imageInfo));
      //    }
      //  }
      //} else {
      //  const TextureResource* tbor
      //      = reinterpret_cast<const TextureResource*>(
      //          InShaderBinding->Resource);
      //  assert(tbor && tbor->Texture);

      //  if (tbor && tbor->Texture) {
      //    VkDescriptorImageInfo imageInfo{};
      //    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

      //    const TextureVk* Texture = (const TextureVk*)tbor->Texture;
      //    if (tbor->MipLevel == 0) {
      //      imageInfo.imageView
      //          = Texture->ViewUAV ? Texture->ViewUAV : Texture->imageView;
      //      assert(imageInfo.imageView);
      //    } else {
      //      const auto& ViewForMipMap = (Texture->ViewUAVForMipMap.size() >
      //      0)
      //                                    ? Texture->ViewUAVForMipMap
      //                                    : Texture->ViewForMipMap;
      //      auto        it_find       = ViewForMipMap.find(tbor->MipLevel);
      //      if (it_find != ViewForMipMap.end() && it_find->second) {
      //        imageInfo.imageView = it_find->second;
      //      }

      //      if (!imageInfo.imageView) {
      //        imageInfo.imageView = Texture->ViewUAV;
      //        assert(imageInfo.imageView);
      //      }
      //    }

      //    if (tbor->SamplerState) {
      //      imageInfo.sampler = (VkSampler)tbor->SamplerState->GetHandle();
      //    }
      //    if (!imageInfo.sampler) {
      //      imageInfo.sampler
      //          = TextureVk::CreateDefaultSamplerState();  // todo
      //    }
      //    assert(imageInfo.imageView);
      //    WriteDescriptorInfos.push_back(WriteDescriptorInfo(imageInfo));
      //  }
      //}
      break;
    }
    case EShaderBindingType::SAMPLER: {
      if (IsBindless) {
        const SamplerResourceBindless* sr
            = reinterpret_cast<const SamplerResourceBindless*>(
                InShaderBinding->Resource);
        assert(sr);

        if (sr) {
          for (auto SamplerState : sr->SamplerStates) {
            assert(SamplerState);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.sampler     = (VkSampler)SamplerState->GetHandle();
            assert(imageInfo.sampler);
            WriteDescriptorInfos.push_back(WriteDescriptorInfo(imageInfo));
          }
        }
      } else {
        const SamplerResource* sr = reinterpret_cast<const SamplerResource*>(
            InShaderBinding->Resource);
        assert(sr && sr->SamplerState);

        if (sr && sr->SamplerState) {
          VkDescriptorImageInfo imageInfo{};
          imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
          imageInfo.sampler     = (VkSampler)sr->SamplerState->GetHandle();
          assert(imageInfo.sampler);
          WriteDescriptorInfos.push_back(WriteDescriptorInfo(imageInfo));
        }
      }
      break;
    }
    case EShaderBindingType::BUFFER_TEXEL_SRV:
    case EShaderBindingType::BUFFER_TEXEL_UAV:
    case EShaderBindingType::BUFFER_SRV:
    case EShaderBindingType::BUFFER_UAV:
    case EShaderBindingType::BUFFER_UAV_DYNAMIC: {
      if (IsBindless) {
        const BufferResourceBindless* br
            = reinterpret_cast<const BufferResourceBindless*>(
                InShaderBinding->Resource);
        for (auto Buffer : br->Buffers) {
          assert(Buffer);

          VkDescriptorBufferInfo bufferInfo{};
          bufferInfo.buffer = (VkBuffer)Buffer->GetHandle();
          bufferInfo.offset = Buffer->GetOffset();
          bufferInfo.range  = Buffer->GetBufferSize();
          assert(bufferInfo.buffer);
          WriteDescriptorInfos.push_back(WriteDescriptorInfo(bufferInfo));
        }
      } else {
        const BufferResource* br = reinterpret_cast<const BufferResource*>(
            InShaderBinding->Resource);
        assert(br && br->Buffer);
        if (br && br->Buffer) {
          VkDescriptorBufferInfo bufferInfo{};
          bufferInfo.buffer = (VkBuffer)br->Buffer->GetHandle();
          bufferInfo.offset = br->Buffer->GetOffset();
          bufferInfo.range  = br->Buffer->GetBufferSize();
          assert(bufferInfo.buffer);
          WriteDescriptorInfos.push_back(WriteDescriptorInfo(bufferInfo));
        }
      }
      break;
    }
    // Not needed for now (RTX stuff)
    /*case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: {
      if (IsBindless) {
        const BufferResourceBindless* br
            = reinterpret_cast<const BufferResourceBindless*>(
                InShaderBinding->Resource);
        for (auto Buffer : br->Buffers) {
          assert(Buffer);

          VkWriteDescriptorSetAccelerationStructureKHR
              AccelerationStructureKHR{};
          AccelerationStructureKHR.sType
              =
    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
          AccelerationStructureKHR.accelerationStructureCount = 1;
          AccelerationStructureKHR.pAccelerationStructures
              = &((BufferVk*)Buffer)->AccelerationStructure;
          assert(AccelerationStructureKHR.pAccelerationStructures);
          WriteDescriptorInfos.push_back(
              WriteDescriptorInfo(AccelerationStructureKHR));
        }
      } else {
        const BufferResource* br = reinterpret_cast<const BufferResource*>(
            InShaderBinding->Resource);
        assert(br && br->Buffer);
        if (br && br->Buffer) {
          VkWriteDescriptorSetAccelerationStructureKHR
              AccelerationStructureKHR{};
          AccelerationStructureKHR.sType
              =
    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
          AccelerationStructureKHR.accelerationStructureCount = 1;
          AccelerationStructureKHR.pAccelerationStructures
              = &((BufferVk*)br->Buffer)->AccelerationStructure;
          assert(AccelerationStructureKHR.pAccelerationStructures);
          WriteDescriptorInfos.push_back(
              WriteDescriptorInfo(AccelerationStructureKHR));
        }
      }
      break;
    }*/
    default:
      assert(0);
      break;
  }
}

// =========================================================

// m_shaderBindingInstance
// =========================================================

void ShaderBindingInstanceVk::CreateWriteDescriptorSet(
    WriteDescriptorSet&        OutDescriptorWrites,
    const VkDescriptorSet      InDescriptorSet,
    const ShaderBindingArray& InShaderBindingArray) {
  //  assert(InShaderBindingArray.NumOfData);
  //
  //  OutDescriptorWrites.Reset();
  //
  //  std::vector<WriteDescriptorInfo>& descriptors
  //      = OutDescriptorWrites.WriteDescriptorInfos;
  //  std::vector<VkWriteDescriptorSet>& descriptorWrites
  //      = OutDescriptorWrites.DescriptorWrites;
  //  descriptors.resize(InShaderBindingArray.NumOfData);
  //  descriptorWrites.resize(InShaderBindingArray.NumOfData);
  //  OutDescriptorWrites.DynamicOffsets.clear();
  //  OutDescriptorWrites.DynamicOffsets.reserve(InShaderBindingArray.NumOfData);
  //
  //  for (int32_t i = 0; i < InShaderBindingArray.NumOfData; ++i) {
  //    OutDescriptorWrites.SetWriteDescriptorInfo(i, InShaderBindingArray[i]);
  //
  //    VkWriteDescriptorSet& CurDescriptorWrite = descriptorWrites[i];
  //    CurDescriptorWrite.sType           =
  //    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; CurDescriptorWrite.dstSet =
  //    InDescriptorSet; CurDescriptorWrite.dstBinding      = i;
  //    CurDescriptorWrite.dstArrayElement = 0;
  //    CurDescriptorWrite.descriptorType
  //        = GetVulkanShaderBindingType(InShaderBindingArray[i]->BindingType);
  //    CurDescriptorWrite.descriptorCount = 1;
  //    if (descriptors[i].BufferInfo.buffer) {
  //      CurDescriptorWrite.pBufferInfo = &descriptors[i].BufferInfo;
  //    } else if (descriptors[i].ImageInfo.imageView
  //               || descriptors[i].ImageInfo.sampler) {
  //      CurDescriptorWrite.pImageInfo = &descriptors[i].ImageInfo;  //
  //      Optional
  //    } else {
  //      assert(0);
  //    }
  //  }
  //
  //  OutDescriptorWrites.IsInitialized = true;
  //}
  if (!(InShaderBindingArray.NumOfData)) {
    // TODO: log error
    assert(0);
    return;
  }

  OutDescriptorWrites.Reset();

  std::vector<WriteDescriptorInfo>& descriptors
      = OutDescriptorWrites.WriteDescriptorInfos;
  std::vector<VkWriteDescriptorSet>& descriptorWrites
      = OutDescriptorWrites.DescriptorWrites;
  std::vector<uint32_t>& dynamicOffsets = OutDescriptorWrites.DynamicOffsets;

  int32_t LastIndex = 0;
  for (int32_t i = 0; i < InShaderBindingArray.NumOfData; ++i) {
    OutDescriptorWrites.SetWriteDescriptorInfo(i, InShaderBindingArray[i]);
    descriptorWrites.resize(descriptors.size());

    for (int32_t k = LastIndex; k < descriptorWrites.size(); ++k) {
      VkWriteDescriptorSet& CurDescriptorWrite = descriptorWrites[k];
      CurDescriptorWrite.sType      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      CurDescriptorWrite.dstSet     = InDescriptorSet;
      CurDescriptorWrite.dstBinding = i;
      CurDescriptorWrite.dstArrayElement = k - LastIndex;
      CurDescriptorWrite.descriptorType
          = GetVulkanShaderBindingType(InShaderBindingArray[i]->BindingType);
      CurDescriptorWrite.descriptorCount = 1;
    }
    LastIndex = (int32_t)descriptorWrites.size();
  }

  assert(descriptors.size() == descriptorWrites.size());
  for (int32_t i = 0; i < (int32_t)descriptorWrites.size(); ++i) {
    const WriteDescriptorInfo& WriteDescriptorInfo = descriptors[i];
    VkWriteDescriptorSet&      CurDescriptorWrite  = descriptorWrites[i];
    if (WriteDescriptorInfo.BufferInfo.buffer) {
      CurDescriptorWrite.pBufferInfo
          = &WriteDescriptorInfo
                 .BufferInfo;  // Buffer should be bound in pBufferInfo
    } else if (WriteDescriptorInfo.ImageInfo.imageView
               || WriteDescriptorInfo.ImageInfo.sampler) {
      CurDescriptorWrite.pImageInfo
          = &WriteDescriptorInfo
                 .ImageInfo;  // Image should be bound in pImageInfo
    } else {
      assert(0);
    }
  }

  OutDescriptorWrites.IsInitialized = true;
}

void ShaderBindingInstanceVk::UpdateWriteDescriptorSet(
    WriteDescriptorSet&        OutDescriptorWrites,
    const ShaderBindingArray& InShaderBindingArray) {
  assert(InShaderBindingArray.NumOfData
         == OutDescriptorWrites.DescriptorWrites.size());

  OutDescriptorWrites.DynamicOffsets.clear();
  for (int32_t i = 0; i < InShaderBindingArray.NumOfData; ++i) {
    OutDescriptorWrites.SetWriteDescriptorInfo(i, InShaderBindingArray[i]);
  }
}

void ShaderBindingInstanceVk::Initialize(
    const ShaderBindingArray& InShaderBindingArray) {
  // if (!writeDescriptorSet.IsInitialized) {
  CreateWriteDescriptorSet(
      writeDescriptorSet, DescriptorSet, InShaderBindingArray);
  // TODO: currently not working
  //} else {
  //  UpdateWriteDescriptorSet(writeDescriptorSet, InShaderBindingArray);
  //}

  vkUpdateDescriptorSets(
      g_rhi_vk->m_device_,
      static_cast<uint32_t>(writeDescriptorSet.DescriptorWrites.size()),
      writeDescriptorSet.DescriptorWrites.data(),
      0,
      nullptr);
}

void ShaderBindingInstanceVk::UpdateShaderBindings(
    const ShaderBindingArray& InShaderBindingArray) {
  assert(ShaderBindingsLayouts->GetShaderBindingsLayout().NumOfData
         == InShaderBindingArray.NumOfData);
  assert(InShaderBindingArray.NumOfData);

  if (!writeDescriptorSet.IsInitialized) {
    CreateWriteDescriptorSet(
        writeDescriptorSet, DescriptorSet, InShaderBindingArray);
  } else {
    UpdateWriteDescriptorSet(writeDescriptorSet, InShaderBindingArray);
  }

  vkUpdateDescriptorSets(
      g_rhi_vk->m_device_,
      static_cast<uint32_t>(writeDescriptorSet.DescriptorWrites.size()),
      writeDescriptorSet.DescriptorWrites.data(),
      0,
      nullptr);
}

void ShaderBindingInstanceVk::Free() {
  if (GetType() == ShaderBindingInstanceType::MultiFrame) {
    g_rhi_vk->GetDescriptorPoolMultiFrame()->Free(shared_from_this());
  }
}

// =========================================================

// ShaderBindingLayoutVk
// =========================================================

bool ShaderBindingLayoutVk::Initialize(
    const ShaderBindingArray& InShaderBindingArray) {
  InShaderBindingArray.CloneWithoutResource(shaderBindingArray);
  DescriptorSetLayout = CreateDescriptorSetLayout(shaderBindingArray);

  return !!DescriptorSetLayout;
}

std::shared_ptr<ShaderBindingInstance>
    ShaderBindingLayoutVk::CreateShaderBindingInstance(
        const ShaderBindingArray&       InShaderBindingArray,
        const ShaderBindingInstanceType InType) const {
  DescriptorPoolVk* DescriptorPool = nullptr;
  switch (InType) {
    case ShaderBindingInstanceType::SingleFrame:
      DescriptorPool = g_rhi_vk->GetDescriptorPoolForSingleFrame();
      break;
    case ShaderBindingInstanceType::MultiFrame:
      DescriptorPool = g_rhi_vk->GetDescriptorPoolMultiFrame();
      break;
    case ShaderBindingInstanceType::Max:
    default:
      assert(0);
      break;
  }

  std::shared_ptr<ShaderBindingInstance> DescriptorSet
      = DescriptorPool->AllocateDescriptorSet(DescriptorSetLayout);

  assert(DescriptorSet && "DescriptorSet allocation failed");

  if (!DescriptorSet) {
    DescriptorSet = DescriptorPool->AllocateDescriptorSet(DescriptorSetLayout);
    return nullptr;
  }

  DescriptorSet->ShaderBindingsLayouts = this;
  DescriptorSet->Initialize(InShaderBindingArray);
  DescriptorSet->SetType(InType);
  return DescriptorSet;
}

size_t ShaderBindingLayoutVk::GetHash() const {
  if (Hash) {
    return Hash;
  }

  Hash = shaderBindingArray.GetHash();
  return Hash;
}

void ShaderBindingLayoutVk::Release() {
  if (DescriptorSetLayout) {
    vkDestroyDescriptorSetLayout(
        g_rhi_vk->m_device_, DescriptorSetLayout, nullptr);
    DescriptorSetLayout = nullptr;
  }
}

std::vector<VkDescriptorPoolSize>
    ShaderBindingLayoutVk::GetDescriptorPoolSizeArray(
        uint32_t maxAllocations) const {
  std::vector<VkDescriptorPoolSize> resultArray;

  if (!shaderBindingArray.NumOfData) {
    uint32_t         NumOfSameType = 0;
    VkDescriptorType PrevType
        = GetVulkanShaderBindingType(shaderBindingArray[0]->BindingType);
    for (int32_t i = 0; i < shaderBindingArray.NumOfData; ++i) {
      if (PrevType
          == GetVulkanShaderBindingType(shaderBindingArray[i]->BindingType)) {
        ++NumOfSameType;
      } else {
        VkDescriptorPoolSize poolSize;
        poolSize.type            = PrevType;
        poolSize.descriptorCount = NumOfSameType * maxAllocations;
        resultArray.push_back(poolSize);

        PrevType
            = GetVulkanShaderBindingType(shaderBindingArray[i]->BindingType);
        NumOfSameType = 1;
      }
    }

    if (NumOfSameType > 0) {
      VkDescriptorPoolSize poolSize;
      poolSize.type            = PrevType;
      poolSize.descriptorCount = NumOfSameType * maxAllocations;
      resultArray.push_back(poolSize);
    }
  }

  return std::move(resultArray);
}

VkDescriptorSetLayout ShaderBindingLayoutVk::CreateDescriptorSetLayout(
    const ShaderBindingArray& InShaderBindingArray) {
  VkDescriptorSetLayout DescriptorSetLayout = nullptr;
  size_t                hash                = InShaderBindingArray.GetHash();
  assert(hash);
  {
    ScopeReadLock sr(&DescriptorLayoutPoolLock);
    auto          it_find = DescriptorLayoutPool.find(hash);
    if (DescriptorLayoutPool.end() != it_find) {
      DescriptorSetLayout = it_find->second;
      return DescriptorSetLayout;
    }
  }

  {
    ScopeWriteLock sw(&DescriptorLayoutPoolLock);

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.reserve(InShaderBindingArray.NumOfData);

    std::vector<VkDescriptorBindingFlagsEXT> bindingFlags;
    bindingFlags.reserve(InShaderBindingArray.NumOfData);

    int32_t                                   LastBindingIndex = 0;
    for (int32_t i = 0; i < (int32_t)InShaderBindingArray.NumOfData; ++i) {
      VkDescriptorSetLayoutBinding binding = {};
      if (InShaderBindingArray[i]->BindingPoint
          != ShaderBinding::APPEND_LAST) {
        binding.binding  = InShaderBindingArray[i]->BindingPoint;
        LastBindingIndex = InShaderBindingArray[i]->BindingPoint + 1;
      } else {
        binding.binding = LastBindingIndex++;
      }
      binding.descriptorType
          = GetVulkanShaderBindingType(InShaderBindingArray[i]->BindingType);
      binding.descriptorCount = InShaderBindingArray[i]->NumOfDescriptors;
      binding.stageFlags      = GetVulkanShaderAccessFlags(
          InShaderBindingArray[i]->AccessStageFlags);
      binding.pImmutableSamplers = nullptr;
      bindings.push_back(binding);
      bindingFlags.push_back(
          InShaderBindingArray[i]->Resource->IsBindless()
              ? (VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
                 | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT)
              : 0);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings    = bindings.data();

    // for bindless resources
    if (bindingFlags.size() > 0) {
      VkDescriptorSetLayoutBindingFlagsCreateInfoEXT setLayoutBindingFlags{};
      setLayoutBindingFlags.sType
          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
      setLayoutBindingFlags.bindingCount  = (uint32_t)bindingFlags.size();
      setLayoutBindingFlags.pBindingFlags = bindingFlags.data();
      layoutInfo.pNext                    = &setLayoutBindingFlags;
    }

    if (vkCreateDescriptorSetLayout(
            g_rhi_vk->m_device_, &layoutInfo, nullptr, &DescriptorSetLayout)
        != VK_SUCCESS) {
      GlobalLogger::Log(LogLevel::Error,
                        "Failed to create descriptor set layout");

      return nullptr;
    }

    DescriptorLayoutPool[hash] = DescriptorSetLayout;
  }

  return DescriptorSetLayout;
}

VkPipelineLayout ShaderBindingLayoutVk::CreatePipelineLayout(
    const ShaderBindingLayoutArray& InShaderBindingLayoutArray,
    const PushConstant*             pushConstant) {
  if (InShaderBindingLayoutArray.NumOfData <= 0) {
    GlobalLogger::Log(LogLevel::Warning,
                      "Shader binding layout array is empty");
    return 0;
  }

  VkPipelineLayout vkPipelineLayout = nullptr;
  size_t           hash             = InShaderBindingLayoutArray.GetHash();

  if (pushConstant) {
    hash = XXH64(pushConstant->GetHash(), hash);
  }
  assert(hash);

  {
    ScopeReadLock sr(&PipelineLayoutPoolLock);
    auto          it_find = PipelineLayoutPool.find(hash);
    if (PipelineLayoutPool.end() != it_find) {
      vkPipelineLayout = it_find->second;
      return vkPipelineLayout;
    }
  }

  {
    ScopeWriteLock sw(&PipelineLayoutPoolLock);

    // Try again, to avoid entering creation section simultaneously.
    auto it_find = PipelineLayoutPool.find(hash);
    if (PipelineLayoutPool.end() != it_find) {
      vkPipelineLayout = it_find->second;
      return vkPipelineLayout;
    }

    std::vector<VkDescriptorSetLayout> DescriptorSetLayouts;
    DescriptorSetLayouts.reserve(InShaderBindingLayoutArray.NumOfData);
    for (int32_t i = 0; i < InShaderBindingLayoutArray.NumOfData; ++i) {
      const ShaderBindingLayoutVk* binding_vulkan
          = (const ShaderBindingLayoutVk*)InShaderBindingLayoutArray[i];
      DescriptorSetLayouts.push_back(binding_vulkan->DescriptorSetLayout);
    }

    std::vector<VkPushConstantRange> PushConstantRanges;
    if (pushConstant) {
      const ResourceContainer<PushConstantRange>* pushConstantRanges
          = pushConstant->GetPushConstantRanges();
      assert(pushConstantRanges);
      if (pushConstantRanges) {
        PushConstantRanges.reserve(pushConstantRanges->NumOfData);
        for (int32_t i = 0; i < pushConstantRanges->NumOfData; ++i) {
          const PushConstantRange& range = (*pushConstantRanges)[i];

          VkPushConstantRange pushConstantRange{};
          pushConstantRange.stageFlags
              = GetVulkanShaderAccessFlags(range.AccessStageFlag);
          pushConstantRange.offset = range.Offset;
          pushConstantRange.size   = range.Size;
          PushConstantRanges.emplace_back(pushConstantRange);
        }
      }
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = (uint32_t)DescriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts    = &DescriptorSetLayouts[0];
    if (PushConstantRanges.size() > 0) {
      pipelineLayoutInfo.pushConstantRangeCount
          = (int32_t)PushConstantRanges.size();
      pipelineLayoutInfo.pPushConstantRanges = PushConstantRanges.data();
    }
    assert(g_rhi_vk->m_device_);
    if (vkCreatePipelineLayout(g_rhi_vk->m_device_,
                               &pipelineLayoutInfo,
                               nullptr,
                               &vkPipelineLayout)
        != VK_SUCCESS) {
      GlobalLogger::Log(LogLevel::Error, "Failed to create pipeline layout");
      return nullptr;
    }

    PipelineLayoutPool[hash] = vkPipelineLayout;
  }

  return vkPipelineLayout;
}

void ShaderBindingLayoutVk::ClearPipelineLayout() {
  assert(g_rhi_vk);
  {
    ScopeWriteLock s(&PipelineLayoutPoolLock);
    for (auto& iter : PipelineLayoutPool) {
      vkDestroyPipelineLayout(g_rhi_vk->m_device_, iter.second, nullptr);
    }
    PipelineLayoutPool.clear();
  }
}


}  // namespace game_engine
