#ifndef GAME_ENGINE_LIGHT_VISUALIZATION_STRATEGY_H
#define GAME_ENGINE_LIGHT_VISUALIZATION_STRATEGY_H

#include "gfx/renderer/debug_strategies/debug_draw_strategy.h"
#include "gfx/rhi/interface/render_pass.h"

#include <unordered_map>
#include <vector>

namespace game_engine {
struct RenderModel;
struct Material;
}  // namespace game_engine

namespace game_engine::gfx::rhi {
class Buffer;
class DescriptorSet;
class GraphicsPipeline;
}  // namespace game_engine::gfx::rhi

namespace game_engine {
namespace gfx {
namespace renderer {

/**
 * Visualizes lighting information - light colors and NdotL factors
 */
class LightVisualizationStrategy : public DebugDrawStrategy {
  public:
  LightVisualizationStrategy() = default;

  ~LightVisualizationStrategy() override { cleanup(); }

  void initialize(rhi::Device*           device,
                  RenderResourceManager* resourceManager,
                  FrameResources*        frameResources,
                  rhi::ShaderManager*    shaderManager) override;

  void resize(const math::Dimension2Di& newDimension) override;
  void prepareFrame(const RenderContext& context) override;
  void render(const RenderContext& context) override;
  void cleanup() override;

  bool isExclusive() const override { return true; }

  private:
  struct ModelBufferCache {
    rhi::Buffer* instanceBuffer = nullptr;
    uint32_t     capacity       = 0;
    uint32_t     count          = 0;
  };

  struct DrawData {
    rhi::GraphicsPipeline* pipeline                 = nullptr;
    rhi::DescriptorSet*    modelMatrixDescriptorSet = nullptr;
    rhi::DescriptorSet*    materialDescriptorSet    = nullptr;
    rhi::Buffer*           vertexBuffer             = nullptr;
    rhi::Buffer*           indexBuffer              = nullptr;
    rhi::Buffer*           instanceBuffer           = nullptr;
    uint32_t               indexCount               = 0;
    uint32_t               instanceCount            = 0;
  };

  rhi::DescriptorSet* getOrCreateMaterialDescriptorSet_(Material* material);

  void setupRenderPass_();
  void createFramebuffers_(const math::Dimension2Di& dimension);
  void prepareDrawCalls_(const RenderContext& context);
  void updateInstanceBuffer_(RenderModel*                         model,
                             const std::vector<math::Matrix4f<>>& matrices,
                             ModelBufferCache&                    cache);
  void cleanupUnusedBuffers_(
      const std::unordered_map<RenderModel*, std::vector<math::Matrix4f<>>>& currentFrameInstances);

  const std::string m_vertexShaderPath_ = "assets/shaders/debug/light_visualization/shader_instancing.vs.hlsl";
  const std::string m_pixelShaderPath_  = "assets/shaders/debug/light_visualization/shader.ps.hlsl";

  rhi::Device*           m_device          = nullptr;
  RenderResourceManager* m_resourceManager = nullptr;
  FrameResources*        m_frameResources  = nullptr;
  rhi::ShaderManager*    m_shaderManager   = nullptr;

  rhi::Shader* m_vertexShader = nullptr;
  rhi::Shader* m_pixelShader  = nullptr;

  rhi::Viewport    m_viewport;
  rhi::ScissorRect m_scissor;

  rhi::RenderPass*               m_renderPass = nullptr;
  std::vector<rhi::Framebuffer*> m_framebuffers;

  std::unordered_map<RenderModel*, ModelBufferCache> m_instanceBufferCache;
  std::vector<DrawData>                              m_drawData;

  struct MaterialCache {
    rhi::DescriptorSet* descriptorSet = nullptr;
  };

  std::unordered_map<Material*, MaterialCache> m_materialCache;
  rhi::DescriptorSetLayout*                    m_materialDescriptorSetLayout = nullptr;
};

}  // namespace renderer
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_LIGHT_VISUALIZATION_STRATEGY_H