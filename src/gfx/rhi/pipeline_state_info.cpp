#include "gfx/rhi/pipeline_state_info.h"

#include <cassert>

namespace game_engine {

size_t PipelineStateInfo::GetHash() const {
  if (m_hash_) {
    return m_hash_;
  }

  m_hash_ = 0;
  if (m_pipelineType_ == EPipelineType::Graphics) {
    assert(m_pipelineStateFixed_);
    m_hash_ ^= m_pipelineStateFixed_->CreateHash();
    m_hash_ ^= m_vertexBufferArray.GetHash();
    m_hash_ ^= m_renderPass->GetHash();
    m_hash_ ^= m_graphicsShader_.GetHash();
  } else if (m_pipelineType_ == EPipelineType::Compute) {
    assert(m_computeShader_);
    m_hash_ ^= m_computeShader_->m_shaderInfo_.GetHash();
  } else if (m_pipelineType_ == EPipelineType::RayTracing) {
    // TODO: implement
    assert(0);
    /*for (int32_t i = 0; i < (int32_t)RaytracingShaders.size(); ++i) {
      Hash ^= RaytracingShaders[i].GetHash();
    }
    Hash ^= RaytracingPipelineData.GetHash();*/
  } else {
    assert(0);
  }

  m_hash_ ^= m_shaderBindingLayoutArray.GetHash();

  if (m_pushConstant) {
    m_hash_ ^= m_pushConstant->GetHash();
  }
  m_hash_ ^= m_subpassIndex_;

  return m_hash_;
}

}  // namespace game_engine