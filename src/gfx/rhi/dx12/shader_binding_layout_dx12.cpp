#include "gfx/rhi/dx12/shader_binding_layout_dx12.h"

#include "gfx/rhi/dx12/rhi_dx12.h"
#include "gfx/rhi/dx12/shader_binding_instance_dx12.h"

namespace game_engine {

std::unordered_map<size_t, ComPtr<ID3D12RootSignature>>
            ShaderBindingLayoutDx12::GRootSignaturePool;
MutexRWLock ShaderBindingLayoutDx12::GRootSignatureLock;

// Below option will be work like switch
#define FORCE_USE_DESCRIPTOR_OFFSET_BY_USING_AUTO_CALCULATION_FOR_BINDLESS 1
#define FORCE_USE_D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND \
  (1 && !FORCE_USE_DESCRIPTOR_OFFSET_BY_USING_AUTO_CALCULATION_FOR_BINDLESS)

void RootParameterExtractor::Extract(
    int32_t&                   InOutDescriptorOffset,
    int32_t&                   InOutSamplerDescriptorOffset,
    const ShaderBindingArray& InShaderBindingArray,
    int32_t                    InRegisterSpace) {
  // Always place inline descriptors at the very beginning of InRootParameters.
  int32_t BindingIndex = 0;  // To support both APIs (Vulkan, DX12): Vulkan
                             // requires a unique binding index.

  for (int32_t i = 0; i < InShaderBindingArray.NumOfData; ++i) {
    const ShaderBinding* shaderBinding = InShaderBindingArray[i];
    const bool            IsBindless    = shaderBinding->IsBindless;

    assert(
        !shaderBinding->IsInline
        || (shaderBinding->IsInline
            && (shaderBinding->BindingType == EShaderBindingType::UNIFORMBUFFER
                || shaderBinding->BindingType
                       == EShaderBindingType::UNIFORMBUFFER_DYNAMIC
                || shaderBinding->BindingType == EShaderBindingType::BUFFER_SRV
                || shaderBinding->BindingType
                       == EShaderBindingType::ACCELERATION_STRUCTURE_SRV
                || shaderBinding->BindingType
                       == EShaderBindingType::BUFFER_UAV)));

    switch (shaderBinding->BindingType) {
      case EShaderBindingType::UNIFORMBUFFER:
      case EShaderBindingType::UNIFORMBUFFER_DYNAMIC: {
        if (shaderBinding->IsInline) {
          D3D12_ROOT_PARAMETER1 rootParameter = {};
          rootParameter.ParameterType         = D3D12_ROOT_PARAMETER_TYPE_CBV;
          rootParameter.ShaderVisibility      = D3D12_SHADER_VISIBILITY_ALL;
          rootParameter.Descriptor.Flags
              = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC;
          rootParameter.Descriptor.ShaderRegister = BindingIndex;
          rootParameter.Descriptor.RegisterSpace  = InRegisterSpace;
          RootParameters.emplace_back(rootParameter);
        } else {
          D3D12_DESCRIPTOR_RANGE1 range = {};
          range.RangeType               = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
          range.NumDescriptors          = shaderBinding->NumOfDescriptors;
          range.BaseShaderRegister      = BindingIndex;
          range.RegisterSpace           = InRegisterSpace;
          if (IsBindless) {
            range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
            range.OffsetInDescriptorsFromTableStart = InOutDescriptorOffset;
          } else {
#if FORCE_USE_DESCRIPTOR_OFFSET_BY_USING_AUTO_CALCULATION_FOR_BINDLESS
            range.OffsetInDescriptorsFromTableStart = InOutDescriptorOffset;
#elif FORCE_USE_D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
            range.OffsetInDescriptorsFromTableStart
                = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
#else
            range.OffsetInDescriptorsFromTableStart
                = (shaderBinding->BindingPoint == -1)
                    ? D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
                    : shaderBinding->BindingPoint;
#endif
            range.Flags
                = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE;
          }
          InOutDescriptorOffset += shaderBinding->NumOfDescriptors;
          Descriptors.emplace_back(range);
        }

        BindingIndex += shaderBinding->NumOfDescriptors;
        break;
      }
      case EShaderBindingType::TEXTURE_SAMPLER_SRV:
      case EShaderBindingType::TEXTURE_SRV:
      case EShaderBindingType::TEXTURE_ARRAY_SRV:
      case EShaderBindingType::BUFFER_SRV:
      case EShaderBindingType::BUFFER_TEXEL_SRV:
      case EShaderBindingType::ACCELERATION_STRUCTURE_SRV: {
        if (shaderBinding->IsInline
            && (shaderBinding->BindingType == EShaderBindingType::BUFFER_SRV
                || shaderBinding->BindingType
                       == EShaderBindingType::ACCELERATION_STRUCTURE_SRV)) {
          D3D12_ROOT_PARAMETER1 rootParameter = {};
          rootParameter.ParameterType         = D3D12_ROOT_PARAMETER_TYPE_SRV;
          rootParameter.ShaderVisibility      = D3D12_SHADER_VISIBILITY_ALL;
          rootParameter.Descriptor.Flags
              = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE;
          rootParameter.Descriptor.ShaderRegister = BindingIndex;
          rootParameter.Descriptor.RegisterSpace  = InRegisterSpace;
          RootParameters.emplace_back(rootParameter);
        } else {
          D3D12_DESCRIPTOR_RANGE1 range = {};
          range.RangeType               = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
          range.NumDescriptors          = shaderBinding->NumOfDescriptors;
          range.BaseShaderRegister      = BindingIndex;
          range.RegisterSpace           = InRegisterSpace;
          if (IsBindless) {
            range.OffsetInDescriptorsFromTableStart = InOutDescriptorOffset;
            range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
          } else {
#if FORCE_USE_DESCRIPTOR_OFFSET_BY_USING_AUTO_CALCULATION_FOR_BINDLESS
            range.OffsetInDescriptorsFromTableStart = InOutDescriptorOffset;
#elif FORCE_USE_D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
            range.OffsetInDescriptorsFromTableStart
                = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
#else
            range.OffsetInDescriptorsFromTableStart
                = (shaderBinding->BindingPoint == -1)
                    ? D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
                    : shaderBinding->BindingPoint;
#endif
            range.Flags
                = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE;
          }
          InOutDescriptorOffset += shaderBinding->NumOfDescriptors;
          Descriptors.emplace_back(range);
        }

        // TODO: Since Texture Sampler SRV defines both Texture and Sampler
        // simultaneously, add Sampler here. Need to consider not using it since
        // it's a Vulkan syntax.
        if (shaderBinding->BindingType
            == EShaderBindingType::TEXTURE_SAMPLER_SRV) {
          D3D12_DESCRIPTOR_RANGE1 range = {};
          range.RangeType               = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
          range.NumDescriptors          = shaderBinding->NumOfDescriptors;
          range.BaseShaderRegister      = BindingIndex;
          range.RegisterSpace           = InRegisterSpace;
          if (IsBindless) {
            range.OffsetInDescriptorsFromTableStart
                = InOutSamplerDescriptorOffset;
            range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
          } else {
#if FORCE_USE_DESCRIPTOR_OFFSET_BY_USING_AUTO_CALCULATION_FOR_BINDLESS
            range.OffsetInDescriptorsFromTableStart
                = InOutSamplerDescriptorOffset;
#elif FORCE_USE_D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
            range.OffsetInDescriptorsFromTableStart
                = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
#else
            range.OffsetInDescriptorsFromTableStart
                = (shaderBinding->BindingPoint == -1)
                    ? D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
                    : shaderBinding->BindingPoint;
#endif
            range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
          }
          InOutSamplerDescriptorOffset += shaderBinding->NumOfDescriptors;
          SamplerDescriptors.emplace_back(range);

          // SamplerIndex += shaderBinding->NumOfDescriptors;
        }

        BindingIndex += shaderBinding->NumOfDescriptors;
        break;
      }
      case EShaderBindingType::TEXTURE_UAV:
      case EShaderBindingType::BUFFER_UAV:
      case EShaderBindingType::BUFFER_UAV_DYNAMIC:
      case EShaderBindingType::BUFFER_TEXEL_UAV: {
        if (shaderBinding->IsInline) {
          D3D12_ROOT_PARAMETER1 rootParameter = {};
          rootParameter.ParameterType         = D3D12_ROOT_PARAMETER_TYPE_UAV;
          rootParameter.ShaderVisibility      = D3D12_SHADER_VISIBILITY_ALL;
          rootParameter.Descriptor.Flags
              = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE;
          rootParameter.Descriptor.ShaderRegister = BindingIndex;
          rootParameter.Descriptor.RegisterSpace  = InRegisterSpace;
          RootParameters.emplace_back(rootParameter);
        } else {
          D3D12_DESCRIPTOR_RANGE1 range = {};
          range.RangeType               = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
          range.NumDescriptors          = shaderBinding->NumOfDescriptors;
          range.BaseShaderRegister      = BindingIndex;
          range.RegisterSpace           = InRegisterSpace;
          if (IsBindless) {
            range.OffsetInDescriptorsFromTableStart = InOutDescriptorOffset;
            range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
          } else {
#if FORCE_USE_DESCRIPTOR_OFFSET_BY_USING_AUTO_CALCULATION_FOR_BINDLESS
            range.OffsetInDescriptorsFromTableStart = InOutDescriptorOffset;
#elif FORCE_USE_D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
            range.OffsetInDescriptorsFromTableStart
                = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
#else
            range.OffsetInDescriptorsFromTableStart
                = (shaderBinding->BindingPoint == -1)
                    ? D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
                    : shaderBinding->BindingPoint;
#endif
            range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE;
          }
          InOutDescriptorOffset += shaderBinding->NumOfDescriptors;
          Descriptors.emplace_back(range);
        }

        BindingIndex += shaderBinding->NumOfDescriptors;
        break;
      }
      case EShaderBindingType::SAMPLER: {
        D3D12_DESCRIPTOR_RANGE1 range = {};
        range.RangeType               = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
        range.NumDescriptors          = shaderBinding->NumOfDescriptors;
        range.BaseShaderRegister      = BindingIndex;
        range.RegisterSpace           = InRegisterSpace;
        if (IsBindless) {
          range.OffsetInDescriptorsFromTableStart
              = InOutSamplerDescriptorOffset;
          range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
        } else {
#if FORCE_USE_DESCRIPTOR_OFFSET_BY_USING_AUTO_CALCULATION_FOR_BINDLESS
          range.OffsetInDescriptorsFromTableStart
              = InOutSamplerDescriptorOffset;
#elif FORCE_USE_D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
          range.OffsetInDescriptorsFromTableStart
              = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
#else
          range.OffsetInDescriptorsFromTableStart
              = (shaderBinding->BindingPoint == -1)
                  ? D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
                  : shaderBinding->BindingPoint;
#endif
          range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
        }
        InOutSamplerDescriptorOffset += shaderBinding->NumOfDescriptors;
        SamplerDescriptors.emplace_back(range);

        BindingIndex += shaderBinding->NumOfDescriptors;
        break;
      }
      case EShaderBindingType::SUBPASS_INPUT_ATTACHMENT:
      case EShaderBindingType::MAX:
      default:
        assert(0);
        break;
    }
  }
}

void RootParameterExtractor::Extract(
    const ShaderBindingLayoutArray& InBindingLayoutArray,
    int32_t                          InRegisterSpace /*= 0*/) {
  int32_t InOutDescriptorOffset        = 0;
  int32_t InOutSamplerDescriptorOffset = 0;
  for (int32_t i = 0; i < InBindingLayoutArray.NumOfData; ++i) {
    ShaderBindingLayoutDx12* Layout
        = (ShaderBindingLayoutDx12*)InBindingLayoutArray[i];
    assert(Layout);

    Extract(InOutDescriptorOffset,
            InOutSamplerDescriptorOffset,
            Layout->GetShaderBindingsLayout(),
            i);
  }

  NumOfInlineRootParameter = (int32_t)RootParameters.size();

  if (Descriptors.size() > 0) {
    D3D12_ROOT_PARAMETER1 rootParameter = {};
    rootParameter.ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameter.DescriptorTable.NumDescriptorRanges
        = (uint32_t)Descriptors.size();
    rootParameter.DescriptorTable.pDescriptorRanges = &Descriptors[0];
    RootParameters.emplace_back(rootParameter);
  }

  if (SamplerDescriptors.size() > 0) {
    D3D12_ROOT_PARAMETER1 rootParameter = {};
    rootParameter.ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameter.DescriptorTable.NumDescriptorRanges
        = (uint32_t)SamplerDescriptors.size();
    rootParameter.DescriptorTable.pDescriptorRanges = &SamplerDescriptors[0];
    RootParameters.emplace_back(rootParameter);
  }
}

void RootParameterExtractor::Extract(
    const ShaderBindingInstanceArray& InBindingInstanceArray,
    int32_t                            InRegisterSpace /*= 0*/) {
  int32_t InOutDescriptorOffset        = 0;
  int32_t InOutSamplerDescriptorOffset = 0;
  for (int32_t i = 0; i < InBindingInstanceArray.NumOfData; ++i) {
    ShaderBindingInstanceDx12* Instance
        = (ShaderBindingInstanceDx12*)InBindingInstanceArray[i];
    assert(Instance);
    assert(Instance->ShaderBindingsLayouts);

    ShaderBindingLayoutDx12* Layout
        = (ShaderBindingLayoutDx12*)Instance->ShaderBindingsLayouts;
    assert(Layout);
    Extract(InOutDescriptorOffset,
            InOutSamplerDescriptorOffset,
            Layout->GetShaderBindingsLayout(),
            i);
  }

  NumOfInlineRootParameter = (int32_t)RootParameters.size();

  if (Descriptors.size() > 0) {
    D3D12_ROOT_PARAMETER1 rootParameter = {};
    rootParameter.ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameter.DescriptorTable.NumDescriptorRanges
        = (uint32_t)Descriptors.size();
    rootParameter.DescriptorTable.pDescriptorRanges = &Descriptors[0];
    RootParameters.emplace_back(rootParameter);
  }

  if (SamplerDescriptors.size() > 0) {
    D3D12_ROOT_PARAMETER1 rootParameter = {};
    rootParameter.ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameter.DescriptorTable.NumDescriptorRanges
        = (uint32_t)SamplerDescriptors.size();
    rootParameter.DescriptorTable.pDescriptorRanges = &SamplerDescriptors[0];
    RootParameters.emplace_back(rootParameter);
  }
}

bool ShaderBindingLayoutDx12::Initialize(
    const ShaderBindingArray& InShaderBindingArray) {
  InShaderBindingArray.CloneWithoutResource(m_shaderBindingArray_);

  return true;
}

std::shared_ptr<ShaderBindingInstance>
    ShaderBindingLayoutDx12::CreateShaderBindingInstance(
        const ShaderBindingArray&       InShaderBindingArray,
        const ShaderBindingInstanceType InType) const {
  auto shaderBindingInstance = new ShaderBindingInstanceDx12();
  shaderBindingInstance->ShaderBindingsLayouts = this;
  shaderBindingInstance->Initialize(InShaderBindingArray);
  shaderBindingInstance->SetType(InType);

  return std::shared_ptr<ShaderBindingInstance>(shaderBindingInstance);
}

ID3D12RootSignature* ShaderBindingLayoutDx12::CreateRootSignatureInternal(
    size_t InHash, FuncGetRootParameterExtractor InFunc) {
  {
    ScopeReadLock sr(&GRootSignatureLock);
    auto          it_find = GRootSignaturePool.find(InHash);
    if (GRootSignaturePool.end() != it_find) {
      return it_find->second.Get();
    }
  }

  {
    ScopeWriteLock sw(&GRootSignatureLock);

    // Try again, to avoid entering creation section simultaneously.
    auto it_find = GRootSignaturePool.find(InHash);
    if (GRootSignaturePool.end() != it_find) {
      return it_find->second.Get();
    }

    RootParameterExtractor DescriptorExtractor;
    InFunc(DescriptorExtractor);

    // Create RootSignature
    D3D12_ROOT_SIGNATURE_DESC1 rootSignatureDesc = {};
    rootSignatureDesc.NumParameters
        = static_cast<uint32_t>(DescriptorExtractor.RootParameters.size());
    rootSignatureDesc.pParameters = DescriptorExtractor.RootParameters.data();
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers   = nullptr;
    rootSignatureDesc.Flags
        = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
        | D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
        | D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;  // Support
                                                                    // for
                                                                    // BindlessResource

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedDesc = {};
    versionedDesc.Version  = D3D_ROOT_SIGNATURE_VERSION_1_1;
    versionedDesc.Desc_1_1 = rootSignatureDesc;

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    HRESULT          hr_serialize = D3D12SerializeVersionedRootSignature(
        &versionedDesc, &signature, &error);
    assert(SUCCEEDED(hr_serialize));

    if (FAILED(hr_serialize)) {
      if (error) {
        OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
      }
      return nullptr;
    }

    ComPtr<ID3D12RootSignature> RootSignature;
    HRESULT                     hr_create
        = g_rhi_dx12->Device->CreateRootSignature(0,
                                                  signature->GetBufferPointer(),
                                                  signature->GetBufferSize(),
                                                  IID_PPV_ARGS(&RootSignature));
    assert(SUCCEEDED(hr_create));

    if (FAILED(hr_create)) {
      return nullptr;
    }

    GRootSignaturePool[InHash] = RootSignature;
    return RootSignature.Get();
  }
}

ID3D12RootSignature* ShaderBindingLayoutDx12::CreateRootSignature(
    const ShaderBindingInstanceArray& InBindingInstanceArray) {
  if (InBindingInstanceArray.NumOfData <= 0) {
    return nullptr;
  }

  size_t hash = 0;
  for (int32_t i = 0; i < InBindingInstanceArray.NumOfData; ++i) {
    ShaderBindingLayoutArray::GetHash(
        hash, i, InBindingInstanceArray[i]->ShaderBindingsLayouts);
  }

  return CreateRootSignatureInternal(
      hash,
      [&InBindingInstanceArray](
          RootParameterExtractor& OutRootParameterExtractor) {
        OutRootParameterExtractor.Extract(InBindingInstanceArray);
      });
}

ID3D12RootSignature* ShaderBindingLayoutDx12::CreateRootSignature(
    const ShaderBindingLayoutArray& InBindingLayoutArray) {
  if (InBindingLayoutArray.NumOfData <= 0) {
    return nullptr;
  }

  const size_t hash = InBindingLayoutArray.GetHash();

  return CreateRootSignatureInternal(
      hash,
      [&InBindingLayoutArray](
          RootParameterExtractor& OutRootParameterExtractor) {
        OutRootParameterExtractor.Extract(InBindingLayoutArray);
      });
}

}  // namespace game_engine