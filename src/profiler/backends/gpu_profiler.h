#ifndef GAME_ENGINE_GPU_PROFILER_H
#define GAME_ENGINE_GPU_PROFILER_H

#include "gfx/rhi/interface/command_buffer.h"
#include "utils/color/color.h"

#include <string>

namespace game_engine {
namespace gpu {

class GpuProfilerVk;
class GpuProfilerDx12;

class GpuProfiler {
  public:
  virtual ~GpuProfiler() = default;

  virtual bool initialize(void* physicalDevice, void* device, void* queue, void* commandBuffer) = 0;
  virtual void destroy()                                                                        = 0;
  virtual void setContextName(const std::string& name)                                          = 0;

  virtual void newFrame()                   = 0;
  virtual void collect(void* commandBuffer) = 0;

  template <uint32_t Color, size_t N>
  void beginZoneNC(gfx::rhi::CommandBuffer* cmdBuf, const char (&name)[N]);

  template <uint32_t Color>
  void beginZoneC(gfx::rhi::CommandBuffer* cmdBuf);

  template <size_t N>
  void beginZoneN(gfx::rhi::CommandBuffer* cmdBuf, const char (&name)[N]);

  virtual void endZone(gfx::rhi::CommandBuffer* cmdBuffer) = 0;

  template <uint32_t Color, size_t N>
  void insertMarker(gfx::rhi::CommandBuffer* cmdBuffer, const char (&name)[N]) {
    if (!cmdBuffer) {
      return;
    }

    auto colorArray = color::g_toFloatArray(Color);
    cmdBuffer->insertDebugMarker(name, colorArray.data());
  }

  template <size_t N>
  void insertMarkerN(gfx::rhi::CommandBuffer* cmdBuffer, const char (&name)[N]) {
    if (!cmdBuffer) {
      return;
    }

    cmdBuffer->insertDebugMarker(name, nullptr);
  }

  void setApi(gfx::rhi::RenderingApi api) noexcept { m_api = api; }

  protected:
  gfx::rhi::RenderingApi m_api = gfx::rhi::RenderingApi::Count;
};

}  // namespace gpu
}  // namespace game_engine

#endif  // GAME_ENGINE_GPU_PROFILER_H