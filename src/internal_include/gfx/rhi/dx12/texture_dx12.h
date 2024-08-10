#ifndef GAME_ENGINE_TEXTURE_DX12_H
#define GAME_ENGINE_TEXTURE_DX12_H

#include "gfx/rhi/dx12/descriptor_heap_dx12.h"
#include "gfx/rhi/dx12/rhi_type_dx12.h"
#include "gfx/rhi/texture.h"

#include <map>

namespace game_engine {

// TODO: constructors
struct jTexture_DX12 : public jTexture {
  jTexture_DX12() = default;

  jTexture_DX12(ETextureType                             InType,
                ETextureFormat                           InFormat,
                const math::Dimension2Di&                extent,
                int32_t                                  InLayerCount,
                EMSAASamples                             InSampleCount,
                bool                                     InSRGB,
                const jRTClearValue&                     InClearValue,
                const std::shared_ptr<jCreatedResource>& InImage,
                jDescriptor_DX12                         InRTV = {},
                jDescriptor_DX12                         InDSV = {},
                jDescriptor_DX12                         InSRV = {},
                jDescriptor_DX12                         InUAV = {},
                EResourceLayout InImageLayout = EResourceLayout::UNDEFINED)
      : jTexture(InType,
                 InFormat,
                 extent,
                 InLayerCount,
                 InSampleCount,
                 InSRGB)
      , Texture(InImage)
      , RTV(InRTV)
      , DSV(InDSV)
      , SRV(InSRV)
      , UAV(InUAV)
      , Layout(InImageLayout) {}

  virtual ~jTexture_DX12();

  std::shared_ptr<jCreatedResource> Texture;
  EResourceLayout                   Layout = EResourceLayout::UNDEFINED;
  jDescriptor_DX12                  SRV;
  jDescriptor_DX12                  UAV;
  jDescriptor_DX12                  RTV;
  jDescriptor_DX12                  DSV;

  std::map<int32_t, jDescriptor_DX12> UAVMipMap;

  virtual void* GetHandle() const override { return Texture->Get(); }

  virtual void* GetSamplerStateHandle() const override { return nullptr; }

  virtual void Release() override;

  virtual EResourceLayout GetLayout() const override { return Layout; }
};

}  // namespace game_engine

#endif  // GAME_ENGINE_TEXTURE_DX12_H