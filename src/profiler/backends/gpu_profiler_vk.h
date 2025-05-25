#ifndef GAME_ENGINE_GPU_PROFILER_VK_H
#define GAME_ENGINE_GPU_PROFILER_VK_H

#include "profiler/backends/gpu_profiler.h"

#ifdef GAME_ENGINE_USE_VULKAN

#include "gfx/rhi/backends/vulkan/command_buffer_vk.h"

// TODO: consider adding more robust check for GPU profiling support
#ifdef GAME_ENGINE_USE_GPU_PROFILING
#include <vulkan/vulkan.h>

#include <tracy/TracyVulkan.hpp>
using TracyVkContextType = TracyVkCtx;
#else
using TracyVkContextType = void*;
#endif

namespace game_engine {
namespace gpu {

class GpuProfilerVk final : public GpuProfiler {
  public:
  GpuProfilerVk() = default;
  ~GpuProfilerVk() override { destroy(); }

  bool initialize(void* physicalDevice, void* device, void* queue, void* commandBuffer) override;
  void destroy() override;
  void setContextName(const std::string& name) override;

  void newFrame() override {}  // Not used in Vulkan
  void collect(void* commandBuffer) override;

  template <uint32_t Color, size_t N>
  void beginZoneNC(gfx::rhi::CommandBuffer* cb, const char (&name)[N]) {
#if defined(TRACY_ENABLE)
    auto* vk = static_cast<gfx::rhi::CommandBufferVk*>(cb);
    TracyVkNamedZoneC(m_tracyContext, _zone, vk->getCommandBuffer(), name, Color, true);
#endif
  }

  template <uint32_t Color>
  void beginZoneC(gfx::rhi::CommandBuffer* cb) {
#if defined(TRACY_ENABLE)
    auto* vk = static_cast<gfx::rhi::CommandBufferVk*>(cb);
    TracyVkZoneC(m_tracyContext, vk->getCommandBuffer(), Color, true);
#endif
  }

  template <size_t N>
  void beginZoneN(gfx::rhi::CommandBuffer* cb, const char (&name)[N]) {
#if defined(TRACY_ENABLE)
    auto* vk = static_cast<gfx::rhi::CommandBufferVk*>(cb);
    TracyVkNamedZone(m_tracyContext, _zone, vk->getCommandBuffer(), name, true);
#endif
  }

  void endZone(gfx::rhi::CommandBuffer* cmdBuffer) override;

  private:
  TracyVkContextType m_tracyContext = nullptr;
  bool               m_initialized  = false;
};

}  // namespace gpu
}  // namespace game_engine

#include "profiler/backends/gpu_profiler.inl"

#endif  // GAME_ENGINE_USE_VULKAN
#endif  // GAME_ENGINE_GPU_PROFILER_VK_H