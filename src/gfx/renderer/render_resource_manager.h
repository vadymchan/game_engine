#ifndef GAME_ENGINE_RENDER_RESOURCE_MANAGER_H
#define GAME_ENGINE_RENDER_RESOURCE_MANAGER_H

#include "gfx/rhi/interface/buffer.h"
#include "gfx/rhi/interface/descriptor.h"
#include "gfx/rhi/interface/device.h"
#include "gfx/rhi/interface/framebuffer.h"
#include "gfx/rhi/interface/pipeline.h"
#include "gfx/rhi/interface/render_pass.h"
#include "gfx/rhi/interface/sampler.h"
#include "gfx/rhi/interface/shader.h"
#include "gfx/rhi/interface/texture.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace game_engine {
namespace gfx {
namespace renderer {

/**
 * RenderResourceManager handles lifetime and ownership of rendering resources.
 *
 * This class maintains separate containers for different resource types and
 * provides methods to add, cache, and access those resources in a type-safe manner.
 */
class RenderResourceManager {
  public:
  RenderResourceManager()  = default;
  ~RenderResourceManager() = default;

  //--------------------------------------------------------------------------
  // Buffer management
  //--------------------------------------------------------------------------
  rhi::Buffer* addBuffer(std::unique_ptr<rhi::Buffer> buffer, const std::string& cacheKey = "") {
    rhi::Buffer* ptr = buffer.get();

    if (cacheKey.empty()) {
      m_buffers.push_back(std::move(buffer));
    } else {
      m_cachedBuffers[cacheKey] = std::move(buffer);
    }

    return ptr;
  }

  rhi::Buffer* getBuffer(const std::string& cacheKey) {
    auto it = m_cachedBuffers.find(cacheKey);
    if (it != m_cachedBuffers.end()) {
      return it->second.get();
    }
    return nullptr;
  }

  //--------------------------------------------------------------------------
  // Texture management
  //--------------------------------------------------------------------------
  rhi::Texture* addTexture(std::unique_ptr<rhi::Texture> texture, const std::string& cacheKey = "") {
    rhi::Texture* ptr = texture.get();

    if (cacheKey.empty()) {
      m_textures.push_back(std::move(texture));
    } else {
      m_cachedTextures[cacheKey] = std::move(texture);
    }

    return ptr;
  }

  rhi::Texture* getTexture(const std::string& cacheKey) {
    auto it = m_cachedTextures.find(cacheKey);
    if (it != m_cachedTextures.end()) {
      return it->second.get();
    }
    return nullptr;
  }

  //--------------------------------------------------------------------------
  // Sampler management
  //--------------------------------------------------------------------------
  rhi::Sampler* addSampler(std::unique_ptr<rhi::Sampler> sampler, const std::string& cacheKey = "") {
    rhi::Sampler* ptr = sampler.get();

    if (cacheKey.empty()) {
      m_samplers.push_back(std::move(sampler));
    } else {
      m_cachedSamplers[cacheKey] = std::move(sampler);
    }

    return ptr;
  }

  rhi::Sampler* getSampler(const std::string& cacheKey) {
    auto it = m_cachedSamplers.find(cacheKey);
    if (it != m_cachedSamplers.end()) {
      return it->second.get();
    }
    return nullptr;
  }

  //--------------------------------------------------------------------------
  // DescriptorSetLayout management
  //--------------------------------------------------------------------------
  rhi::DescriptorSetLayout* addDescriptorSetLayout(std::unique_ptr<rhi::DescriptorSetLayout> layout,
                                                   const std::string&                        cacheKey = "") {
    rhi::DescriptorSetLayout* ptr = layout.get();

    if (cacheKey.empty()) {
      m_descriptorSetLayouts.push_back(std::move(layout));
    } else {
      m_cachedDescriptorSetLayouts[cacheKey] = std::move(layout);
    }

    return ptr;
  }

  rhi::DescriptorSetLayout* getDescriptorSetLayout(const std::string& cacheKey) {
    auto it = m_cachedDescriptorSetLayouts.find(cacheKey);
    if (it != m_cachedDescriptorSetLayouts.end()) {
      return it->second.get();
    }
    return nullptr;
  }

  //--------------------------------------------------------------------------
  // DescriptorSet management
  //--------------------------------------------------------------------------
  rhi::DescriptorSet* addDescriptorSet(std::unique_ptr<rhi::DescriptorSet> set, const std::string& cacheKey = "") {
    rhi::DescriptorSet* ptr = set.get();

    if (cacheKey.empty()) {
      m_descriptorSets.push_back(std::move(set));
    } else {
      m_cachedDescriptorSets[cacheKey] = std::move(set);
    }

    return ptr;
  }

  rhi::DescriptorSet* getDescriptorSet(const std::string& cacheKey) {
    auto it = m_cachedDescriptorSets.find(cacheKey);
    if (it != m_cachedDescriptorSets.end()) {
      return it->second.get();
    }
    return nullptr;
  }

  //--------------------------------------------------------------------------
  // Pipeline management
  //--------------------------------------------------------------------------
  rhi::GraphicsPipeline* addPipeline(std::unique_ptr<rhi::GraphicsPipeline> pipeline,
                                     const std::string&                     cacheKey = "") {
    rhi::GraphicsPipeline* ptr = pipeline.get();

    if (cacheKey.empty()) {
      m_pipelines.push_back(std::move(pipeline));
    } else {
      m_cachedPipelines[cacheKey] = std::move(pipeline);
    }

    return ptr;
  }

  rhi::GraphicsPipeline* getPipeline(const std::string& cacheKey) {
    auto it = m_cachedPipelines.find(cacheKey);
    if (it != m_cachedPipelines.end()) {
      return it->second.get();
    }
    return nullptr;
  }

  //--------------------------------------------------------------------------
  // RenderPass management
  //--------------------------------------------------------------------------
  rhi::RenderPass* addRenderPass(std::unique_ptr<rhi::RenderPass> renderPass, const std::string& cacheKey = "") {
    rhi::RenderPass* ptr = renderPass.get();

    if (cacheKey.empty()) {
      m_renderPasses.push_back(std::move(renderPass));
    } else {
      m_cachedRenderPasses[cacheKey] = std::move(renderPass);
    }

    return ptr;
  }

  rhi::RenderPass* getRenderPass(const std::string& cacheKey) {
    auto it = m_cachedRenderPasses.find(cacheKey);
    if (it != m_cachedRenderPasses.end()) {
      return it->second.get();
    }
    return nullptr;
  }

  //--------------------------------------------------------------------------
  // Framebuffer management
  //--------------------------------------------------------------------------
  rhi::Framebuffer* addFramebuffer(std::unique_ptr<rhi::Framebuffer> framebuffer, const std::string& cacheKey = "") {
    rhi::Framebuffer* ptr = framebuffer.get();

    if (cacheKey.empty()) {
      m_framebuffers.push_back(std::move(framebuffer));
    } else {
      m_cachedFramebuffers[cacheKey] = std::move(framebuffer);
    }

    return ptr;
  }

  rhi::Framebuffer* getFramebuffer(const std::string& cacheKey) {
    auto it = m_cachedFramebuffers.find(cacheKey);
    if (it != m_cachedFramebuffers.end()) {
      return it->second.get();
    }
    return nullptr;
  }

  // Clear all resources
  void clear() {
    m_buffers.clear();
    m_textures.clear();
    m_samplers.clear();
    m_descriptorSetLayouts.clear();
    m_descriptorSets.clear();
    m_pipelines.clear();
    m_renderPasses.clear();
    m_framebuffers.clear();

    m_cachedBuffers.clear();
    m_cachedTextures.clear();
    m_cachedSamplers.clear();
    m_cachedDescriptorSetLayouts.clear();
    m_cachedDescriptorSets.clear();
    m_cachedPipelines.clear();
    m_cachedRenderPasses.clear();
    m_cachedFramebuffers.clear();
  }

  private:
  std::vector<std::unique_ptr<rhi::Buffer>>              m_buffers;
  std::vector<std::unique_ptr<rhi::Texture>>             m_textures;
  std::vector<std::unique_ptr<rhi::Sampler>>             m_samplers;
  std::vector<std::unique_ptr<rhi::DescriptorSetLayout>> m_descriptorSetLayouts;
  std::vector<std::unique_ptr<rhi::DescriptorSet>>       m_descriptorSets;
  std::vector<std::unique_ptr<rhi::GraphicsPipeline>>    m_pipelines;
  std::vector<std::unique_ptr<rhi::RenderPass>>          m_renderPasses;
  std::vector<std::unique_ptr<rhi::Framebuffer>>         m_framebuffers;

  // Cached resources
  std::unordered_map<std::string, std::unique_ptr<rhi::Buffer>>              m_cachedBuffers;
  std::unordered_map<std::string, std::unique_ptr<rhi::Texture>>             m_cachedTextures;
  std::unordered_map<std::string, std::unique_ptr<rhi::Sampler>>             m_cachedSamplers;
  std::unordered_map<std::string, std::unique_ptr<rhi::DescriptorSetLayout>> m_cachedDescriptorSetLayouts;
  std::unordered_map<std::string, std::unique_ptr<rhi::DescriptorSet>>       m_cachedDescriptorSets;
  std::unordered_map<std::string, std::unique_ptr<rhi::GraphicsPipeline>>    m_cachedPipelines;
  std::unordered_map<std::string, std::unique_ptr<rhi::RenderPass>>          m_cachedRenderPasses;
  std::unordered_map<std::string, std::unique_ptr<rhi::Framebuffer>>         m_cachedFramebuffers;
};

}  // namespace renderer
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_RESOURCE_MANAGER_H