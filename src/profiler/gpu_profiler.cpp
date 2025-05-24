#include "profiler/gpu_profiler.h"

#include "gfx/rhi/interface/command_buffer.h"
#include "utils/logger/global_logger.h"

namespace game_engine {
namespace profiler {
namespace gpu {

Marker::Marker(void* cmdBuffer, const char* name, const Color& color)
    : m_cmdBuffer(cmdBuffer) {
  beginMarker(m_cmdBuffer, name, color);
}

Marker::Marker(void* cmdBuffer, const char* name, float r, float g, float b, float a)
    : Marker(cmdBuffer, name, Color(r, g, b, a)) {
}

Marker::~Marker() {
  endMarker(m_cmdBuffer);
}

void beginMarker(void* cmdBuffer, const char* name, const Color& color) {
  auto* cmd = static_cast<gfx::rhi::CommandBuffer*>(cmdBuffer);
  if (!cmd) {
    return;
  }

  float colorArray[4] = {color.r, color.g, color.b, color.a};
  cmd->beginDebugMarker(name, colorArray);

#ifdef GAME_ENGINE_USE_TRACY
  // TODO: 
  // TracyVkZone(ctx, cmdbuf, name);
  // TracyD3D12Zone(ctx, cmdList, name);
#endif
}

void endMarker(void* cmdBuffer) {
  auto* cmd = static_cast<gfx::rhi::CommandBuffer*>(cmdBuffer);
  if (!cmd) {
    return;
  }

  cmd->endDebugMarker();
}

void insertMarker(void* cmdBuffer, const char* name, const Color& color) {
  auto* cmd = static_cast<gfx::rhi::CommandBuffer*>(cmdBuffer);
  if (!cmd) {
    return;
  }

  float colorArray[4] = {color.r, color.g, color.b, color.a};
  cmd->insertDebugMarker(name, colorArray);
}

}  // namespace gpu
}  // namespace profiler
}  // namespace game_engine