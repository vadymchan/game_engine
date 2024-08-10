#include "gfx/rhi/dx12/texture_dx12.h"


namespace game_engine {

jTexture_DX12::~jTexture_DX12() {
  Release();
}

void jTexture_DX12::Release() {
  SRV.Free();
  UAV.Free();
  RTV.Free();
  DSV.Free();
}


}