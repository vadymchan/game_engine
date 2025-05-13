#ifndef GAME_ENGINE_SHADER_H
#define GAME_ENGINE_SHADER_H

#include "gfx/rhi/common/rhi_enums.h"
#include "gfx/rhi/common/rhi_types.h"

#include <string>
#include <vector>

namespace game_engine {
namespace gfx {
namespace rhi {

/**
 * Represents a compiled shader module that can be used in a pipeline.
 */
class Shader {
  public:
  Shader(const ShaderDesc& desc)
      : m_desc_(desc) {}
  virtual ~Shader() = default;

  ShaderStageFlag getStage() const { return m_desc_.stage; }

  const std::string& getEntryPoint() const { return m_desc_.entryPoint; }

  const ShaderDesc& getDesc() const { return m_desc_; }

  virtual void initialize(const std::vector<uint8_t>& code) = 0;
  virtual void release()                                    = 0;

  virtual void reinitialize(const std::vector<uint8_t>& code) {
    release();
    initialize(code);
  }

  protected:
  ShaderDesc m_desc_;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_H