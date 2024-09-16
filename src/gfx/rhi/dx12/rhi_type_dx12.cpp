
#include "gfx/rhi/dx12/rhi_type_dx12.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "gfx/rhi/dx12/rhi_dx12.h"


namespace game_engine {
void CreatedResource::free() {
  if (m_resource_) {
    if (m_resourceType_ == CreatedResource::EType::Standalone) {
      if (g_rhiDx12) {
        g_rhiDx12->m_deallocatorMultiFrameStandaloneResource_.free(m_resource_);
      }
    } else if (m_resourceType_ == CreatedResource::EType::ResourcePool) {
      if (g_rhiDx12) {
        g_rhiDx12->m_deallocatorMultiFramePlacedResource_.free(m_resource_);
      }
    } else if (m_resourceType_ == CreatedResource::EType::Swapchain) {
      // Nothing to do
    } else {
      assert(0);
    }
  }
}

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12