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

  TextureDx12(ETextureType                            type,
              ETextureFormat                          format,
              const math::Dimension2Di&               extent,
              int32_t                                 InLayerCount,
              EMSAASamples                            InSampleCount,
              bool                                    InSRGB,
              const RTClearValue&                     InClearValue,
              const std::shared_ptr<CreatedResource>& InImage,
              DescriptorDx12                          InRTV = {},
              DescriptorDx12                          InDSV = {},
              DescriptorDx12                          InSRV = {},
              DescriptorDx12                          InUAV = {},
              EResourceLayout InImageLayout = EResourceLayout::UNDEFINED)
      : Texture(type, format, extent, InLayerCount, InSampleCount, InSRGB)
      , m_texture(InImage)
      , m_rtv_(InRTV)
      , m_dsv_(InDSV)
      , m_srv_(InSRV)
      , m_uav_(InUAV)
      , m_layout_(InImageLayout) {}

  virtual ~TextureDx12();

  std::shared_ptr<CreatedResource>  m_texture;
  EResourceLayout                   m_layout_ = EResourceLayout::UNDEFINED;
  // TODO: consider if naming conventions is correct for srv, uav, rtv, dsv
  DescriptorDx12                    m_srv_;
  DescriptorDx12                    m_uav_;
  DescriptorDx12                    m_rtv_;
  DescriptorDx12                    m_dsv_;
  std::map<int32_t, DescriptorDx12> m_uavMipMap;

  virtual void* GetHandle() const override { return m_texture->Get(); }

  virtual void* GetSamplerStateHandle() const override { return nullptr; }

  virtual void Release() override;

  virtual EResourceLayout GetLayout() const override { return m_layout_; }
};

}  // namespace game_engine

#endif  // GAME_ENGINE_TEXTURE_DX12_H