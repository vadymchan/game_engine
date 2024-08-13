#include "gfx/rhi/dx12/swapchain_dx12.h"

#include "gfx/rhi/dx12/rhi_dx12.h"
#include "gfx/rhi/dx12/utils_dx12.h"

namespace game_engine {

//////////////////////////////////////////////////////////////////////////
// SwapchainImageDx12
//////////////////////////////////////////////////////////////////////////
void SwapchainImageDx12::Release() {
  ReleaseInternal();
}

void SwapchainImageDx12::ReleaseInternal() {
  TexturePtr = nullptr;
  // if (Available)
  //{
  //     g_rhi->GetSemaphoreManager()->ReturnSemaphore(Available);
  //     Available = nullptr;
  // }
  // if (RenderFinished)
  //{
  //     g_rhi->GetSemaphoreManager()->ReturnSemaphore(RenderFinished);
  //     RenderFinished = nullptr;
  // }
  // if (RenderFinishedAfterShadow)
  //{
  //     g_rhi->GetSemaphoreManager()->ReturnSemaphore(RenderFinishedAfterShadow);
  //     RenderFinishedAfterShadow = nullptr;
  // }
  // if (RenderFinishedAfterBasePass)
  //{
  //     g_rhi->GetSemaphoreManager()->ReturnSemaphore(RenderFinishedAfterBasePass);
  //     RenderFinishedAfterBasePass = nullptr;
  // }
}

//////////////////////////////////////////////////////////////////////////
// SwapchainDx12
//////////////////////////////////////////////////////////////////////////
bool SwapchainDx12::Create(const std::shared_ptr<Window>& window) {
  DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
  swapChainDesc.BufferCount      = RhiDx12::MaxFrameCount;
  swapChainDesc.Width            = window->getSize().width();
  swapChainDesc.Height           = window->getSize().height();
  swapChainDesc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapChainDesc.BufferUsage      = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.SwapEffect       = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swapChainDesc.SampleDesc.Count = 1;
  swapChainDesc.Flags            = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
                      | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
                      | DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

  assert(g_rhi_dx12);
  assert(!!g_rhi_dx12->m_hWnd);
  assert(g_rhi_dx12->Factory);
  assert(g_rhi_dx12->m_commandBufferManager);

  ComPtr<IDXGISwapChain1> swapChainTemp;
  HRESULT                 hr = g_rhi_dx12->Factory->CreateSwapChainForHwnd(
      g_rhi_dx12->m_commandBufferManager->GetCommandQueue().Get(),
      g_rhi_dx12->m_hWnd,
      &swapChainDesc,
      nullptr,
      nullptr,
      &swapChainTemp);

  assert(SUCCEEDED(hr));
  if (FAILED(hr)) {
    return false;
  }

  hr = swapChainTemp->QueryInterface(IID_PPV_ARGS(&SwapChain));
  assert(SUCCEEDED(hr));
  if (FAILED(hr)) {
    return false;
  }


  Extent = window->getSize();

  Images.resize(RhiDx12::MaxFrameCount);
  for (int32_t i = 0; i < Images.size(); ++i) {
    SwapchainImageDx12* swapchainImage = new SwapchainImageDx12();

    ComPtr<ID3D12Resource> NewResource;
    HRESULT hr = SwapChain->GetBuffer(i, IID_PPV_ARGS(&NewResource));
    assert(SUCCEEDED(hr));

    if (FAILED(hr)) {
      return false;
    }


    std::shared_ptr<CreatedResource> RenderTargetResource
        = CreatedResource::CreatedFromSwapchain(NewResource);

    Images[i] = swapchainImage;

    auto TextureDX12Ptr
        = std::make_shared<TextureDx12>(ETextureType::TEXTURE_2D,
                                          Format,
                                          Extent,
                                          1,
                                          EMSAASamples::COUNT_1,
                                          false,
                                          RTClearValue::Invalid,
                                          RenderTargetResource);
    swapchainImage->TexturePtr = TextureDX12Ptr;

    CreateRenderTargetView((TextureDx12*)swapchainImage->TexturePtr.get());
    TextureDX12Ptr->Layout = EResourceLayout::PRESENT_SRC;
  }

  return true;
}

void SwapchainDx12::Release() {
  ReleaseInternal();
}

bool SwapchainDx12::Resize(int32_t InWidth, int32_t InHeight) {
  bool isSwapChainValid = SwapChain;
  assert(isSwapChainValid);

  if (isSwapChainValid) {
    for (int32_t i = 0; i < g_rhi_dx12->MaxFrameCount; ++i) {
      SwapchainImage* swapchainImage = Images[i];
      auto TexDX12 = (TextureDx12*)swapchainImage->TexturePtr.get();
      TexDX12->m_texture->Resource.get()->Reset();
    }

    SwapChain->SetFullscreenState(false, nullptr);
    HRESULT hr = SwapChain->ResizeBuffers(
        g_rhi_dx12->MaxFrameCount,
        InWidth,
        InHeight,
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
                    ? g_rhi_dx12->Device->GetDeviceRemovedReason()
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

  for (int32_t i = 0; i < g_rhi_dx12->MaxFrameCount; ++i) {
    SwapchainImage* swapchainImage = Images[i];

    ComPtr<ID3D12Resource> NewResource;
    HRESULT hr = SwapChain->GetBuffer(i, IID_PPV_ARGS(&NewResource));
    assert(SUCCEEDED(hr));

    if (FAILED(hr)) {
      return false;
    }


    std::shared_ptr<CreatedResource> RenderTargetResource
        = CreatedResource::CreatedFromSwapchain(NewResource);

    auto TextureDX12Ptr = std::make_shared<TextureDx12>(
        ETextureType::TEXTURE_2D,
        GetDX12TextureFormat(DXGI_FORMAT_R8G8B8A8_UNORM),
        // TODO: remove casting
        math::Dimension2Di{static_cast<int>(InWidth),
                           static_cast<int>(InHeight)},
        1,
        EMSAASamples::COUNT_1,
        false,
        RTClearValue::Invalid,
        RenderTargetResource);
    swapchainImage->TexturePtr = TextureDX12Ptr;

    CreateRenderTargetView((TextureDx12*)swapchainImage->TexturePtr.get());
  }

  g_rhi_dx12->RenderPassPool.Release();
  g_rhi_dx12->PipelineStatePool.Release();
  return true;
}

void SwapchainDx12::ReleaseInternal() {
  for (auto& iter : Images) {
    delete iter;
  }
  Images.clear();

  if (SwapChain) {
    SwapChain = nullptr;
  }
}

}  // namespace game_engine