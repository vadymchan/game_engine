#ifndef GAME_ENGINE_GPU_PROFILER_DX12_H
#define GAME_ENGINE_GPU_PROFILER_DX12_H

#include "profiler/backends/config.h"
#include "profiler/backends/gpu_profiler.h"

#ifdef GAME_ENGINE_USE_DX12

#ifdef GAME_ENGINE_TRACY_GPU_PROFILING_DX12
#include <tracy/TracyD3D12.hpp>
using TracyD3D12ContextType = TracyD3D12Ctx;
#else
using TracyD3D12ContextType = void*;
#endif

namespace game_engine {
namespace gpu {

class GpuProfilerDx12 final : public GpuProfiler {
  public:
  GpuProfilerDx12() = default;
  ~GpuProfilerDx12() override { destroy(); }

  bool initialize(gfx::rhi::Device* device) override;
  void destroy() override;
  void setContextName(const std::string& name) override;

  void newFrame() override;
  void collect(gfx::rhi::CommandBuffer* commandBuffer) override;

  void beginZone(gfx::rhi::CommandBuffer* cmdBuf, const std::string& name, uint32_t color = 0) override;
  void endZone(gfx::rhi::CommandBuffer* cmdBuffer) override;

  void insertMarker(gfx::rhi::CommandBuffer* cmdBuffer, const std::string& name, uint32_t color = 0) override;

  void* getContext() const override { return m_tracyContext; }

  private:
  TracyD3D12ContextType m_tracyContext = nullptr;
  bool                  m_initialized  = false;
};

}  // namespace gpu
}  // namespace game_engine

#endif  // GAME_ENGINE_USE_DX12
#endif  // GAME_ENGINE_GPU_PROFILER_DX12_H