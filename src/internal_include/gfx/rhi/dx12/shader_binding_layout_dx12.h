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
  std::vector<D3D12_ROOT_PARAMETER1>   m_rootParameters_;
  std::vector<D3D12_DESCRIPTOR_RANGE1> m_descriptors_;
  std::vector<D3D12_DESCRIPTOR_RANGE1> m_samplerDescriptors_;

  void Extract(const ShaderBindingLayoutArray& bindingLayoutArray,
               int32_t                         registerSpace = 0);
  void Extract(const ShaderBindingInstanceArray& bindingLayoutArray,
               int32_t                           registerSpace = 0);

  protected:
  void Extract(int32_t&                  descriptorOffset,
               int32_t&                  samplerDescriptorOffset,
               const ShaderBindingArray& shaderBindingArray,
               int32_t                   registerSpace = 0);

  private:
  int32_t m_numOfInlineRootParameter_ = 0;
};

struct ShaderBindingLayoutDx12 : public ShaderBindingLayout {
  virtual ~ShaderBindingLayoutDx12() {}

  virtual bool Initialize(
      const ShaderBindingArray& shaderBindingArray) override;
  virtual std::shared_ptr<ShaderBindingInstance> CreateShaderBindingInstance(
      const ShaderBindingArray&       shaderBindingArray,
      const ShaderBindingInstanceType type) const override;

  virtual void* GetHandle() const { return nullptr; }

  //////////////////////////////////////////////////////////////////////////
  // RootSignature extractor utility
  using FuncGetRootParameterExtractor
      = std::function<void(RootParameterExtractor&)>;

  static ID3D12RootSignature* CreateRootSignatureInternal(
      size_t hash, FuncGetRootParameterExtractor func);
  static ID3D12RootSignature* CreateRootSignature(
      const ShaderBindingInstanceArray& bindingInstanceArray);
  static ID3D12RootSignature* CreateRootSignature(
      const ShaderBindingLayoutArray& bindingLayoutArray);

  // TODO: seems unused
  std::vector<D3D12_ROOT_PARAMETER1>   m_rootParameters_;
  std::vector<D3D12_DESCRIPTOR_RANGE1> m_descriptors_;
  std::vector<D3D12_DESCRIPTOR_RANGE1> m_samplerDescriptors_;

  static std::unordered_map<size_t, ComPtr<ID3D12RootSignature>>
                     s_rootSignaturePool;
  static MutexRWLock s_rootSignatureLock;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_BINDING_LAYOUT_DX12_H