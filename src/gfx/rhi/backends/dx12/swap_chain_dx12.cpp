#include "gfx/rhi/backends/dx12/swap_chain_dx12.h"

#include "gfx/rhi/backends/dx12/descriptor_dx12.h"
#include "gfx/rhi/backends/dx12/device_dx12.h"
#include "gfx/rhi/backends/dx12/rhi_enums_dx12.h"
#include "gfx/rhi/backends/dx12/texture_dx12.h"
#include "platform/common/window.h"
#include "utils/logger/global_logger.h"

#include <SDL_syswm.h>

namespace game_engine {
namespace gfx {
namespace rhi {

SwapChainDx12::SwapChainDx12(const SwapchainDesc& desc, DeviceDx12* device)
    : SwapChain(desc)
    , m_device_(device) {
  // low-level swap chain creation
  if (!createSwapChain_()) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create swap chain (IDXGISwapChain)");
    return;
  }

  // Create render target views for the swap chain buffers
  if (!createRenderTargetViews_()) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create render target views for swap chain");
    return;
  }

  // Create texture objects for the back buffers
  if (!createBackBufferTextures_()) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create texture objects for swap chain back buffers");
    return;
  }
}

SwapChainDx12::~SwapChainDx12() {
  cleanup_();

  // Close the waitable object handle
  if (m_frameLatencyWaitableObject_) {
    CloseHandle(m_frameLatencyWaitableObject_);
    m_frameLatencyWaitableObject_ = nullptr;
  }
}

void SwapChainDx12::cleanup_() {
  m_textures_.clear();

  for (auto& backBuffer : m_backBuffers_) {
    backBuffer.Reset();
  }
  m_backBuffers_.clear();

  if (m_device_) {
    auto* rtvHeap = m_device_->getCpuRtvHeap();
    if (rtvHeap) {
      for (uint32_t index : m_backBuferRtvDescriptorIndices) {
        if (index != UINT32_MAX) {
          rtvHeap->free(index);
        }
      }
    }
  }
  m_backBuferRtvDescriptorIndices.clear();
}

bool SwapChainDx12::createSwapChain_() {
  auto deviceDx12 = static_cast<DeviceDx12*>(m_device_);

  // Get the window handle
  auto          window = deviceDx12->getWindow();
  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version);
  if (!SDL_GetWindowWMInfo(static_cast<SDL_Window*>(window->getNativeWindowHandle()), &wmInfo)) {
    // Handle SDL error
    return false;
  }

  HWND hwnd = wmInfo.info.win.window;

  // Create swap chain
  DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
  swapChainDesc.Width                 = m_desc_.width;
  swapChainDesc.Height                = m_desc_.height;
  swapChainDesc.Format                = g_getTextureFormatDx12(m_desc_.format);
  swapChainDesc.SampleDesc.Count      = 1;
  swapChainDesc.SampleDesc.Quality    = 0;
  swapChainDesc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.BufferCount           = m_desc_.bufferCount;
  swapChainDesc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swapChainDesc.Flags                 = 0;  // No special flags (like vsync)

  // Create IDXGISwapChain1 for Hwnd
  ComPtr<IDXGISwapChain1> swapChain1;
  HRESULT                 hr = deviceDx12->getFactory()->CreateSwapChainForHwnd(
      deviceDx12->getCommandQueue(), hwnd, &swapChainDesc, nullptr, nullptr, &swapChain1);

  if (FAILED(hr)) {
    // Handle error
    return false;
  }

  // Convert to IDXGISwapChain3
  hr = swapChain1.As(&m_swapChain_);
  if (FAILED(hr)) {
    // Handle error
    return false;
  }

  return true;
}

bool SwapChainDx12::createRenderTargetViews_() {
  // Get the number of buffers in the swap chain
  DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
  m_swapChain_->GetDesc1(&swapChainDesc);
  uint32_t bufferCount = swapChainDesc.BufferCount;

  // Resize the back buffer array
  m_backBuffers_.resize(bufferCount);

  // Resize and initialize RTV descriptor indices array
  m_backBuferRtvDescriptorIndices.resize(bufferCount, UINT32_MAX);

  // Get a pointer to the RTV heap
  auto* rtvHeap = m_device_->getCpuRtvHeap();
  if (!rtvHeap) {
    return false;
  }

  // Create an RTV for each back buffer
  for (uint32_t i = 0; i < bufferCount; i++) {
    // Get the back buffer
    HRESULT hr = m_swapChain_->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers_[i]));
    if (FAILED(hr)) {
      GlobalLogger::Log(LogLevel::Error, "Failed to get swap chain buffer");
      return false;
    }

    // Allocate a descriptor in the RTV heap and store its index
    m_backBuferRtvDescriptorIndices[i] = rtvHeap->allocate();
    if (m_backBuferRtvDescriptorIndices[i] == UINT32_MAX) {
      GlobalLogger::Log(LogLevel::Error, "Failed to allocate RTV descriptor");
      return false;
    }

    // Create the RTV
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->getCpuHandle(m_backBuferRtvDescriptorIndices[i]);
    m_device_->getDevice()->CreateRenderTargetView(m_backBuffers_[i].Get(), nullptr, rtvHandle);
  }

  return true;
}

bool SwapChainDx12::createBackBufferTextures_() {
  uint32_t bufferCount = static_cast<uint32_t>(m_backBuffers_.size());
  m_textures_.resize(bufferCount);

  auto* rtvHeap = m_device_->getCpuRtvHeap();
  if (!rtvHeap) {
    return false;
  }

  for (uint32_t i = 0; i < bufferCount; i++) {
    TextureDesc textureDesc;
    textureDesc.type          = TextureType::Texture2D;
    textureDesc.format        = getFormat();
    textureDesc.width         = m_desc_.width;
    textureDesc.height        = m_desc_.height;
    textureDesc.depth         = 1;
    textureDesc.mipLevels     = 1;
    textureDesc.arraySize     = 1;
    textureDesc.sampleCount   = MSAASamples::Count1;
    textureDesc.createFlags   = TextureCreateFlag::Rtv;
    textureDesc.initialLayout = ResourceLayout::PresentSrc;

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->getCpuHandle(m_backBuferRtvDescriptorIndices[i]);

    m_textures_[i] = std::make_unique<TextureDx12>(
        m_device_, textureDesc, m_backBuffers_[i].Get(), rtvHandle, D3D12_CPU_DESCRIPTOR_HANDLE{});
  }

  return true;
}

Texture* SwapChainDx12::getCurrentImage() {
  return m_textures_[m_currentBackBufferIndex_].get();
}

uint32_t SwapChainDx12::getCurrentImageIndex() const {
  return m_currentBackBufferIndex_;
}

bool SwapChainDx12::acquireNextImage() {
  // In DX12, the SwapChain::Present call advances to the next back buffer,
  // so we just update our index. The GetCurrentBackBufferIndex() function
  // returns the index of the buffer that should be rendered to next.
  m_currentBackBufferIndex_ = m_swapChain_->GetCurrentBackBufferIndex();

  // Wait for the next frame to be signaled if we're using a waitable object
  if (m_frameLatencyWaitableObject_) {
    // This waits until the GPU has released the next frame
    constexpr DWORD waitTime   = 1000;  // 1 second timeout
    DWORD           waitResult = WaitForSingleObjectEx(m_frameLatencyWaitableObject_, waitTime, TRUE);
    if (waitResult != WAIT_OBJECT_0) {
      GlobalLogger::Log(LogLevel::Warning, "Failed to wait for frame latency waitable object");
    }
  }

  return true;
}

bool SwapChainDx12::present() {
  UINT syncInterval = 0;  // No vsync
  UINT presentFlags = 0;

  HRESULT hr = m_swapChain_->Present(syncInterval, presentFlags);

  if (FAILED(hr)) {
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
      // Handle device lost scenario (would need to recreate the device)
      GlobalLogger::Log(LogLevel::Error, "GPU device removed or reset");
    } else {
      GlobalLogger::Log(LogLevel::Error, "Failed to present swap chain");
    }
    return false;
  }

  return true;
}

bool SwapChainDx12::resize(uint32_t width, uint32_t height) {
  // If dimensions haven't changed, do nothing
  if (width == m_desc_.width && height == m_desc_.height) {
    return true;
  }

  // Wait for the GPU to finish using the resources
  m_device_->waitIdle();

  // Update the dimensions
  m_desc_.width  = width;
  m_desc_.height = height;

  // Release existing resources
  cleanup_();

  // Resize the swap chain
  DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
  m_swapChain_->GetDesc1(&swapChainDesc);

  HRESULT hr = m_swapChain_->ResizeBuffers(
      swapChainDesc.BufferCount, width, height, swapChainDesc.Format, swapChainDesc.Flags);

  if (FAILED(hr)) {
    GlobalLogger::Log(LogLevel::Error, "Failed to resize swap chain buffers");
    return false;
  }

  // Get the new back buffer index
  m_currentBackBufferIndex_ = m_swapChain_->GetCurrentBackBufferIndex();

  // Recreate render target views and texture objects
  if (!createRenderTargetViews_() || !createBackBufferTextures_()) {
    GlobalLogger::Log(LogLevel::Error, "Failed to recreate render targets after resize");
    return false;
  }

  return true;
}

TextureFormat SwapChainDx12::getFormat() const {
  DXGI_SWAP_CHAIN_DESC1 desc;
  m_swapChain_->GetDesc1(&desc);
  return g_getTextureFormatDx12(desc.Format);
}

uint32_t SwapChainDx12::getWidth() const {
  return m_desc_.width;
}

uint32_t SwapChainDx12::getHeight() const {
  return m_desc_.height;
}

uint32_t SwapChainDx12::getBufferCount() const {
  DXGI_SWAP_CHAIN_DESC1 desc;
  m_swapChain_->GetDesc1(&desc);
  return desc.BufferCount;
}

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine