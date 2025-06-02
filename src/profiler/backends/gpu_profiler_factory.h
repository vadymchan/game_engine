#ifndef ARISE_GPU_PROFILER_FACTORY_H
#define ARISE_GPU_PROFILER_FACTORY_H

#ifdef ARISE_USE_GPU_PROFILING

#include "gfx/rhi/common/rhi_enums.h"
#include "profiler/backends/gpu_profiler.h"

#include <memory>

namespace arise {
namespace gpu {

class GpuProfilerFactory {
  public:
  static std::unique_ptr<GpuProfiler> create(gfx::rhi::RenderingApi api);
};

}  // namespace gpu
}  // namespace arise

#endif  // ARISE_USE_GPU_PROFILING

#endif  // ARISE_GPU_PROFILER_FACTORY_H
