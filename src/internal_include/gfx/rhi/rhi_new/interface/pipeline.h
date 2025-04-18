#ifndef GAME_ENGINE_PIPELINE_H
#define GAME_ENGINE_PIPELINE_H

#include "gfx/rhi/rhi_new/common/rhi_enums.h"
#include "gfx/rhi/rhi_new/common/rhi_types.h"

namespace game_engine {
namespace gfx {
namespace rhi {

/*
 * A pipeline represents the configuration of the graphics/compute pipeline
 * stages and states for rendering or computation. It encapsulates shaders,
 * fixed-function state, and pipeline layout information.
 */
class Pipeline {
  public:
  virtual ~Pipeline() = default;

  virtual PipelineType getType() const = 0;
};

/**
 * Represents a complete graphics pipeline state object (PSO) that can be bound for rendering.
 * This includes vertex input configuration, shader stages, and fixed-function state.
 */
class GraphicsPipeline : public Pipeline {
  public:
  GraphicsPipeline(const GraphicsPipelineDesc& desc)
      : m_desc_(desc) {}

  virtual ~GraphicsPipeline() = default;

  PipelineType getType() const { return PipelineType::Graphics; }

  const GraphicsPipelineDesc& getDesc() const { return m_desc_; }

  PrimitiveType getPrimitiveType() const { return m_desc_.inputAssembly.topology; }

  protected:
  GraphicsPipelineDesc m_desc_;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_PIPELINE_H