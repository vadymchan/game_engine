#ifndef GAME_ENGINE_SHADER_BINDING_INSTANCE_DX12_H
#define GAME_ENGINE_SHADER_BINDING_INSTANCE_DX12_H

#include "gfx/rhi/dx12/descriptor_heap_dx12.h"
#include "gfx/rhi/name.h"
#include "gfx/rhi/shader_bindable_resource.h"
#include "gfx/rhi/shader_binding_instance_combiner.h"

#include <cstdint>

namespace game_engine {

struct CommandBufferDx12;

struct ShaderBindingInstanceDx12 : public ShaderBindingInstance {
  virtual ~ShaderBindingInstanceDx12() {}

  virtual void Initialize(
      const ShaderBindingArray& InShaderBindingArray) override;
  virtual void UpdateShaderBindings(
      const ShaderBindingArray& InShaderBindingArray) override;
  virtual void* GetHandle() const override;

  virtual const std::vector<uint32_t>* GetDynamicOffsets() const override {
    return 0;
  }

  virtual void Free() override;

  void BindGraphics(CommandBufferDx12* InCommandList,
                    std::int32_t&        InOutStartIndex) const;
  void BindCompute(CommandBufferDx12* InCommandList,
                   std::int32_t&        InOutStartIndex);
  void CopyToOnlineDescriptorHeap(CommandBufferDx12* InCommandList);


  // TODO: consider using only enum class
  struct InlineRootParamType {
    enum Enum {
      CBV = 0,
      SRV,
      UAV,
      NumOfType
    };
  };

  struct InlineRootParamData {
    InlineRootParamType::Enum Type = InlineRootParamType::NumOfType;
    D3D12_GPU_VIRTUAL_ADDRESS  GPUVirtualAddress = {};

    // TODO: This is debug information, so it would be good to exclude it at
    // runtime.
    Name                          ResourceName;
    const ShaderBindableResource* Resource = nullptr;
  };

  struct DescriptorData {
    inline bool IsValid() const { return Descriptor.IsValid(); }

    DescriptorDx12 Descriptor;

    // TODO: This is debug information, so it would be good to exclude it at
    // runtime.
    Name                          ResourceName;
    const ShaderBindableResource* Resource = nullptr;
  };

  std::vector<InlineRootParamData> RootParameterInlines;
  std::vector<DescriptorData>      Descriptors;
  std::vector<DescriptorData>      SamplerDescriptors;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_BINDING_INSTANCE_DX12_H