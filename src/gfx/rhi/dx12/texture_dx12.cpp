#include "gfx/rhi/dx12/texture_dx12.h"


namespace game_engine {

TextureDx12::~TextureDx12() {
  release();
}

void TextureDx12::release() {
  m_srv_.free();
  m_uav_.free();
  m_rtv_.free();
  m_dsv_.free();
}


}