#ifndef GAME_ENGINE_SAMPLER_H
#define GAME_ENGINE_SAMPLER_H

#include "gfx/rhi/rhi_new/common/rhi_enums.h"
#include "gfx/rhi/rhi_new/common/rhi_types.h"

namespace game_engine {
namespace gfx {
namespace rhi {

/**
 * A sampler defines how texture coordinates are mapped to texels
 * and how texels are filtered when sampling a texture.
 */
class Sampler {
  public:
  Sampler(const SamplerDesc& desc)
      : m_desc_(desc) {}

  virtual ~Sampler() = default;

  const SamplerDesc& getDesc() const { return m_desc_; }

  protected:
  SamplerDesc m_desc_;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_SAMPLER_H