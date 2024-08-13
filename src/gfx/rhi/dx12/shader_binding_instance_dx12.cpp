#include "gfx/rhi/dx12/shader_binding_instance_dx12.h"
#include "gfx/rhi/dx12/rhi_dx12.h"
#include "gfx/rhi/dx12/uniform_buffer_block_dx12.h"
#include "gfx/rhi/dx12/rhi_type_dx12.h"
#include "gfx/rhi/dx12/command_list_dx12.h"


namespace game_engine {

void ShaderBindingInstanceDx12::Initialize(
    const ShaderBindingArray& InShaderBindingArray) {
  UpdateShaderBindings(InShaderBindingArray);
}

void ShaderBindingInstanceDx12::UpdateShaderBindings(
    const ShaderBindingArray& InShaderBindingArray) {
  // Let's copy the descriptors to the online descriptor set.
  // CopySimpleDescriptor
  // Let's find the descriptor layout and bind it.
  // Should we copy from the layout and actually create the root signature?
  // Let's think about this part.

  assert(ShaderBindingsLayouts);
  assert(ShaderBindingsLayouts->GetShaderBindingsLayout().NumOfData
         == InShaderBindingArray.NumOfData);

  Descriptors.clear();
  SamplerDescriptors.clear();
  RootParameterInlines.clear();

  for (int32_t i = 0; i < InShaderBindingArray.NumOfData; ++i) {
    const ShaderBinding* shaderBinding = InShaderBindingArray[i];
    assert(shaderBinding);
    assert(shaderBinding->Resource);
    assert(shaderBinding->IsBindless == shaderBinding->Resource->IsBindless());

    const bool IsBindless = shaderBinding->IsBindless;
    assert((IsBindless && !shaderBinding->IsInline)
           || !IsBindless);  // Bindless must note be inline

    switch (shaderBinding->BindingType) {
      case EShaderBindingType::UNIFORMBUFFER:
      case EShaderBindingType::UNIFORMBUFFER_DYNAMIC: {
        if (IsBindless) {
          auto UniformResourceBindless
              = (UniformBufferResourceBindless*)shaderBinding->Resource;
          for (auto Resource : UniformResourceBindless->UniformBuffers) {
            assert(Resource);

            UniformBufferBlockDx12* UniformBuffer
                = (UniformBufferBlockDx12*)Resource;
            Descriptors.push_back({.Descriptor   = UniformBuffer->GetCBV(),
                                   .ResourceName = UniformBuffer->ResourceName,
                                   .Resource     = UniformBuffer});
          }
        } else {
          UniformBufferBlockDx12* UniformBuffer
              = (UniformBufferBlockDx12*)
                    shaderBinding->Resource->GetResource();
          assert(UniformBuffer->GetLowLevelResource());
          // assert(!UniformBuffer->IsUseRingBuffer() ||
          // (UniformBuffer->IsUseRingBuffer() && shaderBinding->IsInline));
          if (shaderBinding->IsInline) {
            RootParameterInlines.push_back(
                {.Type              = InlineRootParamType::CBV,
                 .GPUVirtualAddress = UniformBuffer->GetGPUAddress(),
                 .ResourceName      = UniformBuffer->ResourceName,
                 .Resource          = UniformBuffer});
          } else {
            Descriptors.push_back({.Descriptor   = UniformBuffer->GetCBV(),
                                   .ResourceName = UniformBuffer->ResourceName,
                                   .Resource     = UniformBuffer});
          }
        }
        break;
      }
      case EShaderBindingType::TEXTURE_SAMPLER_SRV: {
        if (IsBindless) {
          auto TextureResourceResourceBindless
              = (TextureResourceBindless*)shaderBinding->Resource;
          for (auto Resource :
               TextureResourceResourceBindless->TextureBindDatas) {
            assert(Resource.m_texture);

            TextureDx12* TexDX12 = (TextureDx12*)Resource.m_texture;
            Descriptors.push_back({.Descriptor   = TexDX12->SRV,
                                   .ResourceName = TexDX12->ResourceName,
                                   .Resource     = TexDX12});

            if (Resource.SamplerState) {
              SamplerStateInfoDx12* SamplerDX12
                  = (SamplerStateInfoDx12*)Resource.SamplerState;
              assert(SamplerDX12);
              SamplerDescriptors.push_back(
                  {.Descriptor   = SamplerDX12->SamplerSRV,
                   .ResourceName = SamplerDX12->ResourceName,
                   .Resource     = SamplerDX12});
            } else {
              // assert(0);   // todo : need to set DefaultSamplerState
              const SamplerStateInfoDx12* SamplerDX12
                  = (SamplerStateInfoDx12*)
                      TSamplerStateInfo<ETextureFilter::LINEAR_MIPMAP_LINEAR,
                                        ETextureFilter::LINEAR_MIPMAP_LINEAR,
                                        ETextureAddressMode::REPEAT,
                                        ETextureAddressMode::REPEAT,
                                        ETextureAddressMode::REPEAT,
                                        0.0f,
                                        16.0f>::Create();
              assert(SamplerDX12);
              SamplerDescriptors.push_back(
                  {.Descriptor   = SamplerDX12->SamplerSRV,
                   .ResourceName = SamplerDX12->ResourceName,
                   .Resource     = SamplerDX12});
            }
          }
        } else {
          const TextureResource* tbor
              = reinterpret_cast<const TextureResource*>(
                  shaderBinding->Resource);
          assert(tbor && tbor->m_texture);

          if (tbor && tbor->m_texture) {
            TextureDx12* TexDX12 = (TextureDx12*)tbor->m_texture;
            Descriptors.push_back({.Descriptor   = TexDX12->SRV,
                                   .ResourceName = TexDX12->ResourceName,
                                   .Resource     = TexDX12});

            if (tbor->SamplerState) {
              SamplerStateInfoDx12* SamplerDX12
                  = (SamplerStateInfoDx12*)tbor->SamplerState;
              assert(SamplerDX12);
              SamplerDescriptors.push_back(
                  {.Descriptor   = SamplerDX12->SamplerSRV,
                   .ResourceName = SamplerDX12->ResourceName,
                   .Resource     = SamplerDX12});
            } else {
              // assert(0);   // TODO: need to set DefaultSamplerState

              const SamplerStateInfoDx12* SamplerDX12
                  = (SamplerStateInfoDx12*)
                      TSamplerStateInfo<ETextureFilter::LINEAR_MIPMAP_LINEAR,
                                        ETextureFilter::LINEAR_MIPMAP_LINEAR,
                                        ETextureAddressMode::REPEAT,
                                        ETextureAddressMode::REPEAT,
                                        ETextureAddressMode::REPEAT,
                                        0.0f,
                                        16.0f>::Create();
              assert(SamplerDX12);
              SamplerDescriptors.push_back(
                  {.Descriptor   = SamplerDX12->SamplerSRV,
                   .ResourceName = SamplerDX12->ResourceName,
                   .Resource     = SamplerDX12});
            }
          }
        }
        break;
      }
      case EShaderBindingType::TEXTURE_SRV: {
        if (IsBindless) {
          auto TextureResourceResourceBindless
              = (TextureResourceBindless*)shaderBinding->Resource;
          for (auto Resource :
               TextureResourceResourceBindless->TextureBindDatas) {
            TextureDx12* Tex = (TextureDx12*)Resource.m_texture;
            Descriptors.push_back({.Descriptor   = Tex->SRV,
                                   .ResourceName = Tex->ResourceName,
                                   .Resource     = Tex});
          }
        } else {
          TextureDx12* Tex
              = (TextureDx12*)shaderBinding->Resource->GetResource();
          Descriptors.push_back({.Descriptor   = Tex->SRV,
                                 .ResourceName = Tex->ResourceName,
                                 .Resource     = Tex});
        }
        break;
      }
      case EShaderBindingType::TEXTURE_ARRAY_SRV: {
        if (IsBindless) {
          auto TextureResourceResourceArrayBindless
              = (TextureArrayResourceBindless*)shaderBinding->Resource;
          for (auto Resource :
               TextureResourceResourceArrayBindless->TextureArrayBindDatas) {
            TextureDx12** TexArray = (TextureDx12**)Resource.TextureArray;
            for (int32_t i = 0; i < Resource.InNumOfTexure; ++i) {
              assert(TexArray[i]);
              Descriptors.push_back({.Descriptor   = TexArray[i]->SRV,
                                     .ResourceName = TexArray[i]->ResourceName,
                                     .Resource     = TexArray[i]});
            }
          }
        } else {
          TextureDx12** Tex
              = (TextureDx12**)shaderBinding->Resource->GetResource();
          for (int32_t i = 0; i < shaderBinding->Resource->NumOfResource();
               ++i) {
            assert(Tex[i]);
            Descriptors.push_back({.Descriptor   = Tex[i]->SRV,
                                   .ResourceName = Tex[i]->ResourceName,
                                   .Resource     = Tex[i]});
          }
        }
        break;
      }
      case EShaderBindingType::BUFFER_SRV:
      case EShaderBindingType::BUFFER_TEXEL_SRV:
      case EShaderBindingType::ACCELERATION_STRUCTURE_SRV: {
        if (IsBindless) {
          auto bufferResourceBindless
              = (BufferResourceBindless*)shaderBinding->Resource;
          for (auto Resource : bufferResourceBindless->m_buffers) {
            BufferDx12* Buf = (BufferDx12*)Resource;
            Descriptors.push_back({.Descriptor   = Buf->SRV,
                                   .ResourceName = Buf->ResourceName,
                                   .Resource     = Buf});
          }
        } else {
          BufferDx12* Buf
              = (BufferDx12*)shaderBinding->Resource->GetResource();
          assert(Buf->m_buffer->Resource);

          if (shaderBinding->IsInline) {
            RootParameterInlines.push_back(
                {.Type              = InlineRootParamType::SRV,
                 .GPUVirtualAddress = Buf->GetGPUAddress(),
                 .ResourceName      = Buf->ResourceName,
                 .Resource          = Buf});
          } else {
            Descriptors.push_back({.Descriptor   = Buf->SRV,
                                   .ResourceName = Buf->ResourceName,
                                   .Resource     = Buf});
          }
        }
        break;
      }
      case EShaderBindingType::TEXTURE_UAV: {
        if (IsBindless) {
          auto TextureResourceResourceBindless
              = (TextureResourceBindless*)shaderBinding->Resource;
          for (auto Resource :
               TextureResourceResourceBindless->TextureBindDatas) {
            TextureDx12* Tex = (TextureDx12*)Resource.m_texture;
            if (Resource.MipLevel == 0) {
              Descriptors.push_back({.Descriptor   = Tex->UAV,
                                     .ResourceName = Tex->ResourceName,
                                     .Resource     = Tex});
            } else {
              auto it_find = Tex->UAVMipMap.find(Resource.MipLevel);
              if (it_find != Tex->UAVMipMap.end()
                  && it_find->second.IsValid()) {
                Descriptors.push_back({.Descriptor   = it_find->second,
                                       .ResourceName = Tex->ResourceName,
                                       .Resource     = Tex});
              } else {
                Descriptors.push_back({.Descriptor   = Tex->UAV,
                                       .ResourceName = Tex->ResourceName,
                                       .Resource     = Tex});
              }
            }
          }
        } else {
          TextureDx12* Tex
              = (TextureDx12*)shaderBinding->Resource->GetResource();
          const TextureResource* tbor
              = reinterpret_cast<const TextureResource*>(
                  shaderBinding->Resource);
          if (tbor->MipLevel == 0) {
            Descriptors.push_back({.Descriptor   = Tex->UAV,
                                   .ResourceName = Tex->ResourceName,
                                   .Resource     = Tex});
          } else {
            auto it_find = Tex->UAVMipMap.find(tbor->MipLevel);
            if (it_find != Tex->UAVMipMap.end() && it_find->second.IsValid()) {
              Descriptors.push_back({.Descriptor   = it_find->second,
                                     .ResourceName = Tex->ResourceName,
                                     .Resource     = Tex});
            } else {
              Descriptors.push_back({.Descriptor   = Tex->UAV,
                                     .ResourceName = Tex->ResourceName,
                                     .Resource     = Tex});
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
              = (BufferResourceBindless*)shaderBinding->Resource;
          for (auto Resource : bufferResourceBindless->m_buffers) {
            BufferDx12* Buf = (BufferDx12*)Resource;
            Descriptors.push_back({.Descriptor   = Buf->UAV,
                                   .ResourceName = Buf->ResourceName,
                                   .Resource     = Buf});
          }
        } else {
          BufferDx12* Buf
              = (BufferDx12*)shaderBinding->Resource->GetResource();
          assert(Buf->m_buffer->Resource);
          if (shaderBinding->IsInline) {
            RootParameterInlines.push_back(
                {.Type              = InlineRootParamType::UAV,
                 .GPUVirtualAddress = Buf->GetGPUAddress(),
                 .ResourceName      = Buf->ResourceName,
                 .Resource          = Buf});
          } else {
            Descriptors.push_back({.Descriptor   = Buf->UAV,
                                   .ResourceName = Buf->ResourceName,
                                   .Resource     = Buf});
          }
        }
        break;
      }
      case EShaderBindingType::SAMPLER: {
        if (IsBindless) {
          auto samplerResourceBindless
              = (SamplerResourceBindless*)shaderBinding->Resource;
          for (auto Resource : samplerResourceBindless->SamplerStates) {
            SamplerStateInfoDx12* Sampler = (SamplerStateInfoDx12*)Resource;
            assert(Sampler);
            SamplerDescriptors.push_back({.Descriptor   = Sampler->SamplerSRV,
                                          .ResourceName = Sampler->ResourceName,
                                          .Resource     = Sampler});
          }
        } else {
          SamplerStateInfoDx12* Sampler
              = (SamplerStateInfoDx12*)shaderBinding->Resource->GetResource();
          assert(Sampler);
          SamplerDescriptors.push_back({.Descriptor   = Sampler->SamplerSRV,
                                        .ResourceName = Sampler->ResourceName,
                                        .Resource     = Sampler});
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
  for (int32_t i = 0; i < (int32_t)Descriptors.size(); ++i) {
    assert(Descriptors[i].IsValid());
  }
#endif
}

void* ShaderBindingInstanceDx12::GetHandle() const {
  return ShaderBindingsLayouts->GetHandle();
}

void ShaderBindingInstanceDx12::Free() {
  if (GetType() == ShaderBindingInstanceType::MultiFrame) {
    ScopedLock s(&g_rhi_dx12->MultiFrameShaderBindingInstanceLock);
    g_rhi_dx12->m_deallocatorMultiFrameShaderBindingInstance.Free(
        shared_from_this());
  }
}

void ShaderBindingInstanceDx12::BindGraphics(
    CommandBufferDx12* InCommandList, int32_t& InOutStartIndex) const {
  assert(InCommandList);

  auto CommandList = InCommandList->Get();
  assert(CommandList);

  int32_t index = 0;
  for (index = 0; index < RootParameterInlines.size();
       ++index, ++InOutStartIndex) {
    switch (RootParameterInlines[index].Type) {
      case InlineRootParamType::CBV:
        CommandList->SetGraphicsRootConstantBufferView(
            InOutStartIndex, RootParameterInlines[index].GPUVirtualAddress);
        break;
      case InlineRootParamType::SRV:
        CommandList->SetGraphicsRootShaderResourceView(
            InOutStartIndex, RootParameterInlines[index].GPUVirtualAddress);
        break;
      case InlineRootParamType::UAV:
        CommandList->SetGraphicsRootUnorderedAccessView(
            InOutStartIndex, RootParameterInlines[index].GPUVirtualAddress);
      default:
        break;
    }
  }
}

void ShaderBindingInstanceDx12::BindCompute(
    CommandBufferDx12* InCommandList, int32_t& InOutStartIndex) {
  assert(InCommandList);

  auto CommandList = InCommandList->Get();
  assert(CommandList);

  int32_t index = 0;
  for (index = 0; index < RootParameterInlines.size();
       ++index, ++InOutStartIndex) {
    switch (RootParameterInlines[index].Type) {
      case InlineRootParamType::CBV:
        CommandList->SetComputeRootConstantBufferView(
            InOutStartIndex, RootParameterInlines[index].GPUVirtualAddress);
        break;
      case InlineRootParamType::SRV:
        CommandList->SetComputeRootShaderResourceView(
            InOutStartIndex, RootParameterInlines[index].GPUVirtualAddress);
        break;
      case InlineRootParamType::UAV:
        CommandList->SetComputeRootUnorderedAccessView(
            InOutStartIndex, RootParameterInlines[index].GPUVirtualAddress);
      default:
        break;
    }
  }
}

void ShaderBindingInstanceDx12::CopyToOnlineDescriptorHeap(
    CommandBufferDx12* InCommandList) {
  assert(g_rhi_dx12);
  assert(g_rhi_dx12->Device);

  if (Descriptors.size() > 0) {
    assert(Descriptors.size() <= 1000);
    ResourceContainer<D3D12_CPU_DESCRIPTOR_HANDLE, 1000> DestDescriptor; 
    ResourceContainer<D3D12_CPU_DESCRIPTOR_HANDLE, 1000> SrcDescriptor;

    for (int32_t i = 0; i < Descriptors.size(); ++i) {
      SrcDescriptor.Add(Descriptors[i].Descriptor.CPUHandle);

      DescriptorDx12 Descriptor
          = InCommandList->OnlineDescriptorHeap->Alloc();
      assert(Descriptor.IsValid());
      DestDescriptor.Add(Descriptor.CPUHandle);
    }

    g_rhi_dx12->Device->CopyDescriptors((uint32_t)DestDescriptor.NumOfData,
                                        &DestDescriptor[0],
                                        nullptr,
                                        (uint32_t)SrcDescriptor.NumOfData,
                                        &SrcDescriptor[0],
                                        nullptr,
                                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  }

  if (SamplerDescriptors.size() > 0) {
    assert(Descriptors.size() <= 200);
    ResourceContainer<D3D12_CPU_DESCRIPTOR_HANDLE, 1000> DestSamplerDescriptor;
    ResourceContainer<D3D12_CPU_DESCRIPTOR_HANDLE, 1000> SrcSamplerDescriptor;

    for (int32_t i = 0; i < SamplerDescriptors.size(); ++i) {
      SrcSamplerDescriptor.Add(SamplerDescriptors[i].Descriptor.CPUHandle);

      DescriptorDx12 Descriptor
          = InCommandList->OnlineSamplerDescriptorHeap->Alloc();
      assert(Descriptor.IsValid());
      DestSamplerDescriptor.Add(Descriptor.CPUHandle);
    }

    g_rhi_dx12->Device->CopyDescriptors(
        (uint32_t)DestSamplerDescriptor.NumOfData,
        &DestSamplerDescriptor[0],
        nullptr,
        (uint32_t)SrcSamplerDescriptor.NumOfData,
        &SrcSamplerDescriptor[0],
        nullptr,
        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
  }
}

}  // namespace game_engine