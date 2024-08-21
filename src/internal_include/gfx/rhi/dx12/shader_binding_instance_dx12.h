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

  virtual void initialize(
      const ShaderBindingArray& shaderBindingArray) override;
  virtual void updateShaderBindings(
      const ShaderBindingArray& shaderBindingArray) override;
  virtual void* getHandle() const override;

  virtual const std::vector<uint32_t>* getDynamicOffsets() const override {
    return 0;
  }

  virtual void free() override;

  void bindGraphics(CommandBufferDx12* commandList,
                    std::int32_t&      startIndex) const;
  void bindCompute(CommandBufferDx12* commandList, std::int32_t& startIndex);
  void copyToOnlineDescriptorHeap(CommandBufferDx12* commandList);

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
    InlineRootParamType::Enum m_type_ = InlineRootParamType::NumOfType;
    D3D12_GPU_VIRTUAL_ADDRESS m_gpuVirtualAddress_ = {};

    // TODO:
    // - Seems like not used
    // - This is debug information, so it would be good to exclude it at
    // runtime.
    Name                          m_resourceName_;
    const ShaderBindableResource* m_resource_ = nullptr;
  };

  struct DescriptorData {
    inline bool isValid() const { return m_descriptor_.isValid(); }

    DescriptorDx12 m_descriptor_;

    // TODO: This is debug information, so it would be good to exclude it at
    // runtime.
    Name                          m_resourceName_;
    const ShaderBindableResource* m_resource_ = nullptr;
  };

  std::vector<InlineRootParamData> m_rootParameterInlines_;
  std::vector<DescriptorData>      m_descriptors_;
  std::vector<DescriptorData>      m_samplerDescriptors_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_BINDING_INSTANCE_DX12_H