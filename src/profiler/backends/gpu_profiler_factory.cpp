#include "profiler/backends/gpu_profiler_factory.h"

#include "profiler/backends/gpu_profiler_dx12.h"
#include "profiler/backends/gpu_profiler_vk.h"
#include "utils/logger/global_logger.h"

namespace game_engine {
namespace gpu {

std::unique_ptr<GpuProfiler> GpuProfilerFactory::create(gfx::rhi::RenderingApi api) {
  switch (api) {
#ifdef GAME_ENGINE_USE_VULKAN
    case gfx::rhi::RenderingApi::Vulkan:
      return std::make_unique<GpuProfilerVk>();
#endif

#ifdef GAME_ENGINE_USE_DX12
    case gfx::rhi::RenderingApi::Dx12:
      return std::make_unique<GpuProfilerDx12>();
#endif

    default:
      GlobalLogger::Log(LogLevel::Error, "Unsupported rendering API for GPU profiler");
      return nullptr;
  }
}

}  // namespace gpu
}  // namespace game_engine