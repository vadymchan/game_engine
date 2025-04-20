#ifndef GAME_ENGINE_RHI_SWAP_CHAIN_H
#define GAME_ENGINE_RHI_SWAP_CHAIN_H

#include "gfx/rhi/common/rhi_enums.h"
#include "gfx/rhi/common/rhi_types.h"

namespace game_engine {
namespace gfx {
namespace rhi {

/**
 * SwapChain interface
 *
 * The SwapChain represents the chain of backbuffers used for presenting
 * rendered images to the screen. It handles image acquisition, presentation,
 * and synchronization with the display.
 */
class SwapChain {
  public:
  SwapChain(const SwapchainDesc& desc)
      : m_desc_(desc) {}
  virtual ~SwapChain() = default;

  virtual Texture* getCurrentImage() = 0;

  virtual uint32_t getCurrentImageIndex() const = 0;

  virtual bool acquireNextImage() = 0;

  virtual bool present() = 0;

  virtual bool resize(uint32_t width, uint32_t height) = 0;

  virtual TextureFormat getFormat() const = 0;

  virtual uint32_t getWidth() const = 0;

  virtual uint32_t getHeight() const = 0;

  virtual uint32_t getBufferCount() const = 0;

  const SwapchainDesc& getDesc() const { return m_desc_; }

  protected:
  SwapchainDesc m_desc_;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_SWAP_CHAIN_H