#include "gfx/rhi/shader_binding_layout.h"
#include "gfx/rhi/rhi.h"

namespace game_engine {

uint32_t test::GetCurrentFrameNumber() {
  return g_rhi->GetCurrentFrameNumber();
}


}  // namespace game_engine