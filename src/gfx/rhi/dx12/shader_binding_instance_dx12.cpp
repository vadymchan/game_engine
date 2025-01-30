#include "gfx/rhi/dx12/shader_binding_instance_dx12.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "gfx/rhi/dx12/command_list_dx12.h"
#include "gfx/rhi/dx12/rhi_dx12.h"
#include "gfx/rhi/dx12/rhi_type_dx12.h"
#include "gfx/rhi/dx12/uniform_buffer_block_dx12.h"

namespace game_engine {

void ShaderBindingInstanceDx12::initialize(
    const ShaderBindingArray& shaderBindingArray) {
  updateShaderBindings(shaderBindingArray);
}

void ShaderBindingInstanceDx12::updateShaderBindings(
    const ShaderBindingArray& shaderBindingArray) {
  // Let's copy the descriptors to the online descriptor set.
  // CopySimpleDescriptor
  // Let's find the descriptor layout and bind it.
  // Should we copy from the layout and actually create the root signature?
  // Let's think about this part.

  assert(m_shaderBindingsLayouts_);
  assert(m_shaderBindingsLayouts_->getShaderBindingsLayout().m_numOfData_
         == shaderBindingArray.m_numOfData_);

  m_descriptors_.clear();
  m_samplerDescriptors_.clear();
  m_rootParameterInlines_.clear();

  for (int32_t i = 0; i < shaderBindingArray.m_numOfData_; ++i) {
    const ShaderBinding* shaderBinding = shaderBindingArray[i];
    assert(shaderBinding);
    assert(shaderBinding->m_resource_);
    assert(shaderBinding->m_isBindless_
           == shaderBinding->m_resource_->isBindless());

    const bool IsBindless = shaderBinding->m_isBindless_;
    assert((IsBindless && !shaderBinding->m_isInline_)
           || !IsBindless);  // Bindless must note be inline

    switch (shaderBinding->m_bindingType_) {
      case EShaderBindingType::UNIFORMBUFFER:
      case EShaderBindingType::UNIFORMBUFFER_DYNAMIC: {
        if (IsBindless) {
          auto UniformResourceBindless
              = (UniformBufferResourceBindless*)shaderBinding->m_resource_;
          for (auto Resource : UniformResourceBindless->m_uniformBuffers_) {
            assert(Resource);

            UniformBufferBlockDx12* UniformBuffer
                = (UniformBufferBlockDx12*)Resource;
            m_descriptors_.push_back(
                {.m_descriptor_   = UniformBuffer->getCBV(),
                 .m_resourceName_ = UniformBuffer->m_resourceName_,
                 .m_resource_     = UniformBuffer});
          }
        } else {
          UniformBufferBlockDx12* UniformBuffer
              = (UniformBufferBlockDx12*)
                    shaderBinding->m_resource_->getResource();
          assert(UniformBuffer->getLowLevelResource());
          // assert(!m_uniformBuffer_->isUseRingBuffer() ||
          // (m_uniformBuffer_->isUseRingBuffer() &&
          // shaderBinding->m_isInline_));
          if (shaderBinding->m_isInline_) {
            m_rootParameterInlines_.push_back(
                {.m_type_              = InlineRootParamType::CBV,
                 .m_gpuVirtualAddress_ = UniformBuffer->getGPUAddress(),
                 .m_resourceName_      = UniformBuffer->m_resourceName_,
                 .m_resource_          = UniformBuffer});
          } else {
            m_descriptors_.push_back(
                {.m_descriptor_   = UniformBuffer->getCBV(),
                 .m_resourceName_ = UniformBuffer->m_resourceName_,
                 .m_resource_     = UniformBuffer});
          }
        }
        break;
      }
      case EShaderBindingType::TEXTURE_SAMPLER_SRV: {
        if (IsBindless) {
          auto TextureResourceResourceBindless
              = (TextureResourceBindless*)shaderBinding->m_resource_;
          for (auto Resource :
               TextureResourceResourceBindless->m_textureBindDatas_) {
            assert(Resource.m_texture);

            auto TexDX12
                = std::static_pointer_cast<TextureDx12>(Resource.m_texture);
            m_descriptors_.push_back(
                {.m_descriptor_   = TexDX12->m_srv_,
                 .m_resourceName_ = TexDX12->m_resourceName_,
                 .m_resource_     = TexDX12.get()});

            if (Resource.m_samplerState_) {
              SamplerStateInfoDx12* SamplerDX12
                  = (SamplerStateInfoDx12*)Resource.m_samplerState_;
              assert(SamplerDX12);
              m_samplerDescriptors_.push_back(
                  {.m_descriptor_   = SamplerDX12->m_samplerSRV_,
                   .m_resourceName_ = SamplerDX12->m_resourceName_,
                   .m_resource_     = SamplerDX12});
            } else {
              // assert(0);   // todo : need to set DefaultSamplerState
              const SamplerStateInfoDx12* SamplerDX12 = (SamplerStateInfoDx12*)
                  TSamplerStateInfo<ETextureFilter::LINEAR_MIPMAP_LINEAR,
                                    ETextureFilter::LINEAR_MIPMAP_LINEAR,
                                    ETextureAddressMode::REPEAT,
                                    ETextureAddressMode::REPEAT,
                                    ETextureAddressMode::REPEAT,
                                    0.0f,
                                    16.0f>::s_create();
              assert(SamplerDX12);
              m_samplerDescriptors_.push_back(
                  {.m_descriptor_   = SamplerDX12->m_samplerSRV_,
                   .m_resourceName_ = SamplerDX12->m_resourceName_,
                   .m_resource_     = SamplerDX12});
            }
          }
        } else {
          const TextureResource* tbor
              = reinterpret_cast<const TextureResource*>(
                  shaderBinding->m_resource_);
          assert(tbor && tbor->m_texture_);

          if (tbor && tbor->m_texture_) {
            auto           TexDX12 = (TextureDx12*)tbor->m_texture_;
            DescriptorData descriptorData;
            descriptorData.m_descriptor_   = TexDX12->m_srv_;
            descriptorData.m_resourceName_ = TexDX12->m_resourceName_;
            descriptorData.m_resource_     = TexDX12;
            m_descriptors_.push_back(descriptorData);
            // m_descriptors_.push_back(
            //     {.m_descriptor_   = TexDX12->m_srv_,
            //      .m_resourceName_ = TexDX12->m_resourceName_,
            //      .m_resource_     = TexDX12});

            if (tbor->m_samplerState_) {
              SamplerStateInfoDx12* SamplerDX12
                  = (SamplerStateInfoDx12*)tbor->m_samplerState_;
              assert(SamplerDX12);
              m_samplerDescriptors_.push_back(
                  {.m_descriptor_   = SamplerDX12->m_samplerSRV_,
                   .m_resourceName_ = SamplerDX12->m_resourceName_,
                   .m_resource_     = SamplerDX12});
            } else {
              // assert(0);   // TODO: need to set DefaultSamplerState

              const SamplerStateInfoDx12* SamplerDX12 = (SamplerStateInfoDx12*)
                  TSamplerStateInfo<ETextureFilter::LINEAR_MIPMAP_LINEAR,
                                    ETextureFilter::LINEAR_MIPMAP_LINEAR,
                                    ETextureAddressMode::REPEAT,
                                    ETextureAddressMode::REPEAT,
                                    ETextureAddressMode::REPEAT,
                                    0.0f,
                                    16.0f>::s_create();
              assert(SamplerDX12);
              m_samplerDescriptors_.push_back(
                  {.m_descriptor_   = SamplerDX12->m_samplerSRV_,
                   .m_resourceName_ = SamplerDX12->m_resourceName_,
                   .m_resource_     = SamplerDX12});
            }
          }
        }
        break;
      }
      case EShaderBindingType::TEXTURE_SRV: {
        if (IsBindless) {
          auto TextureResourceResourceBindless
              = (TextureResourceBindless*)shaderBinding->m_resource_;
          for (auto Resource :
               TextureResourceResourceBindless->m_textureBindDatas_) {
            auto Tex
                = std::static_pointer_cast<TextureDx12>(Resource.m_texture);
            m_descriptors_.push_back({.m_descriptor_   = Tex->m_srv_,
                                      .m_resourceName_ = Tex->m_resourceName_,
                                      .m_resource_     = Tex.get()});
          }
        } else {
          auto Tex = (TextureDx12*)shaderBinding->m_resource_->getResource();
          m_descriptors_.push_back({.m_descriptor_   = Tex->m_srv_,
                                    .m_resourceName_ = Tex->m_resourceName_,
                                    .m_resource_     = Tex});
        }
        break;
      }
      case EShaderBindingType::TEXTURE_ARRAY_SRV: {
        if (IsBindless) {
          auto TextureResourceResourceArrayBindless
              = (TextureArrayResourceBindless*)shaderBinding->m_resource_;
          for (auto Resource :
               TextureResourceResourceArrayBindless->m_textureArrayBindDatas_) {
            TextureDx12** TexArray = (TextureDx12**)Resource.m_textureArray_;
            for (int32_t i = 0; i < Resource.m_numOfTexure_; ++i) {
              assert(TexArray[i]);
              m_descriptors_.push_back(
                  {.m_descriptor_   = TexArray[i]->m_srv_,
                   .m_resourceName_ = TexArray[i]->m_resourceName_,
                   .m_resource_     = TexArray[i]});
            }
          }
        } else {
          TextureDx12** Tex
              = (TextureDx12**)shaderBinding->m_resource_->getResource();
          for (int32_t i = 0; i < shaderBinding->m_resource_->numOfResource();
               ++i) {
            assert(Tex[i]);
            m_descriptors_.push_back(
                {.m_descriptor_   = Tex[i]->m_srv_,
                 .m_resourceName_ = Tex[i]->m_resourceName_,
                 .m_resource_     = Tex[i]});
          }
        }
        break;
      }
      case EShaderBindingType::BUFFER_SRV:
      case EShaderBindingType::BUFFER_TEXEL_SRV:
      case EShaderBindingType::ACCELERATION_STRUCTURE_SRV: {
        if (IsBindless) {
          auto bufferResourceBindless
              = (BufferResourceBindless*)shaderBinding->m_resource_;
          for (auto Resource : bufferResourceBindless->m_buffers) {
            BufferDx12* Buf = (BufferDx12*)Resource;
            m_descriptors_.push_back({.m_descriptor_   = Buf->m_srv_,
                                      .m_resourceName_ = Buf->m_resourceName_,
                                      .m_resource_     = Buf});
          }
        } else {
          BufferDx12* Buf
              = (BufferDx12*)shaderBinding->m_resource_->getResource();
          assert(Buf->m_buffer->m_resource_);

          if (shaderBinding->m_isInline_) {
            m_rootParameterInlines_.push_back(
                {.m_type_              = InlineRootParamType::SRV,
                 .m_gpuVirtualAddress_ = Buf->getGPUAddress(),
                 .m_resourceName_      = Buf->m_resourceName_,
                 .m_resource_          = Buf});
          } else {
            m_descriptors_.push_back({.m_descriptor_   = Buf->m_srv_,
                                      .m_resourceName_ = Buf->m_resourceName_,
                                      .m_resource_     = Buf});
          }
        }
        break;
      }
      case EShaderBindingType::TEXTURE_UAV: {
        if (IsBindless) {
          auto TextureResourceResourceBindless
              = (TextureResourceBindless*)shaderBinding->m_resource_;
          for (auto Resource :
               TextureResourceResourceBindless->m_textureBindDatas_) {
            auto Tex
                = std::static_pointer_cast<TextureDx12>(Resource.m_texture);
            if (Resource.m_mipLevel_ == 0) {
              m_descriptors_.push_back({.m_descriptor_   = Tex->m_uav_,
                                        .m_resourceName_ = Tex->m_resourceName_,
                                        .m_resource_     = Tex.get()});
            } else {
              auto it_find = Tex->m_uavMipMap.find(Resource.m_mipLevel_);
              if (it_find != Tex->m_uavMipMap.end()
                  && it_find->second.isValid()) {
                m_descriptors_.push_back(
                    {.m_descriptor_   = it_find->second,
                     .m_resourceName_ = Tex->m_resourceName_,
                     .m_resource_     = Tex.get()});
              } else {
                m_descriptors_.push_back(
                    {.m_descriptor_   = Tex->m_uav_,
                     .m_resourceName_ = Tex->m_resourceName_,
                     .m_resource_     = Tex.get()});
              }
            }
          }
        } else {
          auto Tex = (TextureDx12*)shaderBinding->m_resource_->getResource();
          const TextureResource* tbor
              = reinterpret_cast<const TextureResource*>(
                  shaderBinding->m_resource_);
          if (tbor->kMipLevel == 0) {
            m_descriptors_.push_back({.m_descriptor_   = Tex->m_uav_,
                                      .m_resourceName_ = Tex->m_resourceName_,
                                      .m_resource_     = Tex});
          } else {
            auto it_find = Tex->m_uavMipMap.find(tbor->kMipLevel);
            if (it_find != Tex->m_uavMipMap.end()
                && it_find->second.isValid()) {
              m_descriptors_.push_back({.m_descriptor_   = it_find->second,
                                        .m_resourceName_ = Tex->m_resourceName_,
                                        .m_resource_     = Tex});
            } else {
              m_descriptors_.push_back({.m_descriptor_   = Tex->m_uav_,
                                        .m_resourceName_ = Tex->m_resourceName_,
                                        .m_resource_     = Tex});
            }
          }
        }
        break;
      }
      case EShaderBindingType::BUFFER_UAV:
      case EShaderBindingType::BUFFER_UAV_DYNAMIC:
      case EShaderBindingType::BUFFER_TEXEL_UAV: {
        if (IsBindless) {
          auto bufferResourceBindless
              = (BufferResourceBindless*)shaderBinding->m_resource_;
          for (auto Resource : bufferResourceBindless->m_buffers) {
            BufferDx12* Buf = (BufferDx12*)Resource;
            m_descriptors_.push_back({.m_descriptor_   = Buf->m_uav_,
                                      .m_resourceName_ = Buf->m_resourceName_,
                                      .m_resource_     = Buf});
          }
        } else {
          BufferDx12* Buf
              = (BufferDx12*)shaderBinding->m_resource_->getResource();
          assert(Buf->m_buffer->m_resource_);
          if (shaderBinding->m_isInline_) {
            m_rootParameterInlines_.push_back(
                {.m_type_              = InlineRootParamType::UAV,
                 .m_gpuVirtualAddress_ = Buf->getGPUAddress(),
                 .m_resourceName_      = Buf->m_resourceName_,
                 .m_resource_          = Buf});
          } else {
            m_descriptors_.push_back({.m_descriptor_   = Buf->m_uav_,
                                      .m_resourceName_ = Buf->m_resourceName_,
                                      .m_resource_     = Buf});
          }
        }
        break;
      }
      case EShaderBindingType::SAMPLER: {
        if (IsBindless) {
          auto samplerResourceBindless
              = (SamplerResourceBindless*)shaderBinding->m_resource_;
          for (auto Resource : samplerResourceBindless->m_samplerStates_) {
            SamplerStateInfoDx12* Sampler = (SamplerStateInfoDx12*)Resource;
            assert(Sampler);
            m_samplerDescriptors_.push_back(
                {.m_descriptor_   = Sampler->m_samplerSRV_,
                 .m_resourceName_ = Sampler->m_resourceName_,
                 .m_resource_     = Sampler});
          }
        } else {
          SamplerStateInfoDx12* Sampler
              = (SamplerStateInfoDx12*)
                    shaderBinding->m_resource_->getResource();
          assert(Sampler);
          m_samplerDescriptors_.push_back(
              {.m_descriptor_   = Sampler->m_samplerSRV_,
               .m_resourceName_ = Sampler->m_resourceName_,
               .m_resource_     = Sampler});
        }
        break;
      }
      case EShaderBindingType::SUBPASS_INPUT_ATTACHMENT:
      case EShaderBindingType::MAX:
      default:
        assert(0);
        break;
    }
  }

#if _DEBUG
  // validation
  for (int32_t i = 0; i < (int32_t)m_descriptors_.size(); ++i) {
    assert(m_descriptors_[i].isValid());
  }
#endif
}

void* ShaderBindingInstanceDx12::getHandle() const {
  return m_shaderBindingsLayouts_->getHandle();
}

void ShaderBindingInstanceDx12::free() {
  if (getType() == ShaderBindingInstanceType::MultiFrame) {
    ScopedLock s(&g_rhiDx12->m_multiFrameShaderBindingInstanceLock_);
    g_rhiDx12->m_deallocatorMultiFrameShaderBindingInstance_.free(
        shared_from_this());
  }
}

void ShaderBindingInstanceDx12::bindGraphics(
    std::shared_ptr<CommandBufferDx12> commandList,
    int32_t&                           outStartIndex) const {
  assert(commandList);

  auto CommandList = commandList->get();
  assert(CommandList);

  int32_t index = 0;
  for (index = 0; index < m_rootParameterInlines_.size();
       ++index, ++outStartIndex) {
    switch (m_rootParameterInlines_[index].m_type_) {
      case InlineRootParamType::CBV:
        CommandList->SetGraphicsRootConstantBufferView(
            outStartIndex, m_rootParameterInlines_[index].m_gpuVirtualAddress_);
        break;
      case InlineRootParamType::SRV:
        CommandList->SetGraphicsRootShaderResourceView(
            outStartIndex, m_rootParameterInlines_[index].m_gpuVirtualAddress_);
        break;
      case InlineRootParamType::UAV:
        CommandList->SetGraphicsRootUnorderedAccessView(
            outStartIndex, m_rootParameterInlines_[index].m_gpuVirtualAddress_);
      default:
        break;
    }
  }
}

void ShaderBindingInstanceDx12::bindCompute(
    std::shared_ptr<CommandBufferDx12> commandList, int32_t& outStartIndex) {
  assert(commandList);

  auto CommandList = commandList->get();
  assert(CommandList);

  int32_t index = 0;
  for (index = 0; index < m_rootParameterInlines_.size();
       ++index, ++outStartIndex) {
    switch (m_rootParameterInlines_[index].m_type_) {
      case InlineRootParamType::CBV:
        CommandList->SetComputeRootConstantBufferView(
            outStartIndex, m_rootParameterInlines_[index].m_gpuVirtualAddress_);
        break;
      case InlineRootParamType::SRV:
        CommandList->SetComputeRootShaderResourceView(
            outStartIndex, m_rootParameterInlines_[index].m_gpuVirtualAddress_);
        break;
      case InlineRootParamType::UAV:
        CommandList->SetComputeRootUnorderedAccessView(
            outStartIndex, m_rootParameterInlines_[index].m_gpuVirtualAddress_);
      default:
        break;
    }
  }
}

void ShaderBindingInstanceDx12::copyToOnlineDescriptorHeap(
    std::shared_ptr<CommandBufferDx12> commandList) {
  assert(g_rhiDx12);
  assert(g_rhiDx12->m_device_);

  if (m_descriptors_.size() > 0) {
    assert(m_descriptors_.size() <= 1000);
    ResourceContainer<D3D12_CPU_DESCRIPTOR_HANDLE, 1000> DestDescriptor;
    ResourceContainer<D3D12_CPU_DESCRIPTOR_HANDLE, 1000> SrcDescriptor;

    for (int32_t i = 0; i < m_descriptors_.size(); ++i) {
      SrcDescriptor.add(m_descriptors_[i].m_descriptor_.m_cpuHandle_);

      DescriptorDx12 Descriptor = commandList->m_onlineDescriptorHeap_->alloc();
      assert(Descriptor.isValid());
      DestDescriptor.add(Descriptor.m_cpuHandle_);
    }

    g_rhiDx12->m_device_->CopyDescriptors(
        (uint32_t)DestDescriptor.m_numOfData_,
        &DestDescriptor[0],
        nullptr,
        (uint32_t)SrcDescriptor.m_numOfData_,
        &SrcDescriptor[0],
        nullptr,
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  }

  if (m_samplerDescriptors_.size() > 0) {
    assert(m_descriptors_.size() <= 200);
    ResourceContainer<D3D12_CPU_DESCRIPTOR_HANDLE, 1000> DestSamplerDescriptor;
    ResourceContainer<D3D12_CPU_DESCRIPTOR_HANDLE, 1000> SrcSamplerDescriptor;

    for (int32_t i = 0; i < m_samplerDescriptors_.size(); ++i) {
      SrcSamplerDescriptor.add(
          m_samplerDescriptors_[i].m_descriptor_.m_cpuHandle_);

      DescriptorDx12 Descriptor
          = commandList->m_onlineSamplerDescriptorHeap_->alloc();
      assert(Descriptor.isValid());
      DestSamplerDescriptor.add(Descriptor.m_cpuHandle_);
    }

    g_rhiDx12->m_device_->CopyDescriptors(
        (uint32_t)DestSamplerDescriptor.m_numOfData_,
        &DestSamplerDescriptor[0],
        nullptr,
        (uint32_t)SrcSamplerDescriptor.m_numOfData_,
        &SrcSamplerDescriptor[0],
        nullptr,
        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
  }
}

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12
