#include "gfx/rhi/dx12/texture_dx12.h"


namespace game_engine {

TextureDx12::~TextureDx12() {
  Release();
}

void TextureDx12::Release() {
  m_srv_.Free();
  m_uav_.Free();
  m_rtv_.Free();
  m_dsv_.Free();
}


}