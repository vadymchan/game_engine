#ifndef ARISE_SAMPLER_H
#define ARISE_SAMPLER_H

#include "gfx/rhi/common/rhi_enums.h"
#include "gfx/rhi/common/rhi_types.h"

namespace arise {
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
}  // namespace arise

#endif  // ARISE_SAMPLER_H