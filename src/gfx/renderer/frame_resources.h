#ifndef GAME_ENGINE_FRAME_RESOURCES_H
#define GAME_ENGINE_FRAME_RESOURCES_H

#include "ecs/components/render_model.h"
#include "ecs/components/transform.h"
#include "gfx/renderer/render_context.h"
#include "gfx/rhi/interface/buffer.h"
#include "gfx/rhi/interface/descriptor.h"
#include "gfx/rhi/interface/device.h"
#include "gfx/rhi/interface/sampler.h"
#include "gfx/rhi/interface/texture.h"
#include "utils/math/math_util.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace game_engine {
namespace gfx {
namespace renderer {

class RenderResourceManager;

/**
 * Manages shared resources used across different render passes
 */
class FrameResources {
  public:
  FrameResources(rhi::Device* device, RenderResourceManager* resourceManager);

  ~FrameResources() { cleanup(); }

  void initialize(uint32_t framesCount);

  /**
   * Resize resources when viewport (window / editor viewport) changes
   */
  void resize(const math::Dimension2Di& newDimension);

  void updatePerFrameResources(const RenderContext& context);

  void cleanup();

  struct RenderTargets {
    std::unique_ptr<rhi::Texture> colorBuffer;
    std::unique_ptr<rhi::Texture> depthBuffer;
    rhi::Texture*                 backBuffer = nullptr;
  };

  RenderTargets& getRenderTargets(uint32_t frameIndex) {
    return m_renderTargetsPerFrame[frameIndex % m_renderTargetsPerFrame.size()];
  }

  uint32_t getFramesCount() const { return m_renderTargetsPerFrame.size(); }

  const rhi::Viewport& getViewport() const { return m_viewport; }

  const rhi::ScissorRect& getScissor() const { return m_scissor; }

  rhi::DescriptorSet* getViewDescriptorSet() const { return m_viewDescriptorSet; }

  rhi::DescriptorSet* getDirectionalLightDescriptorSet() const { return m_directionalLightDescriptorSet; }

  rhi::DescriptorSet* getPointLightDescriptorSet() const { return m_pointLightDescriptorSet; }

  rhi::DescriptorSet* getSpotLightDescriptorSet() const { return m_spotLightDescriptorSet; }

  rhi::DescriptorSet* getDefaultSamplerDescriptorSet() const { return m_defaultSamplerDescriptorSet; }

  rhi::Sampler* getDefaultSampler() const { return m_defaultSampler; }

  struct ModelInstance {
    RenderModel*     model;
    Transform        transform;
    math::Matrix4f<> modelMatrix;
    entt::entity     entityId;

    uint32_t materialId = 0;  // Material ID for sorting

    bool isDirty = false;
  };

  /**
   * Returns models sorted by material ID for optimal rendering batching.
   * This helps reduce state changes during rendering.
   */
  const std::vector<ModelInstance*>& getModels() const { return m_sortedModels; }

  rhi::DescriptorSetLayout* getViewDescriptorSetLayout() const { return m_viewDescriptorSetLayout; }

  rhi::DescriptorSetLayout* getLightDescriptorSetLayout() const { return m_lightDescriptorSetLayout; }

  rhi::DescriptorSetLayout* getMaterialDescriptorSetLayout() const { return m_materialDescriptorSetLayout; }

  private:
  void createViewDescriptorSetLayout_();
  void createLightDescriptorSetLayouts_();
  void createMaterialDescriptorSetLayout_();
  void createDefaultSampler_();
  void createSamplerDescriptorSet_();
  void createRenderTargets_(RenderTargets& targets, const math::Dimension2Di& dimensions);

  void updateViewResources_(const RenderContext& context);
  void updateLightResources_(const RenderContext& context);
  void updateModelList_(const RenderContext& context);

  void sortModelsByMaterial_();

  void clearInternalDirtyFlags_();
  void clearEntityDirtyFlags_(const RenderContext& context);

  rhi::Device*           m_device          = nullptr;
  RenderResourceManager* m_resourceManager = nullptr;
  FrameResources*        m_frameResources  = nullptr;

  bool m_initialized = false;

  rhi::Viewport    m_viewport;
  rhi::ScissorRect m_scissor;

  std::vector<RenderTargets> m_renderTargetsPerFrame;

  rhi::DescriptorSet* m_viewDescriptorSet             = nullptr;
  rhi::DescriptorSet* m_directionalLightDescriptorSet = nullptr;
  rhi::DescriptorSet* m_pointLightDescriptorSet       = nullptr;
  rhi::DescriptorSet* m_spotLightDescriptorSet        = nullptr;
  rhi::DescriptorSet* m_defaultSamplerDescriptorSet   = nullptr;

  rhi::DescriptorSetLayout* m_viewDescriptorSetLayout     = nullptr;
  rhi::DescriptorSetLayout* m_lightDescriptorSetLayout    = nullptr;
  rhi::DescriptorSetLayout* m_materialDescriptorSetLayout = nullptr;

  rhi::Buffer* m_viewUniformBuffer             = nullptr;
  rhi::Buffer* m_directionalLightUniformBuffer = nullptr;
  rhi::Buffer* m_pointLightUniformBuffer       = nullptr;
  rhi::Buffer* m_spotLightUniformBuffer        = nullptr;

  rhi::Sampler* m_defaultSampler = nullptr;

  std::unordered_map<entt::entity, ModelInstance> m_modelsMap;
  std::vector<ModelInstance*>                     m_sortedModels;
};

}  // namespace renderer
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_FRAME_RESOURCES_H