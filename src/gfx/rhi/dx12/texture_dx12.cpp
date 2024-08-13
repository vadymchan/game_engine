#include "gfx/rhi/dx12/texture_dx12.h"


namespace game_engine {

TextureDx12::~TextureDx12() {
  Release();
}

void TextureDx12::Release() {
  SRV.Free();
  UAV.Free();
  RTV.Free();
  DSV.Free();
}


}