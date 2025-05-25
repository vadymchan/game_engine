#ifndef GAME_ENGINE_GPU_PROFILER_DX12_H
#define GAME_ENGINE_GPU_PROFILER_DX12_H

#include "profiler/backends/gpu_profiler.h"

#ifdef GAME_ENGINE_USE_DX12

#include "gfx/rhi/backends/dx12/command_buffer_dx12.h"

#ifdef GAME_ENGINE_USE_GPU_PROFILING
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

  bool initialize(void* physicalDevice, void* device, void* queue, void* commandBuffer) override;
  void destroy() override;
  void setContextName(const std::string& name) override;

  void newFrame() override;
  void collect(void* commandBuffer) override;

  // Named and colored zone
  template <uint32_t Color, size_t N>
  void beginZoneNC(gfx::rhi::CommandBuffer* cb, const char (&name)[N]) {
#if defined(TRACY_ENABLE)
    auto* dx = static_cast<gfx::rhi::CommandBufferDx12*>(cb);
    TracyD3D12NamedZoneC(m_tracyContext, _zone, dx->getCommandList(), name, Color, true);
#endif
  }

  // Color-only zone
  template <uint32_t Color>
  void beginZoneC(gfx::rhi::CommandBuffer* cb) {
#if defined(TRACY_ENABLE)
    auto* dx = static_cast<gfx::rhi::CommandBufferDx12*>(cb);
    TracyD3D12ZoneC(m_tracyContext, dx->getCommandList(), Color, true);
#endif
  }

  // Name-only zone
  template <size_t N>
  void beginZoneN(gfx::rhi::CommandBuffer* cb, const char (&name)[N]) {
#if defined(TRACY_ENABLE)
    auto* dx = static_cast<gfx::rhi::CommandBufferDx12*>(cb);
    TracyD3D12NamedZone(m_tracyContext, _zone, dx->getCommandList(), name, true);
#endif
  }

  void endZone(gfx::rhi::CommandBuffer* cmdBuffer) override;

  private:
  TracyD3D12ContextType m_tracyContext = nullptr;
  bool                  m_initialized  = false;
};

}  // namespace gpu
}  // namespace game_engine

#include "profiler/backends/gpu_profiler.inl"

#endif  // GAME_ENGINE_USE_DX12
#endif  // GAME_ENGINE_GPU_PROFILER_DX12_H