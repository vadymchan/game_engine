#ifndef GAME_ENGINE_TEXTURE_DX12_H
#define GAME_ENGINE_TEXTURE_DX12_H

#include "gfx/rhi/dx12/descriptor_heap_dx12.h"
#include "gfx/rhi/dx12/rhi_type_dx12.h"
#include "gfx/rhi/texture.h"

#include <map>

namespace game_engine {

// TODO: constructors
struct TextureDx12 : public Texture {
  TextureDx12() = default;

  TextureDx12(ETextureType                             InType,
                ETextureFormat                           InFormat,
                const math::Dimension2Di&                extent,
                int32_t                                  InLayerCount,
                EMSAASamples                             InSampleCount,
                bool                                     InSRGB,
                const RTClearValue&                     InClearValue,
                const std::shared_ptr<CreatedResource>& InImage,
                DescriptorDx12                         InRTV = {},
                DescriptorDx12                         InDSV = {},
                DescriptorDx12                         InSRV = {},
                DescriptorDx12                         InUAV = {},
                EResourceLayout InImageLayout = EResourceLayout::UNDEFINED)
      : Texture(InType,
                 InFormat,
                 extent,
                 InLayerCount,
                 InSampleCount,
                 InSRGB)
      , m_texture(InImage)
      , RTV(InRTV)
      , DSV(InDSV)
      , SRV(InSRV)
      , UAV(InUAV)
      , Layout(InImageLayout) {}

  virtual ~TextureDx12();

  std::shared_ptr<CreatedResource> m_texture;
  EResourceLayout                   Layout = EResourceLayout::UNDEFINED;
  DescriptorDx12                  SRV;
  DescriptorDx12                  UAV;
  DescriptorDx12                  RTV;
  DescriptorDx12                  DSV;

  std::map<int32_t, DescriptorDx12> UAVMipMap;

  virtual void* GetHandle() const override { return m_texture->Get(); }

  virtual void* GetSamplerStateHandle() const override { return nullptr; }

  virtual void Release() override;

  virtual EResourceLayout GetLayout() const override { return Layout; }
};

}  // namespace game_engine

#endif  // GAME_ENGINE_TEXTURE_DX12_H