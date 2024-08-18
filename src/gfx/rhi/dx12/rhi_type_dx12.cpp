
#include "gfx/rhi/dx12/rhi_type_dx12.h"
#include "gfx/rhi/dx12/rhi_dx12.h"


namespace game_engine {
void CreatedResource::Free() {
  if (m_resource_) {
    if (m_resourceType_ == CreatedResource::EType::Standalone) {
      if (g_rhi_dx12) {
        g_rhi_dx12->m_deallocatorMultiFrameStandaloneResource_.Free(m_resource_);
      }
    } else if (m_resourceType_ == CreatedResource::EType::ResourcePool) {
      if (g_rhi_dx12) {
        g_rhi_dx12->m_deallocatorMultiFramePlacedResource_.Free(m_resource_);
      }
    } else if (m_resourceType_ == CreatedResource::EType::Swapchain) {
      // Nothing to do
    } else {
      assert(0);
    }
  }
}

}  // namespace game_engine