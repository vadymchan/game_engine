#ifndef GAME_ENGINE_TEXTURE_DX12_H
#define GAME_ENGINE_TEXTURE_DX12_H

#include "platform/windows/windows_platform_setup.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "gfx/rhi/dx12/descriptor_heap_dx12.h"
#include "gfx/rhi/dx12/rhi_type_dx12.h"
#include "gfx/rhi/texture.h"

#include <map>

namespace game_engine {

// TODO: constructors
struct TextureDx12 : public Texture {
  // ======= BEGIN: public constructors =======================================

  TextureDx12() = default;

  TextureDx12(ETextureType                                type,
              ETextureFormat                              format,
              const math::Dimension2Di&                   extent,
              int32_t                                     layerCount,
              EMSAASamples                                sampleCount,
              bool                                        sRGB,
              const RtClearValue&                         clearValue,
              const std::shared_ptr<CreatedResourceDx12>& image,
              DescriptorDx12                              rtv = {},
              DescriptorDx12                              dsv = {},
              DescriptorDx12                              srv = {},
              DescriptorDx12                              uav = {},
              EResourceLayout imageLayout = EResourceLayout::UNDEFINED)
      : Texture(type, format, extent, layerCount, sampleCount, sRGB)
      , m_texture(image)
      , m_rtv_(rtv)
      , m_dsv_(dsv)
      , m_srv_(srv)
      , m_uav_(uav)
      , m_layout_(imageLayout) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~TextureDx12();

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void release() override;

  virtual void* getHandle() const override { return m_texture->get(); }

  virtual void* getSamplerStateHandle() const override { return nullptr; }

  virtual EResourceLayout getLayout() const override { return m_layout_; }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  std::shared_ptr<CreatedResourceDx12> m_texture;
  EResourceLayout                      m_layout_ = EResourceLayout::UNDEFINED;
  // TODO: consider if naming conventions is correct for srv, uav, rtv, dsv
  DescriptorDx12                       m_srv_;
  DescriptorDx12                       m_uav_;
  DescriptorDx12                       m_rtv_;
  DescriptorDx12                       m_dsv_;
  std::map<int32_t, DescriptorDx12>    m_uavMipMap;

  // ======= END: public misc fields   ========================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12

#endif  // GAME_ENGINE_TEXTURE_DX12_H
