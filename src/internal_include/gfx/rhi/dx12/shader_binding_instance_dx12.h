#ifndef GAME_ENGINE_SHADER_BINDING_INSTANCE_DX12_H
#define GAME_ENGINE_SHADER_BINDING_INSTANCE_DX12_H

#include "gfx/rhi/dx12/descriptor_heap_dx12.h"
#include "gfx/rhi/name.h"
#include "gfx/rhi/shader_bindable_resource.h"
#include "gfx/rhi/shader_binding_instance_combiner.h"

#include <cstdint>

namespace game_engine {

struct jCommandBuffer_DX12;

struct jShaderBindingInstance_DX12 : public jShaderBindingInstance {
  virtual ~jShaderBindingInstance_DX12() {}

  virtual void Initialize(
      const jShaderBindingArray& InShaderBindingArray) override;
  virtual void UpdateShaderBindings(
      const jShaderBindingArray& InShaderBindingArray) override;
  virtual void* GetHandle() const override;

  virtual const std::vector<uint32_t>* GetDynamicOffsets() const override {
    return 0;
  }

  virtual void Free() override;

  void BindGraphics(jCommandBuffer_DX12* InCommandList,
                    std::int32_t&        InOutStartIndex) const;
  void BindCompute(jCommandBuffer_DX12* InCommandList,
                   std::int32_t&        InOutStartIndex);
  void CopyToOnlineDescriptorHeap(jCommandBuffer_DX12* InCommandList);

  struct jInlineRootParamType {
    enum Enum {
      CBV = 0,
      SRV,
      UAV,
      NumOfType
    };
  };

  struct jInlineRootParamData {
    jInlineRootParamType::Enum Type = jInlineRootParamType::NumOfType;
    D3D12_GPU_VIRTUAL_ADDRESS  GPUVirtualAddress = {};

    // TODO: This is debug information, so it would be good to exclude it at
    // runtime.
    Name                          ResourceName;
    const ShaderBindableResource* Resource = nullptr;
  };

  struct jDescriptorData {
    inline bool IsValid() const { return Descriptor.IsValid(); }

    jDescriptor_DX12 Descriptor;

    // TODO: This is debug information, so it would be good to exclude it at
    // runtime.
    Name                          ResourceName;
    const ShaderBindableResource* Resource = nullptr;
  };

  std::vector<jInlineRootParamData> RootParameterInlines;
  std::vector<jDescriptorData>      Descriptors;
  std::vector<jDescriptorData>      SamplerDescriptors;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_BINDING_INSTANCE_DX12_H