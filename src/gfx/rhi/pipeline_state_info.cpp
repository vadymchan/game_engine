#include "gfx/rhi/pipeline_state_info.h"

#include <cassert>

namespace game_engine {

size_t PipelineStateInfo::getHash() const {
  if (m_hash_) {
    return m_hash_;
  }

  m_hash_ = 0;
  if (m_pipelineType_ == EPipelineType::Graphics) {
    assert(m_pipelineStateFixed_);
    m_hash_ ^= m_pipelineStateFixed_->createHash();
    m_hash_ ^= m_vertexBufferArray.getHash();
    m_hash_ ^= m_renderPass->getHash();
    m_hash_ ^= m_graphicsShader_.getHash();
  } else if (m_pipelineType_ == EPipelineType::Compute) {
    assert(m_computeShader_);
    m_hash_ ^= m_computeShader_->m_shaderInfo_.getHash();
  } else if (m_pipelineType_ == EPipelineType::RayTracing) {
    // TODO: implement
    assert(0);
    /*for (int32_t i = 0; i < (int32_t)RaytracingShaders.size(); ++i) {
      Hash ^= RaytracingShaders[i].s_getHash();
    }
    Hash ^= RaytracingPipelineData.s_getHash();*/
  } else {
    assert(0);
  }

  m_hash_ ^= m_shaderBindingLayoutArray.getHash();

  if (m_pushConstant) {
    m_hash_ ^= m_pushConstant->getHash();
  }
  m_hash_ ^= m_subpassIndex_;

  return m_hash_;
}

}  // namespace game_engine