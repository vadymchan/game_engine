#include "profiler/backends/gpu_profiler_factory.h"

#ifdef GAME_ENGINE_USE_GPU_PROFILING

#include "profiler/backends/gpu_profiler_dx12.h"
#include "profiler/backends/gpu_profiler_vk.h"
#include "utils/logger/global_logger.h"

namespace game_engine {
namespace gpu {

std::unique_ptr<GpuProfiler> GpuProfilerFactory::create(gfx::rhi::RenderingApi api) {
#ifndef GAME_ENGINE_USE_GPU_PROFILING
  return nullptr;
#endif

  std::unique_ptr<GpuProfiler> profiler;
  switch (api) {
#ifdef GAME_ENGINE_USE_VULKAN
    case gfx::rhi::RenderingApi::Vulkan:
      profiler = std::make_unique<GpuProfilerVk>();
      break;
#endif

#ifdef GAME_ENGINE_USE_DX12
    case gfx::rhi::RenderingApi::Dx12:
      profiler = std::make_unique<GpuProfilerDx12>();
      break;
#endif

    default:
      GlobalLogger::Log(LogLevel::Error, "Unsupported rendering API for GPU profiler");
      return nullptr;
  }
  profiler->setApi(api);
  return profiler;
}


}  // namespace gpu
}  // namespace game_engine


#endif  // GAME_ENGINE_USE_GPU_PROFILING
