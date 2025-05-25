#include "profiler/backends/gpu_profiler_dx12.h"

#ifdef GAME_ENGINE_USE_DX12

#include "gfx/rhi/backends/dx12/command_buffer_dx12.h"
#include "utils/logger/global_logger.h"

namespace game_engine {
namespace gpu {

bool GpuProfilerDx12::initialize(void* physicalDevice, void* device, void* queue, void* commandBuffer) {
  if (m_initialized) {
    GlobalLogger::Log(LogLevel::Warning, "GpuProfilerDx12 already initialized");
    return true;
  }

  if (!device || !queue) {
    GlobalLogger::Log(LogLevel::Error, "Invalid DirectX 12 parameters for profiler");
    return false;
  }

#ifdef GAME_ENGINE_USE_GPU_PROFILING
  m_tracyContext = TracyD3D12Context(static_cast<ID3D12Device*>(device), static_cast<ID3D12CommandQueue*>(queue));

  if (!m_tracyContext) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create Tracy DirectX 12 context");
    return false;
  }

  GlobalLogger::Log(LogLevel::Info, "Tracy DirectX 12 profiler initialized");
#else
  GlobalLogger::Log(LogLevel::Info, "Tracy profiling disabled, using native GPU markers only");
#endif

  setApi(gfx::rhi::RenderingApi::Dx12);
  m_initialized = true;
  return true;
}

void GpuProfilerDx12::destroy() {
  if (!m_initialized) {
    return;
  }

#ifdef GAME_ENGINE_USE_GPU_PROFILING
  if (m_tracyContext) {
    TracyD3D12Destroy(m_tracyContext);
    m_tracyContext = nullptr;
    GlobalLogger::Log(LogLevel::Info, "Tracy DirectX 12 profiler destroyed");
  }
#endif

  m_initialized = false;
}

void GpuProfilerDx12::setContextName(const std::string& name) {
  if (!m_initialized) {
    return;
  }

#ifdef GAME_ENGINE_USE_GPU_PROFILING
  if (m_tracyContext) {
    TracyD3D12ContextName(m_tracyContext, name.c_str(), name.size());
  }
#endif
}

void GpuProfilerDx12::newFrame() {
  if (!m_initialized) {
    return;
  }

#ifdef GAME_ENGINE_USE_GPU_PROFILING
  if (m_tracyContext) {
    TracyD3D12NewFrame(m_tracyContext);
  }
#endif
}

void GpuProfilerDx12::collect(void* commandBuffer) {
  if (!m_initialized) {
    return;
  }

#ifdef GAME_ENGINE_USE_GPU_PROFILING
  if (m_tracyContext) {
    TracyD3D12Collect(m_tracyContext);
  }
#endif
}

void GpuProfilerDx12::endZone(gfx::rhi::CommandBuffer* cmdBuffer) {
  if (!cmdBuffer) {
    return;
  }
  cmdBuffer->endDebugMarker();
}

}  // namespace gpu
}  // namespace game_engine

#endif  // GAME_ENGINE_USE_DX12