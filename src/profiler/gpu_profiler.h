#ifndef GAME_ENGINE_GPU_PROFILER_H
#define GAME_ENGINE_GPU_PROFILER_H

#include "profiler/profiler_colors.h"

#include <cstdint>

namespace game_engine {
namespace profiler {
namespace gpu {

class Marker {
  public:
  Marker(void* cmdBuffer, const char* name, const Color& color = colors::RENDERING);

  Marker(void* cmdBuffer, const char* name, float r, float g, float b, float a = 1.0f);

  ~Marker();

  Marker(const Marker&)            = delete;
  Marker& operator=(const Marker&) = delete;

  private:
  void* m_cmdBuffer;
};

void beginMarker(void* cmdBuffer, const char* name, const Color& color);
void endMarker(void* cmdBuffer);
void insertMarker(void* cmdBuffer, const char* name, const Color& color);

}  // namespace gpu
}  // namespace profiler
}  // namespace game_engine

#endif  // GAME_ENGINE_GPU_PROFILER_H