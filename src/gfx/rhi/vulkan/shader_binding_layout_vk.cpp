
#include "gfx/rhi/vulkan/shader_binding_layout_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

MutexRWLock ShaderBindingLayoutVk::s_descriptorLayoutPoolLock;
std::unordered_map<uint64_t, VkDescriptorSetLayout>
            ShaderBindingLayoutVk::s_descriptorLayoutPool;
MutexRWLock ShaderBindingLayoutVk::s_pipelineLayoutPoolLock;
std::unordered_map<uint64_t, VkPipelineLayout>
    ShaderBindingLayoutVk::s_pipelineLayoutPool;

//// ShaderBindingVk
//// =========================================================
//size_t ShaderBindingVk::GetHash() const {
//  if (Hash) {
//    return Hash;
//  }
//
//  Hash = GETHASH_FROM_INSTANT_STRUCT(
//      m_isInline_, m_bindingPoint_, NumOfDescriptors, BindingType, m_accessStageFlags_);
//  return Hash;
//}
//
//void ShaderBindingVk::CloneWithoutResource(ShaderBindingVk& OutReslut) const {
//  OutReslut.m_isInline_         = m_isInline_;
//  OutReslut.m_bindingPoint_     = m_bindingPoint_;
//  OutReslut.NumOfDescriptors = NumOfDescriptors;
//  OutReslut.BindingType      = BindingType;
//  OutReslut.m_accessStageFlags_ = m_accessStageFlags_;
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
//const ShaderBindingVk* ShaderBindingArray::operator[](int32_t index) const {
//  assert(index < NumOfData);
//  return (ShaderBindingVk*)(&Data[index]);
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
  m_writeDescriptorInfos_.clear();
  m_descriptorWrites_.clear();
  m_dynamicOffsets_.clear();
  m_isInitialized_ = false;
}

void WriteDescriptorSet::SetWriteDescriptorInfo(
    int32_t index, const ShaderBinding* shaderBinding) {
  assert(shaderBinding);
  assert(shaderBinding->m_resource_);
  const bool IsBindless = shaderBinding->m_resource_->IsBindless();

  switch (shaderBinding->m_bindingType_) {
    case EShaderBindingType::UNIFORMBUFFER: {
      if (IsBindless) {
        const UniformBufferResourceBindless* ubor
            = reinterpret_cast<const UniformBufferResourceBindless*>(
                shaderBinding->m_resource_);
        assert(ubor);

        if (ubor) {
          for (auto UniformBuffer : ubor->m_uniformBuffers_) {
            assert(UniformBuffer);

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = (VkBuffer)UniformBuffer->GetLowLevelResource();
            bufferInfo.offset = UniformBuffer->GetBufferOffset();
            bufferInfo.range  = UniformBuffer->GetBufferSize();
            assert(bufferInfo.buffer);
            m_writeDescriptorInfos_.push_back(WriteDescriptorInfo(bufferInfo));
          }
        }
      } else {
        const UniformBufferResource* ubor
            = reinterpret_cast<const UniformBufferResource*>(
                shaderBinding->m_resource_);
        assert(ubor && ubor->m_uniformBuffer_);

        if (ubor && ubor->m_uniformBuffer_) {
          VkDescriptorBufferInfo bufferInfo{};
          bufferInfo.buffer
              = (VkBuffer)ubor->m_uniformBuffer_->GetLowLevelResource();
          bufferInfo.offset = ubor->m_uniformBuffer_->GetBufferOffset();
          bufferInfo.range  = ubor->m_uniformBuffer_->GetBufferSize();
          assert(bufferInfo.buffer);
          m_writeDescriptorInfos_.push_back(WriteDescriptorInfo(bufferInfo));
        }
      }
      break;
    }
    case EShaderBindingType::UNIFORMBUFFER_DYNAMIC: {
      if (IsBindless) {
        const UniformBufferResourceBindless* ubor
            = reinterpret_cast<const UniformBufferResourceBindless*>(
                shaderBinding->m_resource_);
        assert(ubor);

        if (ubor) {
          for (auto UniformBuffer : ubor->m_uniformBuffers_) {
            assert(UniformBuffer);

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = (VkBuffer)UniformBuffer->GetLowLevelResource();
            bufferInfo.offset = 0;  // TODO: Use DynamicOffset instead
            bufferInfo.range  = UniformBuffer->GetBufferSize();
            assert(bufferInfo.buffer);
            m_writeDescriptorInfos_.push_back(WriteDescriptorInfo(bufferInfo));
            m_dynamicOffsets_.push_back(
                (uint32_t)UniformBuffer->GetBufferOffset());
          }
        }
      } else {
        const UniformBufferResource* ubor
            = reinterpret_cast<const UniformBufferResource*>(
                shaderBinding->m_resource_);
        assert(ubor && ubor->m_uniformBuffer_);

        if (ubor && ubor->m_uniformBuffer_) {
          VkDescriptorBufferInfo bufferInfo{};
          bufferInfo.buffer
              = (VkBuffer)ubor->m_uniformBuffer_->GetLowLevelResource();
          bufferInfo.offset = 0;  // TODO: Use DynamicOffset instead
          bufferInfo.range  = ubor->m_uniformBuffer_->GetBufferSize();
          assert(bufferInfo.buffer);
          m_writeDescriptorInfos_.push_back(WriteDescriptorInfo(bufferInfo));
          m_dynamicOffsets_.push_back(
              (uint32_t)ubor->m_uniformBuffer_->GetBufferOffset());
        }
      }
      break;
    }
    case EShaderBindingType::TEXTURE_SAMPLER_SRV:
    case EShaderBindingType::TEXTURE_SRV: {
      if (IsBindless) {
        const TextureResourceBindless* tbor
            = reinterpret_cast<const TextureResourceBindless*>(
                shaderBinding->m_resource_);
        assert(tbor);

        if (tbor) {
          for (auto TextureData : tbor->m_textureBindDatas_) {
            assert(TextureData.m_texture);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout
                = TextureData.m_texture->IsDepthFormat()
                    ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
                    : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView
                = ((const TextureVk*)TextureData.m_texture)->m_imageView_;
            imageInfo.sampler
                = TextureData.m_samplerState_
                    ? (VkSampler)TextureData.m_samplerState_->GetHandle()
                    : nullptr;
            if (!imageInfo.sampler) {
              imageInfo.sampler
                  = TextureVk::CreateDefaultSamplerState();  // todo
            }
            assert(imageInfo.imageView);
            m_writeDescriptorInfos_.push_back(WriteDescriptorInfo(imageInfo));
          }
        }
      } else {
        const TextureResource* tbor = reinterpret_cast<const TextureResource*>(
            shaderBinding->m_resource_);
        assert(tbor && tbor->m_texture);

        if (tbor && tbor->m_texture) {
          VkDescriptorImageInfo imageInfo{};
          imageInfo.imageLayout
              = tbor->m_texture->IsDepthFormat()
                  ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
                  : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
          imageInfo.imageView = ((const TextureVk*)tbor->m_texture)->m_imageView_;
          imageInfo.sampler   = tbor->m_samplerState_
                                  ? (VkSampler)tbor->m_samplerState_->GetHandle()
                                  : nullptr;
          if (!imageInfo.sampler) {
            imageInfo.sampler = TextureVk::CreateDefaultSamplerState();  // todo
          }
          assert(imageInfo.imageView);
          m_writeDescriptorInfos_.push_back(WriteDescriptorInfo(imageInfo));
        }
      }
      break;
    }
    case EShaderBindingType::TEXTURE_ARRAY_SRV: {
      const TextureArrayResource* tbor
          = reinterpret_cast<const TextureArrayResource*>(
              shaderBinding->m_resource_);
      assert(tbor && tbor->m_textureArray_);
      if (tbor && tbor->m_textureArray_) {
        // TODO: Implement
        assert(0);
      }
      break;
    }
    case EShaderBindingType::SUBPASS_INPUT_ATTACHMENT: {
      if (IsBindless) {
        const TextureResourceBindless* tbor
            = reinterpret_cast<const TextureResourceBindless*>(
                shaderBinding->m_resource_);
        assert(tbor);

        if (tbor) {
          for (auto TextureData : tbor->m_textureBindDatas_) {
            assert(TextureData.m_texture);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout
                = TextureData.m_texture->IsDepthFormat()
                    ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
                    : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView
                = ((const TextureVk*)TextureData.m_texture)->m_imageView_;
            imageInfo.sampler
                = TextureData.m_samplerState_
                    ? (VkSampler)TextureData.m_samplerState_->GetHandle()
                    : nullptr;
            if (!imageInfo.sampler) {
              imageInfo.sampler
                  = TextureVk::CreateDefaultSamplerState();  // todo
            }
            assert(imageInfo.imageView);
            m_writeDescriptorInfos_.push_back(WriteDescriptorInfo(imageInfo));
          }
        }
      } else {
        const TextureResource* tbor = reinterpret_cast<const TextureResource*>(
            shaderBinding->m_resource_);
        assert(tbor && tbor->m_texture);

        if (tbor && tbor->m_texture) {
          VkDescriptorImageInfo imageInfo{};
          imageInfo.imageLayout
              = (tbor->m_texture->IsDepthFormat()
                     ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
                     : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
          imageInfo.imageView = ((const TextureVk*)tbor->m_texture)->m_imageView_;
          assert(imageInfo.imageView);
          m_writeDescriptorInfos_.push_back(WriteDescriptorInfo(imageInfo));
        }
      }
      break;
    }
    // Texture UAV
    case EShaderBindingType::TEXTURE_UAV: {
      // TODO: log error
      assert(0);
      // TODO: WIP
      // if (m_isBindless_) {
      //  const TextureResourceBindless* tbor
      //      = reinterpret_cast<const TextureResourceBindless*>(
      //          shaderBinding->Resource);
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
      //          shaderBinding->Resource);
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
                shaderBinding->m_resource_);
        assert(sr);

        if (sr) {
          for (auto SamplerState : sr->m_samplerStates_) {
            assert(SamplerState);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.sampler     = (VkSampler)SamplerState->GetHandle();
            assert(imageInfo.sampler);
            m_writeDescriptorInfos_.push_back(WriteDescriptorInfo(imageInfo));
          }
        }
      } else {
        const SamplerResource* sr = reinterpret_cast<const SamplerResource*>(
            shaderBinding->m_resource_);
        assert(sr && sr->m_samplerState_);

        if (sr && sr->m_samplerState_) {
          VkDescriptorImageInfo imageInfo{};
          imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
          imageInfo.sampler     = (VkSampler)sr->m_samplerState_->GetHandle();
          assert(imageInfo.sampler);
          m_writeDescriptorInfos_.push_back(WriteDescriptorInfo(imageInfo));
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
                shaderBinding->m_resource_);
        for (auto buffer : br->m_buffers) {
          assert(buffer);

          VkDescriptorBufferInfo bufferInfo{};
          bufferInfo.buffer = (VkBuffer)buffer->GetHandle();
          bufferInfo.offset = buffer->GetOffset();
          bufferInfo.range  = buffer->GetBufferSize();
          assert(bufferInfo.buffer);
          m_writeDescriptorInfos_.push_back(WriteDescriptorInfo(bufferInfo));
        }
      } else {
        const BufferResource* br = reinterpret_cast<const BufferResource*>(
            shaderBinding->m_resource_);
        assert(br && br->m_buffer);
        if (br && br->m_buffer) {
          VkDescriptorBufferInfo bufferInfo{};
          bufferInfo.buffer = (VkBuffer)br->m_buffer->GetHandle();
          bufferInfo.offset = br->m_buffer->GetOffset();
          bufferInfo.range  = br->m_buffer->GetBufferSize();
          assert(bufferInfo.buffer);
          m_writeDescriptorInfos_.push_back(WriteDescriptorInfo(bufferInfo));
        }
      }
      break;
    }
    // Not needed for now (RTX stuff)
    /*case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: {
      if (m_isBindless_) {
        const BufferResourceBindless* br
            = reinterpret_cast<const BufferResourceBindless*>(
                shaderBinding->Resource);
        for (auto m_buffer : br->m_buffers) {
          assert(m_buffer);

          VkWriteDescriptorSetAccelerationStructureKHR
              AccelerationStructureKHR{};
          AccelerationStructureKHR.sType
              =
    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
          AccelerationStructureKHR.accelerationStructureCount = 1;
          AccelerationStructureKHR.pAccelerationStructures
              = &((BufferVk*)m_buffer)->AccelerationStructure;
          assert(AccelerationStructureKHR.pAccelerationStructures);
          WriteDescriptorInfos.push_back(
              WriteDescriptorInfo(AccelerationStructureKHR));
        }
      } else {
        const BufferResource* br = reinterpret_cast<const BufferResource*>(
            shaderBinding->Resource);
        assert(br && br->m_buffer);
        if (br && br->m_buffer) {
          VkWriteDescriptorSetAccelerationStructureKHR
              AccelerationStructureKHR{};
          AccelerationStructureKHR.sType
              =
    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
          AccelerationStructureKHR.accelerationStructureCount = 1;
          AccelerationStructureKHR.pAccelerationStructures
              = &((BufferVk*)br->m_buffer)->AccelerationStructure;
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

// ShaderBindingInstance
// =========================================================

void ShaderBindingInstanceVk::CreateWriteDescriptorSet(
    WriteDescriptorSet&        OutDescriptorWrites,
    const VkDescriptorSet      InDescriptorSet,
    const ShaderBindingArray& shaderBindingArray) {
  //  assert(shaderBindingArray.NumOfData);
  //
  //  OutDescriptorWrites.Reset();
  //
  //  std::vector<WriteDescriptorInfo>& descriptors
  //      = OutDescriptorWrites.WriteDescriptorInfos;
  //  std::vector<VkWriteDescriptorSet>& descriptorWrites
  //      = OutDescriptorWrites.DescriptorWrites;
  //  descriptors.resize(shaderBindingArray.NumOfData);
  //  descriptorWrites.resize(shaderBindingArray.NumOfData);
  //  OutDescriptorWrites.DynamicOffsets.clear();
  //  OutDescriptorWrites.DynamicOffsets.reserve(shaderBindingArray.NumOfData);
  //
  //  for (int32_t i = 0; i < shaderBindingArray.NumOfData; ++i) {
  //    OutDescriptorWrites.SetWriteDescriptorInfo(i, shaderBindingArray[i]);
  //
  //    VkWriteDescriptorSet& CurDescriptorWrite = descriptorWrites[i];
  //    CurDescriptorWrite.sType           =
  //    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; CurDescriptorWrite.dstSet =
  //    InDescriptorSet; CurDescriptorWrite.dstBinding      = i;
  //    CurDescriptorWrite.dstArrayElement = 0;
  //    CurDescriptorWrite.descriptorType
  //        = GetVulkanShaderBindingType(shaderBindingArray[i]->BindingType);
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
  if (!(shaderBindingArray.m_numOfData_)) {
    // TODO: log error
    assert(0);
    return;
  }

  OutDescriptorWrites.Reset();

  std::vector<WriteDescriptorInfo>& descriptors
      = OutDescriptorWrites.m_writeDescriptorInfos_;
  std::vector<VkWriteDescriptorSet>& descriptorWrites
      = OutDescriptorWrites.m_descriptorWrites_;
  std::vector<uint32_t>& dynamicOffsets = OutDescriptorWrites.m_dynamicOffsets_;

  int32_t LastIndex = 0;
  for (int32_t i = 0; i < shaderBindingArray.m_numOfData_; ++i) {
    OutDescriptorWrites.SetWriteDescriptorInfo(i, shaderBindingArray[i]);
    descriptorWrites.resize(descriptors.size());

    for (int32_t k = LastIndex; k < descriptorWrites.size(); ++k) {
      VkWriteDescriptorSet& CurDescriptorWrite = descriptorWrites[k];
      CurDescriptorWrite.sType      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      CurDescriptorWrite.dstSet     = InDescriptorSet;
      CurDescriptorWrite.dstBinding = i;
      CurDescriptorWrite.dstArrayElement = k - LastIndex;
      CurDescriptorWrite.descriptorType
          = GetVulkanShaderBindingType(shaderBindingArray[i]->m_bindingType_);
      CurDescriptorWrite.descriptorCount = 1;
    }
    LastIndex = (int32_t)descriptorWrites.size();
  }

  assert(descriptors.size() == descriptorWrites.size());
  for (int32_t i = 0; i < (int32_t)descriptorWrites.size(); ++i) {
    const WriteDescriptorInfo& WriteDescriptorInfo = descriptors[i];
    VkWriteDescriptorSet&      CurDescriptorWrite  = descriptorWrites[i];
    if (WriteDescriptorInfo.m_bufferInfo_.buffer) {
      CurDescriptorWrite.pBufferInfo
          = &WriteDescriptorInfo
                 .m_bufferInfo_;  // m_buffer should be bound in pBufferInfo
    } else if (WriteDescriptorInfo.m_imageInfo_.imageView
               || WriteDescriptorInfo.m_imageInfo_.sampler) {
      CurDescriptorWrite.pImageInfo
          = &WriteDescriptorInfo
                 .m_imageInfo_;  // Image should be bound in pImageInfo
    } else {
      assert(0);
    }
  }

  OutDescriptorWrites.m_isInitialized_ = true;
}

void ShaderBindingInstanceVk::UpdateWriteDescriptorSet(
    WriteDescriptorSet&        OutDescriptorWrites,
    const ShaderBindingArray& shaderBindingArray) {
  assert(shaderBindingArray.m_numOfData_
         == OutDescriptorWrites.m_descriptorWrites_.size());

  OutDescriptorWrites.m_dynamicOffsets_.clear();
  for (int32_t i = 0; i < shaderBindingArray.m_numOfData_; ++i) {
    OutDescriptorWrites.SetWriteDescriptorInfo(i, shaderBindingArray[i]);
  }
}

void ShaderBindingInstanceVk::Initialize(
    const ShaderBindingArray& shaderBindingArray) {
  // if (!writeDescriptorSet.IsInitialized) {
  CreateWriteDescriptorSet(
      m_writeDescriptorSet_, m_descriptorSet_, shaderBindingArray);
  // TODO: currently not working
  //} else {
  //  UpdateWriteDescriptorSet(writeDescriptorSet, shaderBindingArray);
  //}

  vkUpdateDescriptorSets(
      g_rhi_vk->m_device_,
      static_cast<uint32_t>(m_writeDescriptorSet_.m_descriptorWrites_.size()),
      m_writeDescriptorSet_.m_descriptorWrites_.data(),
      0,
      nullptr);
}

void ShaderBindingInstanceVk::UpdateShaderBindings(
    const ShaderBindingArray& shaderBindingArray) {
  assert(ShaderBindingsLayouts->GetShaderBindingsLayout().m_numOfData_
         == shaderBindingArray.m_numOfData_);
  assert(shaderBindingArray.m_numOfData_);

  if (!m_writeDescriptorSet_.m_isInitialized_) {
    CreateWriteDescriptorSet(
        m_writeDescriptorSet_, m_descriptorSet_, shaderBindingArray);
  } else {
    UpdateWriteDescriptorSet(m_writeDescriptorSet_, shaderBindingArray);
  }

  vkUpdateDescriptorSets(
      g_rhi_vk->m_device_,
      static_cast<uint32_t>(m_writeDescriptorSet_.m_descriptorWrites_.size()),
      m_writeDescriptorSet_.m_descriptorWrites_.data(),
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
    const ShaderBindingArray& shaderBindingArray) {
  shaderBindingArray.CloneWithoutResource(m_shaderBindingArray_);
  m_descriptorSetLayout_ = CreateDescriptorSetLayout(m_shaderBindingArray_);

  return !!m_descriptorSetLayout_;
}

std::shared_ptr<ShaderBindingInstance>
    ShaderBindingLayoutVk::CreateShaderBindingInstance(
        const ShaderBindingArray&       shaderBindingArray,
        const ShaderBindingInstanceType type) const {
  DescriptorPoolVk* DescriptorPool = nullptr;
  switch (type) {
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
      = DescriptorPool->AllocateDescriptorSet(m_descriptorSetLayout_);

  assert(DescriptorSet && "DescriptorSet allocation failed");

  if (!DescriptorSet) {
    DescriptorSet = DescriptorPool->AllocateDescriptorSet(m_descriptorSetLayout_);
    return nullptr;
  }

  DescriptorSet->m_shaderBindingsLayouts_ = this;
  DescriptorSet->Initialize(shaderBindingArray);
  DescriptorSet->SetType(type);
  return DescriptorSet;
}

size_t ShaderBindingLayoutVk::GetHash() const {
  if (m_hash_) {
    return m_hash_;
  }

  m_hash_ = m_shaderBindingArray_.GetHash();
  return m_hash_;
}

void ShaderBindingLayoutVk::Release() {
  if (m_descriptorSetLayout_) {
    vkDestroyDescriptorSetLayout(
        g_rhi_vk->m_device_, m_descriptorSetLayout_, nullptr);
    m_descriptorSetLayout_ = nullptr;
  }
}

std::vector<VkDescriptorPoolSize>
    ShaderBindingLayoutVk::GetDescriptorPoolSizeArray(
        uint32_t maxAllocations) const {
  std::vector<VkDescriptorPoolSize> resultArray;

  if (!m_shaderBindingArray_.m_numOfData_) {
    uint32_t         NumOfSameType = 0;
    VkDescriptorType PrevType
        = GetVulkanShaderBindingType(m_shaderBindingArray_[0]->m_bindingType_);
    for (int32_t i = 0; i < m_shaderBindingArray_.m_numOfData_; ++i) {
      if (PrevType
          == GetVulkanShaderBindingType(m_shaderBindingArray_[i]->m_bindingType_)) {
        ++NumOfSameType;
      } else {
        VkDescriptorPoolSize poolSize;
        poolSize.type            = PrevType;
        poolSize.descriptorCount = NumOfSameType * maxAllocations;
        resultArray.push_back(poolSize);

        PrevType
            = GetVulkanShaderBindingType(m_shaderBindingArray_[i]->m_bindingType_);
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
    const ShaderBindingArray& shaderBindingArray) {
  VkDescriptorSetLayout DescriptorSetLayout = nullptr;
  size_t                hash                = shaderBindingArray.GetHash();
  assert(hash);
  {
    ScopeReadLock sr(&s_descriptorLayoutPoolLock);
    auto          it_find = s_descriptorLayoutPool.find(hash);
    if (s_descriptorLayoutPool.end() != it_find) {
      DescriptorSetLayout = it_find->second;
      return DescriptorSetLayout;
    }
  }

  {
    ScopeWriteLock sw(&s_descriptorLayoutPoolLock);

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.reserve(shaderBindingArray.m_numOfData_);

    std::vector<VkDescriptorBindingFlagsEXT> bindingFlags;
    bindingFlags.reserve(shaderBindingArray.m_numOfData_);

    int32_t                                   LastBindingIndex = 0;
    for (int32_t i = 0; i < (int32_t)shaderBindingArray.m_numOfData_; ++i) {
      VkDescriptorSetLayoutBinding binding = {};
      if (shaderBindingArray[i]->m_bindingPoint_
          != ShaderBinding::s_kAppendLast) {
        binding.binding  = shaderBindingArray[i]->m_bindingPoint_;
        LastBindingIndex = shaderBindingArray[i]->m_bindingPoint_ + 1;
      } else {
        binding.binding = LastBindingIndex++;
      }
      binding.descriptorType
          = GetVulkanShaderBindingType(shaderBindingArray[i]->m_bindingType_);
      binding.descriptorCount = shaderBindingArray[i]->m_numOfDescriptors_;
      binding.stageFlags      = GetVulkanShaderAccessFlags(
          shaderBindingArray[i]->m_accessStageFlags_);
      binding.pImmutableSamplers = nullptr;
      bindings.push_back(binding);
      bindingFlags.push_back(
          shaderBindingArray[i]->m_resource_->IsBindless()
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

    s_descriptorLayoutPool[hash] = DescriptorSetLayout;
  }

  return DescriptorSetLayout;
}

VkPipelineLayout ShaderBindingLayoutVk::CreatePipelineLayout(
    const ShaderBindingLayoutArray& shaderBindingLayoutArray,
    const PushConstant*             pushConstant) {
  if (shaderBindingLayoutArray.m_numOfData_ <= 0) {
    GlobalLogger::Log(LogLevel::Warning,
                      "Shader binding layout array is empty");
    return 0;
  }

  VkPipelineLayout vkPipelineLayout = nullptr;
  size_t           hash             = shaderBindingLayoutArray.GetHash();

  if (pushConstant) {
    hash = XXH64(pushConstant->GetHash(), hash);
  }
  assert(hash);

  {
    ScopeReadLock sr(&s_pipelineLayoutPoolLock);
    auto          it_find = s_pipelineLayoutPool.find(hash);
    if (s_pipelineLayoutPool.end() != it_find) {
      vkPipelineLayout = it_find->second;
      return vkPipelineLayout;
    }
  }

  {
    ScopeWriteLock sw(&s_pipelineLayoutPoolLock);

    // Try again, to avoid entering creation section simultaneously.
    auto it_find = s_pipelineLayoutPool.find(hash);
    if (s_pipelineLayoutPool.end() != it_find) {
      vkPipelineLayout = it_find->second;
      return vkPipelineLayout;
    }

    std::vector<VkDescriptorSetLayout> DescriptorSetLayouts;
    DescriptorSetLayouts.reserve(shaderBindingLayoutArray.m_numOfData_);
    for (int32_t i = 0; i < shaderBindingLayoutArray.m_numOfData_; ++i) {
      const ShaderBindingLayoutVk* binding_vulkan
          = (const ShaderBindingLayoutVk*)shaderBindingLayoutArray[i];
      DescriptorSetLayouts.push_back(binding_vulkan->m_descriptorSetLayout_);
    }

    std::vector<VkPushConstantRange> PushConstantRanges;
    if (pushConstant) {
      const ResourceContainer<PushConstantRange>* pushConstantRanges
          = pushConstant->GetPushConstantRanges();
      assert(pushConstantRanges);
      if (pushConstantRanges) {
        PushConstantRanges.reserve(pushConstantRanges->m_numOfData_);
        for (int32_t i = 0; i < pushConstantRanges->m_numOfData_; ++i) {
          const PushConstantRange& range = (*pushConstantRanges)[i];

          VkPushConstantRange pushConstantRange{};
          pushConstantRange.stageFlags
              = GetVulkanShaderAccessFlags(range.m_accessStageFlag_);
          pushConstantRange.offset = range.m_offset_;
          pushConstantRange.size   = range.m_size_;
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

    s_pipelineLayoutPool[hash] = vkPipelineLayout;
  }

  return vkPipelineLayout;
}

void ShaderBindingLayoutVk::ClearPipelineLayout() {
  assert(g_rhi_vk);
  {
    ScopeWriteLock s(&s_pipelineLayoutPoolLock);
    for (auto& iter : s_pipelineLayoutPool) {
      vkDestroyPipelineLayout(g_rhi_vk->m_device_, iter.second, nullptr);
    }
    s_pipelineLayoutPool.clear();
  }
}


}  // namespace game_engine
