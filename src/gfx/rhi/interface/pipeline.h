#ifndef GAME_ENGINE_PIPELINE_H
#define GAME_ENGINE_PIPELINE_H

#include "gfx/rhi/common/rhi_enums.h"
#include "gfx/rhi/common/rhi_types.h"
#include "gfx/rhi/shader_reload_observer.h"

namespace game_engine {
namespace gfx {
namespace rhi {

class ShaderManager;

/*
 * A pipeline represents the configuration of the graphics/compute pipeline
 * stages and states for rendering or computation. It encapsulates shaders,
 * fixed-function state, and pipeline layout information.
 */
class Pipeline : public ShaderReloadObserver {
  public:
  virtual ~Pipeline() = default;

  virtual PipelineType getType() const = 0;

  virtual const std::vector<Shader*>& getShaders() const = 0;

  void onShaderReloaded(Shader* shader) override {
    for (auto& pipelineShader : getShaders()) {
      if (pipelineShader == shader) {
        rebuildPipeline();
        return;
      }
    }
  }

  virtual void rebuildPipeline() = 0;
};

/**
 * Represents a complete graphics pipeline state object (PSO) that can be bound for rendering.
 * This includes vertex input configuration, shader stages, and fixed-function state.
 */
class GraphicsPipeline : public Pipeline {
  public:
  GraphicsPipeline(const GraphicsPipelineDesc& desc, ShaderManager* shaderManager)
      : m_desc_(desc)
      , m_shaderManager_(shaderManager) {}

  virtual ~GraphicsPipeline() = default;

  PipelineType getType() const { return PipelineType::Graphics; }

  const GraphicsPipelineDesc& getDesc() const { return m_desc_; }

  PrimitiveType getPrimitiveType() const { return m_desc_.inputAssembly.topology; }

  const std::vector<Shader*>& getShaders() const override { return m_desc_.shaders; }

  protected:
  GraphicsPipelineDesc m_desc_;
  ShaderManager*       m_shaderManager_;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_PIPELINE_H