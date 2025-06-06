#ifndef ARISE_GPU_PROFILER_VK_H
#define ARISE_GPU_PROFILER_VK_H

#ifdef ARISE_USE_GPU_PROFILING

#include "profiler/backends/config.h"
#include "profiler/backends/gpu_profiler.h"

#ifdef ARISE_USE_VULKAN

#include "gfx/rhi/backends/vulkan/command_buffer_vk.h"

#ifdef ARISE_TRACY_GPU_PROFILING_VK
#include <vulkan/vulkan.h>

#include <tracy/TracyVulkan.hpp>
using TracyVkContextType = TracyVkCtx;
#else
using TracyVkContextType = void*;
#endif
namespace arise {
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
}  // namespace arise

#endif  // ARISE_USE_VULKAN
#endif  // ARISE_USE_GPU_PROFILING
#endif  // ARISE_GPU_PROFILER_VK_H