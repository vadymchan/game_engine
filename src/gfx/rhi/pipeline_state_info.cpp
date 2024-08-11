#include "gfx/rhi/pipeline_state_info.h"

#include <cassert>

namespace game_engine {

size_t PipelineStateInfo::GetHash() const {
  if (Hash) {
    return Hash;
  }

  Hash = 0;
  if (PipelineType == EPipelineType::Graphics) {
    assert(PipelineStateFixed);
    Hash ^= PipelineStateFixed->CreateHash();
    Hash ^= m_vertexBufferArray.GetHash();
    Hash ^= m_renderPass->GetHash();
    Hash ^= GraphicsShader.GetHash();
  } else if (PipelineType == EPipelineType::Compute) {
    assert(ComputeShader);
    Hash ^= ComputeShader->shaderInfo.GetHash();
  } else if (PipelineType == EPipelineType::RayTracing) {
    // TODO: implement
    assert(0);
    /*for (int32_t i = 0; i < (int32_t)RaytracingShaders.size(); ++i) {
      Hash ^= RaytracingShaders[i].GetHash();
    }
    Hash ^= RaytracingPipelineData.GetHash();*/
  } else {
    assert(0);
  }

  Hash ^= m_shaderBindingLayoutArray.GetHash();

  if (m_pushConstant) {
    Hash ^= m_pushConstant->GetHash();
  }
  Hash ^= SubpassIndex;

  return Hash;
}

}  // namespace game_engine