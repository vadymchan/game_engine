#ifndef GAME_ENGINE_SHADER_BINDING_LAYOUT_DX12_H
#define GAME_ENGINE_SHADER_BINDING_LAYOUT_DX12_H

#include "platform/windows/windows_platform_setup.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "gfx/rhi/lock.h"
#include "gfx/rhi/shader_binding_layout.h"

#include <unordered_map>
#include <vector>

namespace game_engine {

struct RootParameterExtractor {
  public:
  // ======= BEGIN: public misc methods =======================================

  void extract(const ShaderBindingLayoutArray& bindingLayoutArray,
               int32_t                         registerSpace = 0);
  void extract(const ShaderBindingInstanceArray& bindingLayoutArray,
               int32_t                           registerSpace = 0);

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  std::vector<D3D12_ROOT_PARAMETER1>   m_rootParameters_;
  std::vector<D3D12_DESCRIPTOR_RANGE1> m_descriptors_;
  std::vector<D3D12_DESCRIPTOR_RANGE1> m_samplerDescriptors_;

  // ======= END: public misc fields   ========================================

  protected:
  // ======= BEGIN: protected misc methods ====================================

  void extract_(int32_t&                  descriptorOffset,
                int32_t&                  samplerDescriptorOffset,
                const ShaderBindingArray& shaderBindingArray,
                int32_t                   registerSpace = 0);

  // ======= END: protected misc methods   ====================================

  private:
  // ======= BEGIN: private misc fields =======================================

  int32_t m_numOfInlineRootParameter_ = 0;

  // ======= END: private misc fields   =======================================
};

struct ShaderBindingLayoutDx12 : public ShaderBindingLayout {
  virtual ~ShaderBindingLayoutDx12() {}

  virtual bool initialize(
      const ShaderBindingArray& shaderBindingArray) override;
  virtual std::shared_ptr<ShaderBindingInstance> createShaderBindingInstance(
      const ShaderBindingArray&       shaderBindingArray,
      const ShaderBindingInstanceType type) override;

  virtual void* getHandle() const { return nullptr; }

  //////////////////////////////////////////////////////////////////////////
  // RootSignature extractor utility
  using FuncGetRootParameterExtractor
      = std::function<void(RootParameterExtractor&)>;

  static ID3D12RootSignature* s_createRootSignatureInternal(
      size_t hash, FuncGetRootParameterExtractor func);
  static ID3D12RootSignature* s_createRootSignature(
      const ShaderBindingInstanceArray& bindingInstanceArray);
  static ID3D12RootSignature* s_createRootSignature(
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

#endif  // GAME_ENGINE_RHI_DX12

#endif  // GAME_ENGINE_SHADER_BINDING_LAYOUT_DX12_H
