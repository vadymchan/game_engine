#include "gfx/rhi/shader_binding_layout.h"
#include "gfx/rhi/rhi.h"

namespace game_engine {

uint32_t test::g_getCurrentFrameNumber() {
  return g_rhi->getCurrentFrameNumber();
}


}  // namespace game_engine