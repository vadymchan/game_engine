#ifndef ARISE_GPU_PROFILER_H
#define ARISE_GPU_PROFILER_H

#include "gfx/rhi/common/rhi_enums.h"
#include "utils/color/color.h"

#include <string>

namespace arise::gfx::rhi {
class CommandBuffer;
class Device;
}  // namespace arise::gfx::rhi

namespace arise {
namespace gpu {

class GpuProfiler {
  public:
  virtual ~GpuProfiler() = default;

  virtual bool initialize(gfx::rhi::Device* device)    = 0;
  virtual void destroy()                               = 0;
  virtual void setContextName(const std::string& name) = 0;

  virtual void newFrame()                                      = 0;
  virtual void collect(gfx::rhi::CommandBuffer* commandBuffer) = 0;

  virtual void beginZone(gfx::rhi::CommandBuffer* cmdBuf, const std::string& name, uint32_t color = 0) = 0;
  virtual void endZone(gfx::rhi::CommandBuffer* cmdBuffer)                                             = 0;

  virtual void insertMarker(gfx::rhi::CommandBuffer* cmdBuffer, const std::string& name, uint32_t color = 0) = 0;

  virtual void* getContext() const = 0;

  gfx::rhi::RenderingApi getApi() const noexcept { return m_api; }
  void                   setApi(gfx::rhi::RenderingApi api) noexcept { m_api = api; }

  protected:
  gfx::rhi::RenderingApi m_api = gfx::rhi::RenderingApi::Count;
};

}  // namespace gpu
}  // namespace arise

#endif  // ARISE_GPU_PROFILER_H