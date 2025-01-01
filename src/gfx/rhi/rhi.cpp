#include "gfx/rhi/rhi.h"

namespace game_engine {

RHI* g_rhi = nullptr;

int32_t g_maxCheckCountForRealTimeShaderUpdate = 10;
int32_t g_sleepMSForRealTimeShaderUpdate       = 100;
bool    g_useRealTimeShaderUpdate              = true;

TResourcePool<Shader, MutexRWLock> RHI::s_shaderPool;

const RtClearValue RtClearValue::s_kInvalid = RtClearValue();

bool RHI::init(const std::shared_ptr<Window>& window) {
  return false;
}

void RHI::onInitRHI() {
}

void RHI::release() {

  Shader::s_releaseCheckUpdateShaderThread();
  s_shaderPool.release();
}

RHI::RHI() {
}

}  // namespace game_engine
