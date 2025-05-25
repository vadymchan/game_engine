#ifndef GAME_ENGINE_GPU_PROFILER_FACTORY_H
#define GAME_ENGINE_GPU_PROFILER_FACTORY_H

#include "gfx/rhi/common/rhi_enums.h"
#include "profiler/backends/gpu_profiler.h"

#include <memory>

namespace game_engine {
namespace gpu {

class GpuProfilerFactory {
  public:
  static std::unique_ptr<GpuProfiler> create(gfx::rhi::RenderingApi api);
};

}  // namespace gpu
}  // namespace game_engine

#endif  // GAME_ENGINE_GPU_PROFILER_FACTORY_H
