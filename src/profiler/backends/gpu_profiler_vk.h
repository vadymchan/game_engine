#ifndef GAME_ENGINE_GPU_PROFILER_VK_H
#define GAME_ENGINE_GPU_PROFILER_VK_H

#ifdef GAME_ENGINE_USE_GPU_PROFILING

#include "profiler/backends/config.h"
#include "profiler/backends/gpu_profiler.h"

#ifdef GAME_ENGINE_USE_VULKAN

#include "gfx/rhi/backends/vulkan/command_buffer_vk.h"

#ifdef GAME_ENGINE_TRACY_GPU_PROFILING_VK
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

  bool initialize(gfx::rhi::Device* device) override;
  void destroy() override;
  void setContextName(const std::string& name) override;

  void newFrame() override {}  // Not used in Vulkan
  void collect(gfx::rhi::CommandBuffer* commandBuffer) override;

  void beginZone(gfx::rhi::CommandBuffer* cmdBuf, const std::string& name, uint32_t color = 0) override;
  void endZone(gfx::rhi::CommandBuffer* cmdBuffer) override;

  void insertMarker(gfx::rhi::CommandBuffer* cmdBuffer, const std::string& name, uint32_t color = 0) override;

  void* getContext() const override { return m_tracyContext; }

  private:
  TracyVkContextType                       m_tracyContext = nullptr;
  bool                                     m_initialized  = false;
  std::unique_ptr<gfx::rhi::CommandBuffer> m_cmdBuffer;
};

}  // namespace gpu
}  // namespace game_engine

#endif  // GAME_ENGINE_USE_VULKAN
#endif  // GAME_ENGINE_USE_GPU_PROFILING
#endif  // GAME_ENGINE_GPU_PROFILER_VK_H