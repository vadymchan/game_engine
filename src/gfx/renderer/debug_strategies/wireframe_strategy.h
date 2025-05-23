#ifndef GAME_ENGINE_WIREFRAME_STRATEGY_H
#define GAME_ENGINE_WIREFRAME_STRATEGY_H

#include "gfx/renderer/debug_strategies/debug_draw_strategy.h"
#include "gfx/rhi/interface/render_pass.h"

#include <unordered_map>
#include <vector>

namespace game_engine {
struct RenderModel;
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
 * Renders scene geometry as wireframe
 */
class WireframeStrategy : public DebugDrawStrategy {
  public:
  WireframeStrategy() = default;

  ~WireframeStrategy() override { cleanup(); }

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
    rhi::GraphicsPipeline* pipeline       = nullptr;
    rhi::Buffer*           vertexBuffer   = nullptr;
    rhi::Buffer*           indexBuffer    = nullptr;
    rhi::Buffer*           instanceBuffer = nullptr;
    uint32_t               indexCount     = 0;
    uint32_t               instanceCount  = 0;
  };

  void setupRenderPass_();
  void createFramebuffers_(const math::Dimension2Di& dimension);
  void prepareDrawCalls_(const RenderContext& context);
  void updateInstanceBuffer_(RenderModel*                         model,
                             const std::vector<math::Matrix4f<>>& matrices,
                             ModelBufferCache&                    cache);
  void cleanupUnusedBuffers_(
      const std::unordered_map<RenderModel*, std::vector<math::Matrix4f<>>>& currentFrameInstances);

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
};

}  // namespace renderer
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_WIREFRAME_STRATEGY_H