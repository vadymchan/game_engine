#include "gfx/rhi/dx12/swapchain_dx12.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "gfx/rhi/dx12/rhi_dx12.h"
#include "gfx/rhi/dx12/utils_dx12.h"

namespace game_engine {

//////////////////////////////////////////////////////////////////////////
// SwapchainImageDx12
//////////////////////////////////////////////////////////////////////////
void SwapchainImageDx12::release() {
  releaseInternal();
}

void SwapchainImageDx12::releaseInternal() {
  m_TexturePtr_ = nullptr;
  // if (Available)
  //{
  //     g_rhi->getSemaphoreManager()->returnSemaphore(Available);
  //     Available = nullptr;
  // }
  // if (RenderFinished)
  //{
  //     g_rhi->getSemaphoreManager()->returnSemaphore(RenderFinished);
  //     RenderFinished = nullptr;
  // }
  // if (RenderFinishedAfterShadow)
  //{
  //     g_rhi->getSemaphoreManager()->returnSemaphore(RenderFinishedAfterShadow);
  //     RenderFinishedAfterShadow = nullptr;
  // }
  // if (RenderFinishedAfterBasePass)
  //{
  //     g_rhi->getSemaphoreManager()->returnSemaphore(RenderFinishedAfterBasePass);
  //     RenderFinishedAfterBasePass = nullptr;
  // }
}

//////////////////////////////////////////////////////////////////////////
// SwapchainDx12
//////////////////////////////////////////////////////////////////////////
bool SwapchainDx12::create(const std::shared_ptr<Window>& window) {
  DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
  swapChainDesc.BufferCount      = RhiDx12::s_kMaxFrameCount;
  swapChainDesc.Width            = window->getSize().width();
  swapChainDesc.Height           = window->getSize().height();
  swapChainDesc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapChainDesc.BufferUsage      = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.SwapEffect       = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swapChainDesc.SampleDesc.Count = 1;
  swapChainDesc.Flags            = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
                      | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
                      | DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

  assert(g_rhiDx12);
  assert(!!g_rhiDx12->m_hWnd);
  assert(g_rhiDx12->m_factory_);
  assert(g_rhiDx12->m_commandBufferManager_);

  ComPtr<IDXGISwapChain1> swapChainTemp;
  HRESULT                 hr = g_rhiDx12->m_factory_->CreateSwapChainForHwnd(
      g_rhiDx12->m_commandBufferManager_->getCommandQueue().Get(),
      g_rhiDx12->m_hWnd,
      &swapChainDesc,
      nullptr,
      nullptr,
      &swapChainTemp);

  assert(SUCCEEDED(hr));
  if (FAILED(hr)) {
    return false;
  }

  hr = swapChainTemp->QueryInterface(IID_PPV_ARGS(&m_swapChain_));
  assert(SUCCEEDED(hr));
  if (FAILED(hr)) {
    return false;
  }


  m_extent_ = window->getSize();

  m_images_.resize(RhiDx12::s_kMaxFrameCount);
  for (int32_t i = 0; i < m_images_.size(); ++i) {
    SwapchainImageDx12* swapchainImage = new SwapchainImageDx12();

    ComPtr<ID3D12Resource> NewResource;
    HRESULT hr = m_swapChain_->GetBuffer(i, IID_PPV_ARGS(&NewResource));
    assert(SUCCEEDED(hr));

    if (FAILED(hr)) {
      return false;
    }


    std::shared_ptr<CreatedResourceDx12> RenderTargetResource
        = CreatedResourceDx12::s_createdFromSwapchain(NewResource);

    m_images_[i] = swapchainImage;

    auto TextureDX12Ptr
        = std::make_shared<TextureDx12>(ETextureType::TEXTURE_2D,
                                          m_format_,
                                          m_extent_,
                                          1,
                                          EMSAASamples::COUNT_1,
                                          false,
                                          RtClearValue::s_kInvalid,
                                          RenderTargetResource);
    swapchainImage->m_TexturePtr_ = TextureDX12Ptr;

    g_createRenderTargetView((TextureDx12*)swapchainImage->m_TexturePtr_.get());
    TextureDX12Ptr->m_layout_ = EResourceLayout::PRESENT_SRC;
  }

  return true;
}

void SwapchainDx12::release() {
  releaseInternal();
}

bool SwapchainDx12::resize(int32_t witdh, int32_t height) {
  bool isSwapChainValid = m_swapChain_;
  assert(isSwapChainValid);

  if (isSwapChainValid) {
    for (int32_t i = 0; i < g_rhiDx12->s_kMaxFrameCount; ++i) {
      ISwapchainImage* swapchainImage = m_images_[i];
      auto TexDX12 = (TextureDx12*)swapchainImage->m_TexturePtr_.get();
      TexDX12->m_texture->m_resource_.get()->Reset();
    }

    m_swapChain_->SetFullscreenState(false, nullptr);
    HRESULT hr = m_swapChain_->ResizeBuffers(
        g_rhiDx12->s_kMaxFrameCount,
        witdh,
        height,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
            | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
            | DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);

    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
#ifdef _DEBUG
      char buff[64] = {};
      sprintf_s(buff,
                "Device Lost on ResizeBuffers: Reason code 0x%08X\n",
                (hr == DXGI_ERROR_DEVICE_REMOVED)
                    ? g_rhiDx12->m_device_->GetDeviceRemovedReason()
                    : hr);
      OutputDebugStringA(buff);
#endif
      return false;
    } else {
      // TODO: refactor
      assert(SUCCEEDED(hr));
      if (FAILED(hr)) {
        return false;
      }
    }
  }

  for (int32_t i = 0; i < g_rhiDx12->s_kMaxFrameCount; ++i) {
    ISwapchainImage* swapchainImage = m_images_[i];

    ComPtr<ID3D12Resource> NewResource;
    HRESULT hr = m_swapChain_->GetBuffer(i, IID_PPV_ARGS(&NewResource));
    assert(SUCCEEDED(hr));

    if (FAILED(hr)) {
      return false;
    }


    std::shared_ptr<CreatedResourceDx12> RenderTargetResource
        = CreatedResourceDx12::s_createdFromSwapchain(NewResource);

    auto TextureDX12Ptr = std::make_shared<TextureDx12>(
        ETextureType::TEXTURE_2D,
        g_getDX12TextureFormat(DXGI_FORMAT_R8G8B8A8_UNORM),
        // TODO: remove casting
        math::Dimension2Di{static_cast<int>(witdh),
                           static_cast<int>(height)},
        1,
        EMSAASamples::COUNT_1,
        false,
        RtClearValue::s_kInvalid,
        RenderTargetResource);
    swapchainImage->m_TexturePtr_ = TextureDX12Ptr;

    g_createRenderTargetView((TextureDx12*)swapchainImage->m_TexturePtr_.get());
  }

  g_rhiDx12->s_renderPassPool.release();
  g_rhiDx12->s_pipelineStatePool.release();
  return true;
}

void SwapchainDx12::releaseInternal() {
  for (auto& iter : m_images_) {
    delete iter;
  }
  m_images_.clear();

  if (m_swapChain_) {
    m_swapChain_ = nullptr;
  }
}

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12