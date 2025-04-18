#ifndef GAME_ENGINE_PIPELINE_DX12_H
#define GAME_ENGINE_PIPELINE_DX12_H

#include "gfx/rhi/rhi_new/interface/pipeline.h"
#include "platform/windows/windows_platform_setup.h"

#ifdef GAME_ENGINE_RHI_DX12

#include <array>
#include <memory>
#include <unordered_map>
#include <vector>

namespace game_engine {
namespace gfx {
namespace rhi {

class DeviceDx12;
class ShaderDx12;
class RenderPassDx12;
class DescriptorSetLayoutDx12;

class GraphicsPipelineDx12 : public GraphicsPipeline {
  public:
  GraphicsPipelineDx12(const GraphicsPipelineDesc& desc, DeviceDx12* device);
  ~GraphicsPipelineDx12() = default;

  GraphicsPipelineDx12(const GraphicsPipelineDx12&)            = delete;
  GraphicsPipelineDx12& operator=(const GraphicsPipelineDx12&) = delete;

  const std::array<float, 4>& getBlendFactors() const { return m_blendFactors_; }

  // DirectX 12-specific methods
  ID3D12PipelineState* getPipelineState() const { return m_pipelineState_.Get(); }

  ID3D12RootSignature* getRootSignature() const { return m_rootSignature_.Get(); }

  private:
  bool initialize_();

  bool createRootSignature_();
  bool createPipelineState_();

  bool collectShaders_(ComPtr<ID3DBlob>& vertexShader,
                       ComPtr<ID3DBlob>& pixelShader,
                       ComPtr<ID3DBlob>& domainShader,
                       ComPtr<ID3DBlob>& hullShader,
                       ComPtr<ID3DBlob>& geometryShader);

  DeviceDx12* m_device_;

  ComPtr<ID3D12PipelineState> m_pipelineState_;
  ComPtr<ID3D12RootSignature> m_rootSignature_;

  // Store blend factors separately since D3D12 doesn't include them in the blend state
  std::array<float, 4> m_blendFactors_;

  // Map of parameter indices for descriptor tables
  // TODO: define an alias for uint32_t
  std::unordered_map<uint32_t, uint32_t> m_descriptorTableMap;

  // Resources owned by this pipeline
  std::vector<std::unique_ptr<DescriptorSetLayout>> m_ownedDescriptorSetLayouts_;
  std::vector<std::unique_ptr<Shader>>              m_ownedShaders_;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12
#endif  // GAME_ENGINE_PIPELINE_DX12_H