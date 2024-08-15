#ifndef GAME_ENGINE_SHADER_BINDING_LAYOUT_DX12_H
#define GAME_ENGINE_SHADER_BINDING_LAYOUT_DX12_H

#include "gfx/rhi/lock.h"
#include "gfx/rhi/shader_binding_layout.h"
#include "platform/windows/windows_platform_setup.h"

#include <unordered_map>
#include <vector>

namespace game_engine {

struct RootParameterExtractor {
  public:
  std::vector<D3D12_ROOT_PARAMETER1>   RootParameters;
  std::vector<D3D12_DESCRIPTOR_RANGE1> Descriptors;
  std::vector<D3D12_DESCRIPTOR_RANGE1> SamplerDescriptors;

  void Extract(const ShaderBindingLayoutArray& InBindingLayoutArray,
               int32_t                          InRegisterSpace = 0);
  void Extract(const ShaderBindingInstanceArray& InBindingLayoutArray,
               int32_t                            InRegisterSpace = 0);

  protected:
  void Extract(int32_t&                   InOutDescriptorOffset,
               int32_t&                   InOutSamplerDescriptorOffset,
               const ShaderBindingArray& InShaderBindingArray,
               int32_t                    InRegisterSpace = 0);

  private:
  int32_t NumOfInlineRootParameter = 0;
};

struct ShaderBindingLayoutDx12 : public ShaderBindingLayout {
  virtual ~ShaderBindingLayoutDx12() {}

  virtual bool Initialize(
      const ShaderBindingArray& InShaderBindingArray) override;
  virtual std::shared_ptr<ShaderBindingInstance> CreateShaderBindingInstance(
      const ShaderBindingArray&       InShaderBindingArray,
      const ShaderBindingInstanceType InType) const override;

  virtual void* GetHandle() const { return nullptr; }

  std::vector<D3D12_ROOT_PARAMETER1>   RootParameters;
  std::vector<D3D12_DESCRIPTOR_RANGE1> Descriptors;
  std::vector<D3D12_DESCRIPTOR_RANGE1> SamplerDescriptors;

  static std::unordered_map<size_t, ComPtr<ID3D12RootSignature>>
                     GRootSignaturePool;
  static MutexRWLock GRootSignatureLock;

  //////////////////////////////////////////////////////////////////////////
  // RootSignature extractor utility
  using FuncGetRootParameterExtractor
      = std::function<void(RootParameterExtractor&)>;

  static ID3D12RootSignature* CreateRootSignatureInternal(
      size_t InHash, FuncGetRootParameterExtractor InFunc);
  static ID3D12RootSignature* CreateRootSignature(
      const ShaderBindingInstanceArray& InBindingInstanceArray);
  static ID3D12RootSignature* CreateRootSignature(
      const ShaderBindingLayoutArray& InBindingLayoutArray);
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_BINDING_LAYOUT_DX12_H