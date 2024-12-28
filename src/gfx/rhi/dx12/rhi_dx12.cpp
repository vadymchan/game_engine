#include "gfx/rhi/dx12/rhi_dx12.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "file_loader/file.h"
#include "gfx/rhi/dx12/dxc_util.h"
#include "gfx/rhi/dx12/render_frame_context_dx12.h"
#include "gfx/rhi/dx12/ring_buffer_dx12.h"
#include "gfx/rhi/dx12/shader_binding_instance_dx12.h"
#include "gfx/rhi/dx12/shader_binding_layout_dx12.h"
#include "gfx/rhi/dx12/shader_dx12.h"
#include "gfx/rhi/dx12/uniform_buffer_block_dx12.h"
#include "gfx/rhi/dx12/utils_dx12.h"
#include "utils/string/string_conversion.h"

#include <SDL_syswm.h>

// Whether to use InlineDescriptor or DescriptorTable
#define USE_INLINE_DESCRIPTOR               0
// Whether to use temporary Descriptor and m_buffer only for the current frame
#define USE_ONE_FRAME_BUFFER_AND_DESCRIPTOR (USE_INLINE_DESCRIPTOR && 1)

namespace game_engine {

// TODO: seems like not used
struct SimpleConstantBuffer {
  math::Matrix4f<> m_matrix;
  int32_t          m_texIndex = 0;
};

RhiDx12*                                         g_rhiDx12 = nullptr;
std::unordered_map<size_t, ShaderBindingLayout*> RhiDx12::s_shaderBindingPool;
TResourcePool<SamplerStateInfoDx12, MutexRWLock> RhiDx12::s_samplerStatePool;
TResourcePool<RasterizationStateInfoDx12, MutexRWLock>
    RhiDx12::s_rasterizationStatePool;
TResourcePool<StencilOpStateInfoDx12, MutexRWLock>
    RhiDx12::s_stencilOpStatePool;
TResourcePool<DepthStencilStateInfoDx12, MutexRWLock>
    RhiDx12::s_depthStencilStatePool;
TResourcePool<BlendingStateInfoDx12, MutexRWLock> RhiDx12::s_blendingStatePool;
TResourcePool<PipelineStateInfoDx12, MutexRWLock> RhiDx12::s_pipelineStatePool;
TResourcePool<RenderPassDx12, MutexRWLock>        RhiDx12::s_renderPassPool;

// LRESULT CALLBACK WindowProc(HWND   hWnd,
//                             UINT   message,
//                             WPARAM wParam,
//                             LPARAM lParam) {
//   switch (message) {
//     case WM_CREATE: {
//       // Save the DXSample* passed in to CreateWindow.
//       LPCREATESTRUCT pCreateStruct =
//       reinterpret_cast<LPCREATESTRUCT>(lParam); SetWindowLongPtr(
//           hWnd,
//           GWLP_USERDATA,
//           reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
//       return 0;
//     }
//     case WM_KEYDOWN: {
//       return 0;
//     }
//     case WM_KEYUP: {
//       return 0;
//     }
//     case WM_LBUTTONDOWN:
//     case WM_RBUTTONDOWN:
//     case WM_MBUTTONDOWN: {
//       return 0;
//     }
//     case WM_LBUTTONUP:
//     case WM_RBUTTONUP:
//     case WM_MBUTTONUP: {
//       return 0;
//     }
//     case WM_MOUSEMOVE: {
//     }
//     case WM_KILLFOCUS: {
//       return 0;
//     }
//
//     case WM_SIZE: {
//       // TODO: implement resizing
//     }
//       return 0;
//     case WM_ENTERSIZEMOVE:
//       break;
//     case WM_EXITSIZEMOVE:
//       break;
//       return 0;
//     case WM_PAINT:
//       break;
//     case WM_DESTROY:
//       PostQuitMessage(0);
//       return 0;
//   }
//
//   // Handle any messages the switch statement didn't.
//   return DefWindowProc(hWnd, message, wParam, lParam);
// }

//////////////////////////////////////////////////////////////////////////
// RhiDx12
//////////////////////////////////////////////////////////////////////////
RhiDx12::RhiDx12() {
  g_rhiDx12 = this;
}

RhiDx12::~RhiDx12() {
}

// TODO: consider whether parameter window is needed (m_window_ is presented in
// RhiDx12 class)
// HWND RhiDx12::CreateMainWindow(const std::shared_ptr<Window>& window) const
// {
//  auto hInstance = GetModuleHandle(NULL);
//
//  // Initialize the window class.
//  WNDCLASSEX windowClass    = {0};
//  windowClass.cbSize        = sizeof(WNDCLASSEX);
//  windowClass.style         = CS_HREDRAW | CS_VREDRAW;
//  windowClass.lpfnWndProc   = WindowProc;
//  windowClass.hInstance     = hInstance;
//  windowClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
//  windowClass.lpszClassName = "DXSampleClass";
//  RegisterClassEx(&windowClass);
//
//  RECT windowRect = {0,
//                     0,
//                     static_cast<LONG>(window->getSize().width()),
//                     static_cast<LONG>(window->getSize().height())};
//  AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
//
//  // Create the window and store a handle to it.
//  auto hWnd = CreateWindow(windowClass.lpszClassName,
//                           Text("DX12"),
//                           WS_OVERLAPPEDWINDOW,
//                           CW_USEDEFAULT,
//                           CW_USEDEFAULT,
//                           windowRect.right - windowRect.left,
//                           windowRect.bottom - windowRect.top,
//                           nullptr,  // We have no parent window.
//                           nullptr,  // We aren't using menus.
//                           hInstance,
//                           nullptr);
//
//  return hWnd;
//}

int32_t GetHardwareAdapter(IDXGIFactory1*  factory,
                           IDXGIAdapter1** adapter,
                           bool requestHighPerformanceAdapter = false) {
  *adapter = nullptr;

  uint32_t              adapterIndex = 0;
  // TODO: consider renaming
  ComPtr<IDXGIAdapter1> adapter1;
  ComPtr<IDXGIFactory6> factory6;
  HRESULT               hr = factory->QueryInterface(IID_PPV_ARGS(&factory6));
  assert(SUCCEEDED(hr));

  if (SUCCEEDED(hr)) {
    const auto GpuPreference = requestHighPerformanceAdapter
                                 ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE
                                 : DXGI_GPU_PREFERENCE_UNSPECIFIED;

    while (S_OK
           != factory6->EnumAdapterByGpuPreference(
               adapterIndex, GpuPreference, IID_PPV_ARGS(&adapter1))) {
      ++adapterIndex;

      if (!adapter1) {
        continue;
      }

      DXGI_ADAPTER_DESC1 desc;
      adapter1->GetDesc1(&desc);

      if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
        // Do not select the basic render driver adapter. Use warpAdapter if you
        // want a software renderer.
        continue;
      }

      break;
    }
  } else {
    while (S_OK != factory->EnumAdapters1(adapterIndex, &adapter1)) {
      ++adapterIndex;

      DXGI_ADAPTER_DESC1 desc;
      adapter1->GetDesc1(&desc);

      if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
        continue;
      }

      break;
    }
  }

  *adapter = adapter1.Detach();
  return adapterIndex;
}

void RhiDx12::waitForGPU() const {
  assert(m_swapchain_);

  auto Queue = m_commandBufferManager_->getCommandQueue();
  assert(Queue);

  if (m_commandBufferManager_ && m_commandBufferManager_->m_fence_) {
    m_commandBufferManager_->m_fence_->signalWithNextFenceValue(Queue.Get(),
                                                                true);
  }
}

// TODO: dirty solution, consider refactoring
void SetupDebugLayerSettings(ID3D12Device* device) {
  ComPtr<ID3D12InfoQueue> infoQueue;
  if (SUCCEEDED(device->QueryInterface(
          __uuidof(ID3D12InfoQueue),
          reinterpret_cast<void**>(infoQueue.GetAddressOf())))) {
    D3D12_MESSAGE_SEVERITY severities[] = {D3D12_MESSAGE_SEVERITY_INFO,
                                           D3D12_MESSAGE_SEVERITY_WARNING,
                                           D3D12_MESSAGE_SEVERITY_ERROR,
                                           D3D12_MESSAGE_SEVERITY_CORRUPTION};

    D3D12_MESSAGE_ID hide[] = {D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
                               D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE};

    D3D12_INFO_QUEUE_FILTER filter = {};
    filter.AllowList.NumSeverities = _countof(severities);
    filter.AllowList.pSeverityList = severities;
    filter.DenyList.NumIDs         = _countof(hide);
    filter.DenyList.pIDList        = hide;

    infoQueue->PushStorageFilter(&filter);

    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
  }
}

bool RhiDx12::init(const std::shared_ptr<Window>& window) {
  m_window_ = window;

  // TODO: consider remove
  g_rhi = this;

  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version);
  if (SDL_GetWindowWMInfo(m_window_->getNativeWindowHandle(), &wmInfo)
      != SDL_TRUE) {
    // TODO: consider use more modern console output
    printf("SDL_GetWindowWMInfo Error: %s\n", SDL_GetError());
    return false;
  }
  m_hWnd = wmInfo.info.win.window;

  // TODO: remove
  // m_hWnd = CreateMainWindow(m_window_);

  // 1. Device
  uint32_t dxgiFactoryFlags = 0;

#if defined(_DEBUG)
  // Enable debug layer ("optional feature" requires graphic tools)
  // Note: Enabling the debug layer after device creation invalidates the active
  // device.
  {
    ComPtr<ID3D12Debug> debugController;
    HRESULT hr_debug = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
    assert(SUCCEEDED(hr_debug));

    if (SUCCEEDED(hr_debug)) {
      debugController->EnableDebugLayer();

      // GPU-based validation
      ComPtr<ID3D12Debug1> debugController1;
      if (SUCCEEDED(debugController.As(&debugController1))) {
        debugController1->SetEnableGPUBasedValidation(TRUE);
      }

      // Additionally enable the debug layer
      dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
  }
#endif

  ComPtr<IDXGIFactory4> factory;
  HRESULT               hr_factory
      = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
  assert(SUCCEEDED(hr_factory));

  if (FAILED(hr_factory)) {
    return false;
  }

  HRESULT hr_factory_as = factory.As(&m_factory_);
  if (SUCCEEDED(hr_factory_as)) {
    BOOL    allowTearing = false;
    HRESULT hr_feature_support
        = m_factory_->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                                          &allowTearing,
                                          sizeof(allowTearing));
  }

  bool UseWarpDevice = false;  // Whether to use software rasterizer
  if (UseWarpDevice) {
    HRESULT hr_enum_warp = factory->EnumWarpAdapter(IID_PPV_ARGS(&m_adapter_));
    assert(SUCCEEDED(hr_enum_warp));
    if (FAILED(hr_enum_warp)) {
      return false;
    }

    HRESULT hr_create_device = D3D12CreateDevice(
        m_adapter_.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device_));
    assert(SUCCEEDED(hr_create_device));
    if (FAILED(hr_create_device)) {
      return false;
    }
  } else {
    ComPtr<IDXGIAdapter1> hardwareAdapter;
    const int32_t         ResultAdapterID
        = GetHardwareAdapter(factory.Get(), &hardwareAdapter);
    hardwareAdapter.As(&m_adapter_);

    HRESULT hr_create_device_hw = D3D12CreateDevice(
        m_adapter_.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device_));
    assert(SUCCEEDED(hr_create_device_hw));
    if (FAILED(hr_create_device_hw)) {
      return false;
    } else {
      DXGI_ADAPTER_DESC desc;
      m_adapter_->GetDesc(&desc);
      m_adapterId_   = ResultAdapterID;
      m_adapterName_ = desc.Description;

#ifdef _DEBUG
      wchar_t buff[256] = {};
      swprintf_s(buff,
                 L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n",
                 m_adapterId_,
                 desc.VendorId,
                 desc.DeviceId,
                 desc.Description);
      OutputDebugStringW(buff);

      // Existing debug setup call if any
      SetupDebugLayerSettings(m_device_.Get());

      // Add debug info queue setup here
      {
        ComPtr<ID3D12InfoQueue> infoQueue;
        if (SUCCEEDED(m_device_.As(&infoQueue))) {
          infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION,
                                        TRUE);
          infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
          infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

          // Optionally filter out spammy messages
          D3D12_MESSAGE_ID hideMessages[]
              = {D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
                 D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE};

          D3D12_INFO_QUEUE_FILTER filter = {};
          filter.DenyList.NumIDs         = _countof(hideMessages);
          filter.DenyList.pIDList        = hideMessages;
          infoQueue->PushStorageFilter(&filter);
        }
      }
#endif
    }
  }

  m_placedResourcePool_.init();
  // TODO: consider remove nested smart pointers (probably need changes in
  // DeallocatorMultiFrameResource)
  m_deallocatorMultiFrameStandaloneResource_.m_freeDelegate_
      = [](std::shared_ptr<ComPtr<ID3D12Resource>> data) { data->Reset(); };

  //////////////////////////////////////////////////////////////////////////
  // PlacedResouce test
  {
    D3D12_HEAP_DESC heapDesc;
    heapDesc.SizeInBytes                     = s_kDefaultPlacedResourceHeapSize;
    heapDesc.Properties.Type                 = D3D12_HEAP_TYPE_DEFAULT;
    heapDesc.Properties.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapDesc.Properties.CreationNodeMask     = 1;
    heapDesc.Properties.VisibleNodeMask      = 1;
    heapDesc.Alignment                       = 0;
    heapDesc.Flags                           = D3D12_HEAP_FLAG_NONE;

    HRESULT hr_create_heap_default = m_device_->CreateHeap(
        &heapDesc, IID_PPV_ARGS(&m_placedResourceDefaultHeap_));
    assert(SUCCEEDED(hr_create_heap_default));

    if (FAILED(hr_create_heap_default)) {
      return false;
    }
  }

  {
    D3D12_HEAP_DESC heapDesc;
    heapDesc.SizeInBytes                     = s_kDefaultPlacedResourceHeapSize;
    heapDesc.Properties.Type                 = D3D12_HEAP_TYPE_UPLOAD;
    heapDesc.Properties.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapDesc.Properties.CreationNodeMask     = 1;
    heapDesc.Properties.VisibleNodeMask      = 1;
    heapDesc.Alignment                       = 0;
    heapDesc.Flags                           = D3D12_HEAP_FLAG_NONE;

    HRESULT hr_create_heap_upload = m_device_->CreateHeap(
        &heapDesc, IID_PPV_ARGS(&m_placedResourceUploadHeap_));
    assert(SUCCEEDED(hr_create_heap_upload));

    if (FAILED(hr_create_heap_upload)) {
      return false;
    }
  }
  //////////////////////////////////////////////////////////////////////////

  // 2. Command
  m_commandBufferManager_ = new CommandBufferManagerDx12();
  m_commandBufferManager_->initialize(m_device_,
                                      D3D12_COMMAND_LIST_TYPE_DIRECT);
  m_copyCommandBufferManager_ = new CommandBufferManagerDx12();
  m_copyCommandBufferManager_->initialize(m_device_,
                                          D3D12_COMMAND_LIST_TYPE_COPY);

  // 4. Heap
  m_rtvDescriptorHeaps_.initialize(EDescriptorHeapTypeDX12::RTV);
  m_dsvDescriptorHeaps_.initialize(EDescriptorHeapTypeDX12::DSV);
  m_descriptorHeaps_.initialize(EDescriptorHeapTypeDX12::CBV_SRV_UAV);
  m_samplerDescriptorHeaps_.initialize(EDescriptorHeapTypeDX12::SAMPLER);

  // 3. Swapchain
  // m_swapchain_ = new SwapchainDx12();
  m_swapchain_->create(m_window_);
  m_currentFrameIndex_ = m_swapchain_->getCurrentBackBufferIndex();

  m_oneFrameUniformRingBuffers_.resize(m_swapchain_->getNumOfSwapchainImages());
  for (RingBufferDx12*& iter : m_oneFrameUniformRingBuffers_) {
    iter = new RingBufferDx12();
    // iter->Create(16 * 1024 * 1024);
    iter->create(65'536);
  }

  // 7. Create sync object
  waitForGPU();

  // 8. Raytracing device and commandlist
  D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureSupportData{};
  HRESULT                           hr_check_feature_support
      = m_device_->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5,
                                       &featureSupportData,
                                       sizeof(featureSupportData));
  assert(SUCCEEDED(hr_check_feature_support));

  if (FAILED(hr_check_feature_support)) {
    return false;
  }

#if SUPPORT_RAYTRACING
  assert(featureSupportData.RaytracingTier
         > D3D12_RAYTRACING_TIER_NOT_SUPPORTED);
  if (featureSupportData.RaytracingTier
      == D3D12_RAYTRACING_TIER_NOT_SUPPORTED) {
    return false;
  }
#endif

  // QueryPoolTime = new QueryPoolTimeDx12();
  // QueryPoolTime->Create();

  // g_ImGUI = new ImGUIDx12();
  // g_ImGUI->initialize((float)m_window_->getSize().width(),
  //                     (float)m_window_->getSize().height());

  //////////////////////////////////////////////////////////////////////////

  ShowWindow(m_hWnd, SW_SHOW);

#if SUPPORT_RAYTRACING
  RaytracingScene = CreateRaytracingScene();
#endif  // SUPPORT_RAYTRACING

  return true;
}

void RhiDx12::release() {
  waitForGPU();

  RHI::release();

  if (m_commandBufferManager_) {
    m_commandBufferManager_->release();
  }

  if (m_copyCommandBufferManager_) {
    m_copyCommandBufferManager_->release();
  }

  s_samplerStatePool.release();

  {
    ScopeWriteLock s(&m_shaderBindingPoolLock_);
    for (auto& iter : s_shaderBindingPool) {
      delete iter.second;
    }
    s_shaderBindingPool.clear();
  }

  //////////////////////////////////////////////////////////////////////////
  // 8. Raytracing device and commandlist
  m_device_.Reset();

  //////////////////////////////////////////////////////////////////////////
  // 4. Heap
  m_rtvDescriptorHeaps_.release();
  m_dsvDescriptorHeaps_.release();
  m_descriptorHeaps_.release();
  m_samplerDescriptorHeaps_.release();

  m_onlineDescriptorHeapManager_.release();

  //////////////////////////////////////////////////////////////////////////
  // 3. Swapchain
  // TODO: consider using destructor
  m_swapchain_->release();
  // delete m_swapchain_;

  //////////////////////////////////////////////////////////////////////////
  // 2. Command
  delete m_commandBufferManager_;

  m_placedResourcePool_.release();

  //////////////////////////////////////////////////////////////////////////
  // 1. Device
  m_device_.Reset();
}

bool RhiDx12::onHandleResized(uint32_t witdh,
                              uint32_t height,
                              bool     isMinimized) {
  assert(witdh > 0);
  assert(height > 0);

  {
    char szTemp[126];
    sprintf_s(szTemp,
              sizeof(szTemp),
              "Called onHandleResized %d %d\n",
              witdh,
              height);
    OutputDebugStringA(szTemp);
  }

  waitForGPU();

  m_swapchain_->resize(witdh, height);
  m_currentFrameIndex_ = m_swapchain_->getCurrentBackBufferIndex();

  return true;
}

std::shared_ptr<CommandBuffer> RhiDx12::beginSingleTimeCommands() const {
  assert(m_commandBufferManager_);
  return m_commandBufferManager_->getOrCreateCommandBuffer();
}

void RhiDx12::endSingleTimeCommands(
    std::shared_ptr<CommandBuffer> commandBuffer) const {
  auto commandBufferDx12
      = std::static_pointer_cast<CommandBufferDx12>(commandBuffer);

  assert(m_commandBufferManager_);
  m_commandBufferManager_->executeCommandList(commandBufferDx12, true);
  m_commandBufferManager_->returnCommandBuffer(commandBufferDx12);
}

std::shared_ptr<CommandBuffer> RhiDx12::beginSingleTimeCopyCommands() const {
  assert(m_copyCommandBufferManager_);
  return m_copyCommandBufferManager_->getOrCreateCommandBuffer();
}

void RhiDx12::endSingleTimeCopyCommands(
    std::shared_ptr<CommandBufferDx12> commandBuffer) const {
  assert(m_copyCommandBufferManager_);
  m_copyCommandBufferManager_->executeCommandList(commandBuffer, true);
  m_copyCommandBufferManager_->returnCommandBuffer(commandBuffer);
}

std::shared_ptr<Texture> RhiDx12::createTextureFromData(
    const std::shared_ptr<Image>& image) const {
  assert(image->width > 0 && image->height > 0 && image->mipLevels > 0
         && image->arraySize > 0);

  const EResourceLayout        layout = EResourceLayout::GENERAL;
  std::shared_ptr<TextureDx12> TexturePtr;

  // Create the texture resource depending on dimension
  if (image->dimension == ETextureType::TEXTURE_CUBE) {
    // TODO: implement
    // TexturePtr = g_rhi->createCubeTexture<TextureDx12>(
    //    static_cast<uint32_t>(image->width),
    //    static_cast<uint32_t>(image->height),
    //    static_cast<uint32_t>(image->mipLevels),
    //    image->format,
    //    ETextureCreateFlag::UAV,
    //    layout,
    //    RtClearValue::s_kInvalid,
    //    nullptr);
  } else {
    TexturePtr = g_rhi->create2DTexture<TextureDx12>(
        static_cast<uint32_t>(image->width),
        static_cast<uint32_t>(image->height),
        static_cast<uint32_t>(image->arraySize),
        static_cast<uint32_t>(image->mipLevels),
        image->format,
        ETextureCreateFlag::NONE,
        // ETextureCreateFlag::UAV,
        layout,
        image,
        RtClearValue::s_kInvalid,
        nullptr);
  }

  // If sRGB is tracked separately (not shown in current struct), set it here if
  // needed: TexturePtr->m_sRGB_ = image->isSRGB; // if such a field existed in
  // Image

  return TexturePtr;
}

std::shared_ptr<Texture> RhiDx12::createTextureFromDataDeprecated(
    const ImageDataDeprecated* imageData) const {
  assert(imageData);

  const int32_t         MipLevel = imageData->m_mipLevel_;
  const EResourceLayout Layout   = EResourceLayout::GENERAL;

  std::shared_ptr<TextureDx12> TexturePtr;
  if (imageData->m_textureType_ == ETextureType::TEXTURE_CUBE) {
    TexturePtr = g_rhi->createCubeTextureDeprecated<TextureDx12>(
        imageData->m_width_,
        imageData->m_height_,
        MipLevel,
        imageData->m_format_,
        ETextureCreateFlag::UAV,
        Layout,
        imageData->m_imageBulkData_);
  } else {
    TexturePtr = g_rhi->create2DTextureDeprecated<TextureDx12>(
        imageData->m_width_,
        imageData->m_height_,
        imageData->m_layerCount_,
        MipLevel,
        imageData->m_format_,
        ETextureCreateFlag::UAV,
        Layout,
        imageData->m_imageBulkData_);
  }
  TexturePtr->m_sRGB_ = imageData->m_sRGB_;
  return TexturePtr;
}

ShaderBindingLayout* RhiDx12::createShaderBindings(
    const ShaderBindingArray& shaderBindingArray) const {
  size_t hash = shaderBindingArray.getHash();

  {
    ScopeReadLock sr(&m_shaderBindingPoolLock_);

    auto it_find = s_shaderBindingPool.find(hash);
    if (s_shaderBindingPool.end() != it_find) {
      return it_find->second;
    }
  }

  {
    ScopeWriteLock sw(&m_shaderBindingPoolLock_);

    // Try again, to avoid entering creation section simultanteously.
    auto it_find = s_shaderBindingPool.find(hash);
    if (s_shaderBindingPool.end() != it_find) {
      return it_find->second;
    }

    auto NewShaderBinding = new ShaderBindingLayoutDx12();
    NewShaderBinding->initialize(shaderBindingArray);
    NewShaderBinding->m_hash_ = hash;
    s_shaderBindingPool[hash] = NewShaderBinding;

    return NewShaderBinding;
  }
}

SamplerStateInfo* RhiDx12::createSamplerState(
    const SamplerStateInfo& initializer) const {
  return s_samplerStatePool.getOrCreate(initializer);
}

RasterizationStateInfo* RhiDx12::createRasterizationState(
    const RasterizationStateInfo& initializer) const {
  return s_rasterizationStatePool.getOrCreate(initializer);
}

StencilOpStateInfo* RhiDx12::createStencilOpStateInfo(
    const StencilOpStateInfo& initializer) const {
  return s_stencilOpStatePool.getOrCreate(initializer);
}

DepthStencilStateInfo* RhiDx12::createDepthStencilState(
    const DepthStencilStateInfo& initializer) const {
  return s_depthStencilStatePool.getOrCreate(initializer);
}

BlendingStateInfo* RhiDx12::createBlendingState(
    const BlendingStateInfo& initializer) const {
  return s_blendingStatePool.getOrCreate(initializer);
}

// TODO: consider rewriting this method for DX12 shader creation
bool RhiDx12::createShaderInternal(Shader*           shader,
                                   const ShaderInfo& shaderInfo) const {
  std::vector<Name> IncludeFilePaths;
  Shader*           shader_dx12 = shader;
  assert(shader_dx12->getPermutationCount());
  {
    assert(!shader_dx12->m_compiledShader);
    CompiledShaderDx12* CurCompiledShader = new CompiledShaderDx12();
    shader_dx12->m_compiledShader         = CurCompiledShader;

    // Prepare for compilation by setting the PermutationId.
    shader_dx12->setPermutationId(shaderInfo.getPermutationId());

    std::string PermutationDefines;
    shader_dx12->getPermutationDefines(PermutationDefines);

    const wchar_t* ShadingModel = nullptr;
    switch (shaderInfo.getShaderType()) {
      case EShaderAccessStageFlag::VERTEX:
        ShadingModel = TEXT(L"vs_6_6");
        break;
      case EShaderAccessStageFlag::GEOMETRY:
        ShadingModel = TEXT(L"gs_6_6");
        break;
      case EShaderAccessStageFlag::FRAGMENT:
        ShadingModel = TEXT(L"ps_6_6");
        break;
      case EShaderAccessStageFlag::COMPUTE:
        ShadingModel = TEXT(L"cs_6_6");
        break;
      case EShaderAccessStageFlag::RAYTRACING:
      case EShaderAccessStageFlag::RAYTRACING_RAYGEN:
      case EShaderAccessStageFlag::RAYTRACING_MISS:
      case EShaderAccessStageFlag::RAYTRACING_CLOSESTHIT:
      case EShaderAccessStageFlag::RAYTRACING_ANYHIT:
        ShadingModel = TEXT(L"lib_6_6");
        break;
      default:
        assert(0);
        break;
    }

    {
      const bool isHLSL
          = !!strstr(shaderInfo.getShaderFilepath().toStr(), ".hlsl");

      File ShaderFile;
      if (!ShaderFile.openFile(shaderInfo.getShaderFilepath().toStr(),
                               FileType::Text,
                               ReadWriteType::Read)) {
        return false;
      }
      ShaderFile.readFileToBuffer(false);
      std::string ShaderText;

      if (shaderInfo.getPreProcessors().getStringLength() > 0) {
        ShaderText += shaderInfo.getPreProcessors().toStr();
        ShaderText += "\r\n";
      }
      ShaderText += PermutationDefines;
      ShaderText += "\r\n";

      ShaderText += ShaderFile.getBuffer();

      // Find relative file path
      constexpr char    includePrefixString[] = "#include \"";
      constexpr int32_t includePrefixLength   = sizeof(includePrefixString) - 1;

      const std::filesystem::path shaderFilePath(
          shaderInfo.getShaderFilepath().toStr());
      const std::string includeShaderPath
          = shaderFilePath.has_parent_path()
              ? (shaderFilePath.parent_path().string() + "/")
              : "";

      std::set<std::string> AlreadyIncludedSets;
      while (1) {
        size_t startOfInclude = ShaderText.find(includePrefixString);
        if (startOfInclude == std::string::npos) {
          break;
        }

        // Parse include file path
        startOfInclude           += includePrefixLength;
        size_t      endOfInclude  = ShaderText.find("\"", startOfInclude);
        std::string includeFilepath
            = includeShaderPath
            + ShaderText.substr(startOfInclude, endOfInclude - startOfInclude);

        // Replace range '#include "filepath"' with shader file content
        const size_t ReplaceStartPos = startOfInclude - includePrefixLength;
        const size_t ReplaceCount    = endOfInclude - ReplaceStartPos + 1;

        if (AlreadyIncludedSets.contains(includeFilepath)) {
          ShaderText.replace(ReplaceStartPos, ReplaceCount, "");
          continue;
        }

        // If already included file, skip it.
        AlreadyIncludedSets.insert(includeFilepath);
        IncludeFilePaths.push_back(Name(includeFilepath.c_str()));

        // Load include shader file
        File IncludeShaderFile;
        if (!IncludeShaderFile.openFile(
                includeFilepath.c_str(), FileType::Text, ReadWriteType::Read)) {
          return false;
        }
        IncludeShaderFile.readFileToBuffer(false);
        ShaderText.replace(
            ReplaceStartPos, ReplaceCount, IncludeShaderFile.getBuffer());
        IncludeShaderFile.closeFile();
      }

      const std::wstring EntryPoint
          = g_convertToWchar(shaderInfo.getEntryPoint());

      auto& dxcUtil = game_engine::DxcUtil::s_get();

      CurCompiledShader->m_shaderBlob_
          = dxcUtil.compileHlslCode(ShaderText,
                                    shaderInfo.getShaderType(),
                                    EntryPoint,
                                    ShaderBackend::DXIL);

      if (!CurCompiledShader->m_shaderBlob_) {
        return false;
      }
    }
  }
  shader_dx12->m_shaderInfo_ = shaderInfo;
  shader_dx12->m_shaderInfo_.setIncludeShaderFilePaths(IncludeFilePaths);

  return true;
}

RenderPass* RhiDx12::getOrCreateRenderPass(
    const std::vector<Attachment>& colorAttachments,
    const math::Vector2Di&         offset,
    const math::Vector2Di&         extent) const {
  return s_renderPassPool.getOrCreate(
      RenderPassDx12(colorAttachments, offset, extent));
}

RenderPass* RhiDx12::getOrCreateRenderPass(
    const std::vector<Attachment>& colorAttachments,
    const Attachment&              depthAttachment,
    const math::Vector2Di&         offset,
    const math::Vector2Di&         extent) const {
  return s_renderPassPool.getOrCreate(
      RenderPassDx12(colorAttachments, depthAttachment, offset, extent));
}

RenderPass* RhiDx12::getOrCreateRenderPass(
    const std::vector<Attachment>& colorAttachments,
    const Attachment&              depthAttachment,
    const Attachment&              colorResolveAttachment,
    const math::Vector2Di&         offset,
    const math::Vector2Di&         extent) const {
  return s_renderPassPool.getOrCreate(RenderPassDx12(colorAttachments,
                                                     depthAttachment,
                                                     colorResolveAttachment,
                                                     offset,
                                                     extent));
}

RenderPass* RhiDx12::getOrCreateRenderPass(
    const RenderPassInfo&  renderPassInfo,
    const math::Vector2Di& offset,
    const math::Vector2Di& extent) const {
  return s_renderPassPool.getOrCreate(
      RenderPassDx12(renderPassInfo, offset, extent));
}

PipelineStateInfo* RhiDx12::createPipelineStateInfo(
    const PipelineStateFixedInfo*   pipelineStateFixed,
    const GraphicsPipelineShader    shader,
    const VertexBufferArray&        vertexBufferArray,
    const RenderPass*               renderPass,
    const ShaderBindingLayoutArray& shaderBindingArray,
    const PushConstant*             pushConstant,
    int32_t                         subpassIndex) const {
  return s_pipelineStatePool.getOrCreateMove(
      std::move(PipelineStateInfo(pipelineStateFixed,
                                  shader,
                                  vertexBufferArray,
                                  renderPass,
                                  shaderBindingArray,
                                  pushConstant,
                                  subpassIndex)));
}

PipelineStateInfo* RhiDx12::createComputePipelineStateInfo(
    const Shader*                   shader,
    const ShaderBindingLayoutArray& shaderBindingArray,
    const PushConstant*             pushConstant) const {
  return s_pipelineStatePool.getOrCreateMove(
      std::move(PipelineStateInfo(shader, shaderBindingArray, pushConstant)));
}

void RhiDx12::removePipelineStateInfo(size_t hash) {
  s_pipelineStatePool.release(hash);
}

std::shared_ptr<RenderFrameContext> RhiDx12::beginRenderFrame() {
  // SCOPE_CPU_PROFILE(beginRenderFrame);

  //////////////////////////////////////////////////////////////////////////
  // Acquire new swapchain image
  SwapchainImageDx12* currentSwapchainImage
      = (SwapchainImageDx12*)m_swapchain_->getCurrentSwapchainImage();
  assert(m_commandBufferManager_);
  m_commandBufferManager_->m_fence_->waitForFenceValue(
      currentSwapchainImage->m_fenceValue_);
  //////////////////////////////////////////////////////////////////////////

  getOneFrameUniformRingBuffer()->reset();

  auto commandBuffer = std::static_pointer_cast<CommandBufferDx12>(
      m_commandBufferManager_->getOrCreateCommandBuffer());

  auto renderFrameContextPtr
      = std::make_shared<RenderFrameContextDx12>(commandBuffer);
  // TODO: remove constant for UseForwardRenderer
  renderFrameContextPtr->m_useForwardRenderer_ = true;
  renderFrameContextPtr->m_frameIndex_         = m_currentFrameIndex_;
  renderFrameContextPtr->m_sceneRenderTargetPtr_
      = std::make_shared<SceneRenderTarget>();
  renderFrameContextPtr->m_sceneRenderTargetPtr_->create(m_window_,
                                                         currentSwapchainImage);

  return renderFrameContextPtr;
}

void RhiDx12::endRenderFrame(
    const std::shared_ptr<RenderFrameContext>& renderFrameContextPtr) {
  // SCOPE_CPU_PROFILE(endRenderFrame);

  auto commandBuffer = std::static_pointer_cast<CommandBufferDx12>(
      renderFrameContextPtr->getActiveCommandBuffer());

  SwapchainImageDx12* CurrentSwapchainImage
      = (SwapchainImageDx12*)m_swapchain_->getCurrentSwapchainImage();
  g_rhi->transitionLayout(commandBuffer,
                          CurrentSwapchainImage->m_TexturePtr_.get(),
                          EResourceLayout::PRESENT_SRC);

  m_commandBufferManager_->executeCommandList(commandBuffer);

  CurrentSwapchainImage->m_fenceValue_
      = commandBuffer->m_owner_->m_fence_->signalWithNextFenceValue(
          m_commandBufferManager_->getCommandQueue().Get());

  HRESULT hr = S_OK;
  if (g_rhi->isSupportVSync()) {
    // Wait for VSync, the application will sleep until the next VSync.
    // This is done to save cycles of frames that do not appear on the screen.
    hr = m_swapchain_->m_swapChain_->Present(1, 0);
  } else {
    hr = m_swapchain_->m_swapChain_->Present(0, DXGI_PRESENT_ALLOW_TEARING);
  }

  assert(hr == S_OK);

  // CurrentFrameIndex = (CurrentFrameIndex + 1) % m_swapchain_->Images.size();
  m_currentFrameIndex_ = m_swapchain_->getCurrentBackBufferIndex();
  renderFrameContextPtr->destroy();
}

std::shared_ptr<IUniformBufferBlock> RhiDx12::createUniformBufferBlock(
    Name name, LifeTimeType lifeTimeType, size_t size /*= 0*/) const {
  auto uniformBufferBlockPtr
      = std::make_shared<UniformBufferBlockDx12>(name, lifeTimeType);
  uniformBufferBlockPtr->init(size);
  return uniformBufferBlockPtr;
}

void RhiDx12::bindGraphicsShaderBindingInstances(
    const std::shared_ptr<CommandBuffer> commandBuffer,
    const PipelineStateInfo*             piplineState,
    const ShaderBindingInstanceCombiner& shaderBindingInstanceCombiner,
    uint32_t                             firstSet) const {
  // This part will use the previously created one if it is structured.
  if (shaderBindingInstanceCombiner.m_shaderBindingInstanceArray) {
    auto commandBufferDx12
        = std::static_pointer_cast<CommandBufferDx12>(commandBuffer);
    assert(commandBufferDx12);

    const ShaderBindingInstanceArray& m_shaderBindingInstanceArray
        = *(shaderBindingInstanceCombiner.m_shaderBindingInstanceArray);
    commandBufferDx12->m_commandList_->SetGraphicsRootSignature(
        ShaderBindingLayoutDx12::s_createRootSignature(
            m_shaderBindingInstanceArray));

    int32_t RootParameterIndex     = 0;
    int32_t NumOfDescriptor        = 0;
    int32_t NumOfSamplerDescriptor = 0;

    // Check current online descriptor is enough to allocate descriptors, if
    // not, allocate descriptor blocks for current commandlist
    {
      for (int32_t i = 0; i < m_shaderBindingInstanceArray.m_numOfData_; ++i) {
        ShaderBindingInstanceDx12* Instance
            = (ShaderBindingInstanceDx12*)m_shaderBindingInstanceArray[i];
        NumOfDescriptor += (int32_t)Instance->m_descriptors_.size();
        NumOfSamplerDescriptor
            += (int32_t)Instance->m_samplerDescriptors_.size();
      }

      assert(m_device_);

      // Check if the descriptor is sufficient, if not, allocate a new one.
      bool NeedSetDescriptorHeapsAgain = false;
      if (!commandBufferDx12->m_onlineDescriptorHeap_->canAllocate(
              NumOfDescriptor)) {
        const ID3D12DescriptorHeap* PrevDescriptorHeap
            = commandBufferDx12->m_onlineDescriptorHeap_->getHeap();

        commandBufferDx12->m_onlineDescriptorHeap_->release();
        commandBufferDx12->m_onlineDescriptorHeap_
            = ((RhiDx12*)this)
                  ->m_onlineDescriptorHeapManager_.alloc(
                      EDescriptorHeapTypeDX12::CBV_SRV_UAV);
        assert(commandBufferDx12->m_onlineDescriptorHeap_->canAllocate(
            NumOfDescriptor));

        if (PrevDescriptorHeap
            != commandBufferDx12->m_onlineDescriptorHeap_->getHeap()) {
          NeedSetDescriptorHeapsAgain = true;
        }
      }

      // Check if the sampler descriptor is sufficient, if not, allocate a new
      // one.
      const ID3D12DescriptorHeap* PrevOnlineSamplerDescriptorHeap
          = commandBufferDx12->m_onlineDescriptorHeap_->getHeap();
      if (!commandBufferDx12->m_onlineSamplerDescriptorHeap_->canAllocate(
              NumOfSamplerDescriptor)) {
        const ID3D12DescriptorHeap* PrevDescriptorHeap
            = commandBufferDx12->m_onlineSamplerDescriptorHeap_->getHeap();

        commandBufferDx12->m_onlineSamplerDescriptorHeap_->release();
        commandBufferDx12->m_onlineSamplerDescriptorHeap_
            = ((RhiDx12*)this)
                  ->m_onlineDescriptorHeapManager_.alloc(
                      EDescriptorHeapTypeDX12::SAMPLER);
        assert(commandBufferDx12->m_onlineSamplerDescriptorHeap_->canAllocate(
            NumOfSamplerDescriptor));

        if (PrevDescriptorHeap
            != commandBufferDx12->m_onlineSamplerDescriptorHeap_->getHeap()) {
          NeedSetDescriptorHeapsAgain = true;
        }
      }

      // If Descriptor is newly allocated, replace OnlineDescriptorHeap with
      // SetDescriptorHeaps
      if (NeedSetDescriptorHeapsAgain) {
        assert(commandBufferDx12->m_onlineDescriptorHeap_
                   && commandBufferDx12->m_onlineSamplerDescriptorHeap_
               || (!commandBufferDx12->m_onlineDescriptorHeap_
                   && !commandBufferDx12->m_onlineSamplerDescriptorHeap_));
        if (commandBufferDx12->m_onlineDescriptorHeap_
            && commandBufferDx12->m_onlineSamplerDescriptorHeap_) {
          ID3D12DescriptorHeap* ppHeaps[]
              = {commandBufferDx12->m_onlineDescriptorHeap_->getHeap(),
                 commandBufferDx12->m_onlineSamplerDescriptorHeap_->getHeap()};
          commandBufferDx12->m_commandList_->SetDescriptorHeaps(
              _countof(ppHeaps), ppHeaps);
        }
      }
    }

    const D3D12_GPU_DESCRIPTOR_HANDLE FirstGPUDescriptorHandle
        = commandBufferDx12->m_onlineDescriptorHeap_->getGPUHandle(
            commandBufferDx12->m_onlineDescriptorHeap_->getNumOfAllocated());
    const D3D12_GPU_DESCRIPTOR_HANDLE FirstGPUSamplerDescriptorHandle
        = commandBufferDx12->m_onlineSamplerDescriptorHeap_->getGPUHandle(
            commandBufferDx12->m_onlineSamplerDescriptorHeap_
                ->getNumOfAllocated());

    for (int32_t i = 0; i < m_shaderBindingInstanceArray.m_numOfData_; ++i) {
      ShaderBindingInstanceDx12* Instance
          = (ShaderBindingInstanceDx12*)m_shaderBindingInstanceArray[i];
      ShaderBindingLayoutDx12* Layout
          = (ShaderBindingLayoutDx12*)(Instance->m_shaderBindingsLayouts_);

      Instance->copyToOnlineDescriptorHeap(commandBufferDx12);
      Instance->bindGraphics(commandBufferDx12, RootParameterIndex);
    }

    // DescriptorTable is always bound last.
    if (NumOfDescriptor > 0) {
      commandBufferDx12->m_commandList_->SetGraphicsRootDescriptorTable(
          RootParameterIndex++,
          FirstGPUDescriptorHandle);  // StructuredBuffer test, I will use
                                      // descriptor index based on GPU handle
                                      // start of SRVDescriptorHeap
    }

    if (NumOfSamplerDescriptor > 0) {
      commandBufferDx12->m_commandList_->SetGraphicsRootDescriptorTable(
          RootParameterIndex++,
          FirstGPUSamplerDescriptorHandle);  // SamplerState test
    }
  }
}

void RhiDx12::bindComputeShaderBindingInstances(
    const std::shared_ptr<CommandBuffer> commandBuffer,
    const PipelineStateInfo*             piplineState,
    const ShaderBindingInstanceCombiner& shaderBindingInstanceCombiner,
    uint32_t                             firstSet) const {
  // This part will use the previously created one if it is structured.
  if (shaderBindingInstanceCombiner.m_shaderBindingInstanceArray) {
    auto commandBufferDx12
        = std::static_pointer_cast<CommandBufferDx12>(commandBuffer);
    assert(commandBufferDx12);

    const ShaderBindingInstanceArray& m_shaderBindingInstanceArray
        = *(shaderBindingInstanceCombiner.m_shaderBindingInstanceArray);
    commandBufferDx12->m_commandList_->SetComputeRootSignature(
        ShaderBindingLayoutDx12::s_createRootSignature(
            m_shaderBindingInstanceArray));

    int32_t RootParameterIndex   = 0;
    bool    HasDescriptor        = false;
    bool    HasSamplerDescriptor = false;

    const D3D12_GPU_DESCRIPTOR_HANDLE FirstGPUDescriptorHandle
        = commandBufferDx12->m_onlineDescriptorHeap_->getGPUHandle(
            commandBufferDx12->m_onlineDescriptorHeap_->getNumOfAllocated());
    const D3D12_GPU_DESCRIPTOR_HANDLE FirstGPUSamplerDescriptorHandle
        = commandBufferDx12->m_onlineSamplerDescriptorHeap_->getGPUHandle(
            commandBufferDx12->m_onlineSamplerDescriptorHeap_
                ->getNumOfAllocated());

    for (int32_t i = 0; i < m_shaderBindingInstanceArray.m_numOfData_; ++i) {
      ShaderBindingInstanceDx12* Instance
          = (ShaderBindingInstanceDx12*)m_shaderBindingInstanceArray[i];
      ShaderBindingLayoutDx12* Layout
          = (ShaderBindingLayoutDx12*)(Instance->m_shaderBindingsLayouts_);

      Instance->copyToOnlineDescriptorHeap(commandBufferDx12);

      HasDescriptor        |= Instance->m_descriptors_.size() > 0;
      HasSamplerDescriptor |= Instance->m_samplerDescriptors_.size() > 0;

      Instance->bindCompute(commandBufferDx12, RootParameterIndex);
    }

    // DescriptorTable is always bound last.
    if (HasDescriptor) {
      commandBufferDx12->m_commandList_->SetComputeRootDescriptorTable(
          RootParameterIndex++,
          FirstGPUDescriptorHandle);  // StructuredBuffer test, I will use
                                      // descriptor index based on GPU handle
                                      // start of SRVDescriptorHeap
    }

    if (HasSamplerDescriptor) {
      commandBufferDx12->m_commandList_->SetComputeRootDescriptorTable(
          RootParameterIndex++,
          FirstGPUSamplerDescriptorHandle);  // SamplerState test
    }
  }
}

void RhiDx12::bindRaytracingShaderBindingInstances(
    const std::shared_ptr<CommandBuffer> commandBuffer,
    const PipelineStateInfo*             piplineState,
    const ShaderBindingInstanceCombiner& shaderBindingInstanceCombiner,
    uint32_t                             firstSet) const {
  bindComputeShaderBindingInstances(
      commandBuffer, piplineState, shaderBindingInstanceCombiner, firstSet);
}

std::shared_ptr<VertexBuffer> RhiDx12::createVertexBuffer(
    const std::shared_ptr<VertexStreamData>& streamData) const {
  if (!streamData) {
    return nullptr;
  }

  auto vertexBufferPtr = std::make_shared<VertexBufferDx12>();
  vertexBufferPtr->initialize(streamData);
  return vertexBufferPtr;
}

std::shared_ptr<IndexBuffer> RhiDx12::createIndexBuffer(
    const std::shared_ptr<IndexStreamData>& streamData) const {
  if (!streamData) {
    return nullptr;
  }

  assert(streamData);
  assert(streamData->m_stream_);

  auto indexBufferPtr = std::make_shared<IndexBufferDx12>();
  indexBufferPtr->initialize(streamData);
  return indexBufferPtr;
}

std::shared_ptr<Texture> RhiDx12::create2DTexture(
    uint32_t                      width,
    uint32_t                      height,
    uint32_t                      arrayLayers,
    uint32_t                      mipLevels,
    ETextureFormat                format,
    ETextureCreateFlag            textureCreateFlag,
    EResourceLayout               imageLayout,
    const std::shared_ptr<Image>& image,
    const RtClearValue&           clearValue,
    const wchar_t*                resourceName) const {
  // Just create an empty texture resource here.
  auto TexturePtr = g_createTexture(width,
                                    height,
                                    arrayLayers,
                                    mipLevels,
                                    1,
                                    ETextureType::TEXTURE_2D,
                                    format,
                                    textureCreateFlag,
                                    imageLayout,
                                    clearValue,
                                    resourceName);

  // If we have pixel data, upload it
  if (!image->pixels.empty() && !image->subImages.empty()) {
    // Create a staging buffer for pixel data
    auto BufferPtr = g_createBuffer(image->pixels.size(),
                                    0,
                                    EBufferCreateFlag::CPUAccess,
                                    EResourceLayout::READ_ONLY,
                                    image->pixels.data(),
                                    image->pixels.size());
    assert(BufferPtr);

    // Begin copy commands
    auto commandList = std::static_pointer_cast<CommandBufferDx12>(
        beginSingleTimeCopyCommands());

    // Copy buffer to texture using image & subImages directly
    g_copyBufferToTexture(commandList->get(),
                          BufferPtr->m_buffer->get(),
                          TexturePtr->m_texture->get(),
                          image);

    endSingleTimeCopyCommands(commandList);
  }

  return TexturePtr;
}

std::shared_ptr<Texture> RhiDx12::create2DTextureDeprecated(
    uint32_t                       witdh,
    uint32_t                       height,
    uint32_t                       arrayLayers,
    uint32_t                       mipLevels,
    ETextureFormat                 format,
    ETextureCreateFlag             textureCreateFlag,
    EResourceLayout                imageLayout,
    const ImageBulkDataDeprecated& imageBulkData,
    const RtClearValue&            clearValue,
    const wchar_t*                 resourceName) const {
  auto TexturePtr = g_createTexture(witdh,
                                    height,
                                    arrayLayers,
                                    mipLevels,
                                    1,
                                    ETextureType::TEXTURE_2D,
                                    format,
                                    textureCreateFlag,
                                    imageLayout,
                                    clearValue,
                                    resourceName);
  if (imageBulkData.m_imageData_.size() > 0) {
    // TODO: recycle temp buffer
    auto BufferPtr = g_createBuffer(imageBulkData.m_imageData_.size(),
                                    0,
                                    EBufferCreateFlag::CPUAccess,
                                    EResourceLayout::READ_ONLY,
                                    &imageBulkData.m_imageData_[0],
                                    imageBulkData.m_imageData_.size());
    assert(BufferPtr);
    auto commandList = std::static_pointer_cast<CommandBufferDx12>(
        beginSingleTimeCopyCommands());
    if (imageBulkData.m_subresourceFootprints_.size() > 0) {
      g_copyBufferToTextureDeprecated(commandList->get(),
                                      BufferPtr->m_buffer->get(),
                                      TexturePtr->m_texture->get(),
                                      imageBulkData.m_subresourceFootprints_);
    } else {
      g_copyBufferToTextureDeprecated(commandList->get(),
                                      BufferPtr->m_buffer->get(),
                                      0,
                                      TexturePtr->m_texture->get());
    }

    endSingleTimeCopyCommands(commandList);
  }
  return TexturePtr;
}

std::shared_ptr<Texture> RhiDx12::createCubeTextureDeprecated(
    uint32_t                       witdh,
    uint32_t                       height,
    uint32_t                       mipLevels,
    ETextureFormat                 format,
    ETextureCreateFlag             textureCreateFlag,
    EResourceLayout                imageLayout,
    const ImageBulkDataDeprecated& imageBulkData,
    const RtClearValue&            clearValue,
    const wchar_t*                 resourceName) const {
  auto TexturePtr = g_createTexture(witdh,
                                    height,
                                    6,
                                    mipLevels,
                                    1,
                                    ETextureType::TEXTURE_CUBE,
                                    format,
                                    textureCreateFlag,
                                    imageLayout,
                                    clearValue,
                                    resourceName);
  if (imageBulkData.m_imageData_.size() > 0) {
    // TODO: recycle temp buffer
    auto BufferPtr = g_createBuffer(imageBulkData.m_imageData_.size(),
                                    0,
                                    EBufferCreateFlag::CPUAccess,
                                    EResourceLayout::READ_ONLY,
                                    &imageBulkData.m_imageData_[0],
                                    imageBulkData.m_imageData_.size());
    assert(BufferPtr);

    auto commandList = std::static_pointer_cast<CommandBufferDx12>(
        beginSingleTimeCopyCommands());
    if (imageBulkData.m_subresourceFootprints_.size() > 0) {
      g_copyBufferToTextureDeprecated(commandList->get(),
                                      BufferPtr->m_buffer->get(),
                                      TexturePtr->m_texture->get(),
                                      imageBulkData.m_subresourceFootprints_);
    } else {
      g_copyBufferToTextureDeprecated(commandList->get(),
                                      BufferPtr->m_buffer->get(),
                                      0,
                                      TexturePtr->m_texture->get());
    }

    endSingleTimeCopyCommands(commandList);
  }
  return TexturePtr;
}

std::shared_ptr<ShaderBindingInstance> RhiDx12::createShaderBindingInstance(
    const ShaderBindingArray&       shaderBindingArray,
    const ShaderBindingInstanceType type) const {
  auto shaderBindingsLayout = createShaderBindings(shaderBindingArray);
  assert(shaderBindingsLayout);
  return shaderBindingsLayout->createShaderBindingInstance(shaderBindingArray,
                                                           type);
}

void RhiDx12::drawArrays(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext,
    // EPrimitiveType                              type,
    int32_t                                    vertStartIndex,
    int32_t                                    vertCount) const {
  auto commandBufferDx12 = std::static_pointer_cast<CommandBufferDx12>(
      renderFrameContext->getActiveCommandBuffer());
  assert(commandBufferDx12);

  commandBufferDx12->m_commandList_->DrawInstanced(
      vertCount, 1, vertStartIndex, 0);
}

void RhiDx12::drawArraysInstanced(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext,
    // EPrimitiveType                              type,
    int32_t                                    vertStartIndex,
    int32_t                                    vertCount,
    int32_t                                    instanceCount) const {
  auto commandBufferDx12 = std::static_pointer_cast<CommandBufferDx12>(
      renderFrameContext->getActiveCommandBuffer());
  assert(commandBufferDx12);

  commandBufferDx12->m_commandList_->DrawInstanced(
      vertCount, instanceCount, vertStartIndex, 0);
}

void RhiDx12::drawElements(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext,
    // EPrimitiveType                              type,
    int32_t                                    elementSize,
    int32_t                                    startIndex,
    int32_t                                    indexCount) const {
  auto commandBufferDx12 = std::static_pointer_cast<CommandBufferDx12>(
      renderFrameContext->getActiveCommandBuffer());
  assert(commandBufferDx12);

  commandBufferDx12->m_commandList_->DrawIndexedInstanced(
      indexCount, 1, startIndex, 0, 0);
}

void RhiDx12::drawElementsInstanced(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext,
    // EPrimitiveType                              type,
    int32_t                                    elementSize,
    int32_t                                    startIndex,
    int32_t                                    indexCount,
    int32_t                                    instanceCount) const {
  auto commandBufferDx12 = std::static_pointer_cast<CommandBufferDx12>(
      renderFrameContext->getActiveCommandBuffer());
  assert(commandBufferDx12);

  commandBufferDx12->m_commandList_->DrawIndexedInstanced(
      indexCount, instanceCount, startIndex, 0, 0);
}

void RhiDx12::drawElementsBaseVertex(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext,
    // EPrimitiveType                              type,
    int32_t                                    elementSize,
    int32_t                                    startIndex,
    int32_t                                    indexCount,
    int32_t                                    baseVertexIndex) const {
  auto commandBufferDx12 = std::static_pointer_cast<CommandBufferDx12>(
      renderFrameContext->getActiveCommandBuffer());
  assert(commandBufferDx12);

  commandBufferDx12->m_commandList_->DrawIndexedInstanced(
      indexCount, 1, startIndex, baseVertexIndex, 0);
}

void RhiDx12::drawElementsInstancedBaseVertex(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext,
    // EPrimitiveType                              type,
    int32_t                                    elementSize,
    int32_t                                    startIndex,
    int32_t                                    indexCount,
    int32_t                                    baseVertexIndex,
    int32_t                                    instanceCount) const {
  auto commandBufferDx12 = std::static_pointer_cast<CommandBufferDx12>(
      renderFrameContext->getActiveCommandBuffer());
  assert(commandBufferDx12);

  commandBufferDx12->m_commandList_->DrawIndexedInstanced(
      indexCount, instanceCount, startIndex, baseVertexIndex, 0);
}

void RhiDx12::drawIndirect(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext,
    // EPrimitiveType                              type,
    IBuffer*                                   buffer,
    int32_t                                    startIndex,
    int32_t                                    drawCount) const {
  auto commandBufferDx12 = std::static_pointer_cast<CommandBufferDx12>(
      renderFrameContext->getActiveCommandBuffer());
  assert(commandBufferDx12);

  assert(0);
}

void RhiDx12::drawElementsIndirect(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext,
    // EPrimitiveType                              type,
    IBuffer*                                   buffer,
    int32_t                                    startIndex,
    int32_t                                    drawCount) const {
  auto commandBufferDx12 = std::static_pointer_cast<CommandBufferDx12>(
      renderFrameContext->getActiveCommandBuffer());
  assert(commandBufferDx12);

  assert(0);
}

void RhiDx12::dispatchCompute(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext,
    uint32_t                                   numGroupsX,
    uint32_t                                   numGroupsY,
    uint32_t                                   numGroupsZ) const {
  auto commandBufferDx12 = std::static_pointer_cast<CommandBufferDx12>(
      renderFrameContext->getActiveCommandBuffer());
  assert(commandBufferDx12);
  assert(commandBufferDx12->m_commandList_);
  assert(numGroupsX * numGroupsY * numGroupsZ > 0);

  commandBufferDx12->m_commandList_->Dispatch(
      numGroupsX, numGroupsY, numGroupsZ);
}

std::shared_ptr<RenderTarget> RhiDx12::createRenderTarget(
    const RenderTargetInfo& info) const {
  const uint16_t MipLevels
      = info.m_isGenerateMipmap_
          ? static_cast<uint32_t>(std::floor(std::log2(std::max<int>(
                info.m_extent_.width(), info.m_extent_.height()))))
                + 1
          : 1;

  auto TexturePtr
      = g_createTexture(info.m_extent_.width(),
                        info.m_extent_.height(),
                        info.m_layerCount_,
                        MipLevels,
                        (uint32_t)info.m_sampleCount_,
                        info.m_rype_,
                        info.m_format_,
                        (ETextureCreateFlag::RTV | info.m_textureCreateFlag_),
                        EResourceLayout::UNDEFINED,
                        info.m_rtClearValue,
                        // TODO: use info.ResourceName instead of string literal
                        L"RenderTarget");

  auto RenderTargetPtr = std::make_shared<RenderTarget>();
  assert(RenderTargetPtr);
  RenderTargetPtr->m_info_       = info;
  RenderTargetPtr->m_texturePtr_ = TexturePtr;

  return RenderTargetPtr;
}

bool RhiDx12::transitionLayoutInternal(
    std::shared_ptr<CommandBuffer> commandBuffer,
    ID3D12Resource*                resource,
    D3D12_RESOURCE_STATES          srcLayout,
    D3D12_RESOURCE_STATES          dstLayout) const {
  assert(commandBuffer);
  assert(resource);

  if (srcLayout == dstLayout) {
    return true;
  }

  auto commandBufferDx12
      = std::static_pointer_cast<CommandBufferDx12>(commandBuffer);

  D3D12_RESOURCE_BARRIER barrier = {};
  barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource   = resource;
  barrier.Transition.StateBefore = srcLayout;
  barrier.Transition.StateAfter  = dstLayout;
  barrier.Transition.Subresource
      = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;  // TODO: each subresource
                                                  // control
  commandBufferDx12->m_commandList_->ResourceBarrier(1, &barrier);

  return true;
}

bool RhiDx12::transitionLayout(std::shared_ptr<CommandBuffer> commandBuffer,
                               Texture*                       texture,
                               EResourceLayout                newLayout) const {
  assert(commandBuffer);
  assert(texture);

  auto       textureDx12 = (TextureDx12*)texture;
  const auto SrcLayout   = g_getDX12ResourceLayout(textureDx12->m_layout_);
  const auto DstLayout   = g_getDX12ResourceLayout(newLayout);
  if (SrcLayout == DstLayout) {
    return true;
  }

  textureDx12->m_layout_ = newLayout;
  return transitionLayoutInternal(
      commandBuffer, textureDx12->m_texture->get(), SrcLayout, DstLayout);
}

bool RhiDx12::transitionLayoutImmediate(Texture*        texture,
                                        EResourceLayout newLayout) const {
  assert(texture);
  if (texture->getLayout() != newLayout) {
    auto       textureDx12 = (TextureDx12*)texture;
    const auto SrcLayout   = g_getDX12ResourceLayout(textureDx12->m_layout_);
    const auto DstLayout   = g_getDX12ResourceLayout(newLayout);
    if (SrcLayout == DstLayout) {
      return true;
    }

    auto commandBuffer = beginSingleTimeCommands();
    assert(commandBuffer);

    if (commandBuffer) {
      const bool ret = transitionLayoutInternal(
          commandBuffer, textureDx12->m_texture->get(), SrcLayout, DstLayout);
      textureDx12->m_layout_ = newLayout;

      endSingleTimeCommands(commandBuffer);
      return ret;
    }
  }

  return false;
}

void RhiDx12::uavBarrier(std::shared_ptr<CommandBuffer> commandBuffer,
                         Texture*                       texture) const {
  assert(commandBuffer);
  auto commandBufferDx12
      = std::static_pointer_cast<CommandBufferDx12>(commandBuffer);
  auto texture_dx12 = (TextureDx12*)texture;
  assert(texture_dx12->m_texture);

  D3D12_RESOURCE_BARRIER uavBarrier = {};
  uavBarrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_UAV;
  uavBarrier.UAV.pResource          = texture_dx12->m_texture->get();
  assert(uavBarrier.UAV.pResource);
  commandBufferDx12->m_commandList_->ResourceBarrier(1, &uavBarrier);
}

void RhiDx12::uavBarrier(std::shared_ptr<CommandBuffer> commandBuffer,
                         IBuffer*                       buffer) const {
  assert(commandBuffer);
  auto commandBufferDx12
      = std::static_pointer_cast<CommandBufferDx12>(commandBuffer);
  auto buffer_dx12 = (BufferDx12*)buffer;
  assert(buffer_dx12->m_buffer);

  D3D12_RESOURCE_BARRIER uavBarrier = {};
  uavBarrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_UAV;
  uavBarrier.UAV.pResource          = buffer_dx12->m_buffer->get();
  assert(uavBarrier.UAV.pResource);
  commandBufferDx12->m_commandList_->ResourceBarrier(1, &uavBarrier);
}

void RhiDx12::uavBarrierImmediate(Texture* texture) const {
  auto commandBuffer = beginSingleTimeCommands();
  assert(commandBuffer);

  if (commandBuffer) {
    uavBarrier(commandBuffer, texture);
    endSingleTimeCommands(commandBuffer);
  }
}

void RhiDx12::uavBarrierImmediate(IBuffer* buffer) const {
  auto commandBuffer = beginSingleTimeCommands();
  assert(commandBuffer);

  if (commandBuffer) {
    uavBarrier(commandBuffer, buffer);
    endSingleTimeCommands(commandBuffer);
  }
}

bool RhiDx12::transitionLayout(std::shared_ptr<CommandBuffer> commandBuffer,
                               IBuffer*                       buffer,
                               EResourceLayout                newLayout) const {
  assert(commandBuffer);
  assert(buffer);
  assert(buffer->getLayout() != newLayout);

  auto       bufferDx12 = (BufferDx12*)buffer;
  const auto SrcLayout  = g_getDX12ResourceLayout(bufferDx12->m_layout_);
  const auto DstLayout  = g_getDX12ResourceLayout(newLayout);
  if (SrcLayout == DstLayout) {
    return true;
  }

  bufferDx12->m_layout_ = newLayout;
  return transitionLayoutInternal(
      commandBuffer, bufferDx12->m_buffer->get(), SrcLayout, DstLayout);
}

bool RhiDx12::transitionLayoutImmediate(IBuffer*        buffer,
                                        EResourceLayout newLayout) const {
  assert(buffer);
  if (buffer->getLayout() != newLayout) {
    auto       bufferDx12 = (BufferDx12*)buffer;
    const auto SrcLayout  = g_getDX12ResourceLayout(bufferDx12->m_layout_);
    const auto DstLayout  = g_getDX12ResourceLayout(newLayout);
    if (SrcLayout == DstLayout) {
      return true;
    }

    auto commandBuffer = beginSingleTimeCommands();
    assert(commandBuffer);

    if (commandBuffer) {
      const bool ret = transitionLayoutInternal(
          commandBuffer, bufferDx12->m_buffer->get(), SrcLayout, DstLayout);
      bufferDx12->m_layout_ = newLayout;

      endSingleTimeCommands(commandBuffer);
      return ret;
    }
  }

  return false;
}

// void RhiDx12::beginDebugEvent(
//     std::shared_ptr<CommandBuffer> commandBuffer,
//     const char*     name,
//     const Vector4&  color /*= Vector4::g_kColorGreen*/) const {
//   std::shared_ptr<CommandBufferDx12> CommandList =
//   std::static_pointer_cast<CommandBufferDx12>(; assert(CommandList);
//   assert(!CommandList->IsClosed);
//
//   PIXBeginEvent(CommandList->get(),
//                 PIX_COLOR((BYTE)(255 * color.x),
//                           (BYTE)(255 * color.y),
//                           (BYTE)(255 * color.z)),
//                 name);
// }
//
// void RhiDx12::endDebugEvent(std::shared_ptr<CommandBuffer> commandBuffer)
// const {
//   std::shared_ptr<CommandBufferDx12> CommandList =
//   std::static_pointer_cast<CommandBufferDx12>(;//   assert(CommandList);
//   assert(!CommandList->IsClosed);
//
//   PIXEndEvent(CommandList->get());
// }

void RhiDx12::flush() const {
  waitForGPU();
}

void RhiDx12::finish() const {
  waitForGPU();
}

std::shared_ptr<IBuffer> RhiDx12::createStructuredBuffer(
    uint64_t          size,
    uint64_t          alignment,
    uint64_t          stride,
    EBufferCreateFlag bufferCreateFlag,
    EResourceLayout   initialState,
    const void*       data,
    uint64_t          dataSize
    /*, const wchar_t*    resourceName*/) const {
  auto BufferPtr
      = g_createBuffer(size,
                       stride,
                       bufferCreateFlag,
                       initialState,
                       data,
                       dataSize,
                       // TODO: consider why using L prefix in string constant
                       TEXT(L"StructuredBuffer"));

  g_createShaderResourceViewStructuredBuffer(
      BufferPtr.get(), (uint32_t)stride, (uint32_t)(size / stride));

  if (!!(EBufferCreateFlag::UAV & bufferCreateFlag)) {
    g_createUnorderedAccessViewStructuredBuffer(
        BufferPtr.get(), (uint32_t)stride, (uint32_t)(size / stride));
  }

  return std::shared_ptr<IBuffer>(BufferPtr);
}

std::shared_ptr<IBuffer> RhiDx12::createRawBuffer(
    uint64_t          size,
    uint64_t          alignment,
    EBufferCreateFlag bufferCreateFlag,
    EResourceLayout   initialState,
    const void*       data,
    uint64_t          dataSize
    /*, const wchar_t*    resourceName*/) const {
  auto BufferPtr = g_createBuffer(size,
                                  alignment,
                                  bufferCreateFlag,
                                  initialState,
                                  data,
                                  dataSize,
                                  TEXT(L"RawBuffer"));

  g_createShaderResourceViewRaw(BufferPtr.get(), (uint32_t)size);

  if (!!(EBufferCreateFlag::UAV & bufferCreateFlag)) {
    g_createUnorderedAccessViewRaw(BufferPtr.get(), (uint32_t)size);
  }

  return BufferPtr;
}

std::shared_ptr<IBuffer> RhiDx12::createFormattedBuffer(
    uint64_t          size,
    uint64_t          alignment,
    ETextureFormat    format,
    EBufferCreateFlag bufferCreateFlag,
    EResourceLayout   initialState,
    const void*       data,
    uint64_t          dataSize
    /*, const wchar_t*    resourceName*/) const {
  auto BufferPtr = g_createBuffer(size,
                                  alignment,
                                  bufferCreateFlag,
                                  initialState,
                                  data,
                                  dataSize,
                                  TEXT(L"FormattedBuffer"));

  g_createShaderResourceViewFormatted(BufferPtr.get(), format, (uint32_t)size);

  if (!!(EBufferCreateFlag::UAV & bufferCreateFlag)) {
    g_createUnorderedAccessViewFormatted(
        BufferPtr.get(), format, (uint32_t)size);
  }

  return BufferPtr;
}

bool RhiDx12::isSupportVSync() const {
  return false;
}

//////////////////////////////////////////////////////////////////////////
// PlacedResourcePool
void PlacedResourcePool::init() {
  // The allocator should be able to allocate memory larger than the
  // PlacedResourceSizeThreshold.
  assert(g_rhiDx12->s_kPlacedResourceSizeThreshold
         <= s_kMemorySize[(int32_t)EPoolSizeType::MAX - 1]);

  assert(g_rhiDx12);
  // TODO: consider remove nested smart pointers (probably need changes in
  // DeallocatorMultiFrameResource)
  g_rhiDx12->m_deallocatorMultiFramePlacedResource_.m_freeDelegate_ = std::bind(
      &PlacedResourcePool::freedFromPendingDelegateForCreatedResource,
      this,
      std::placeholders::_1);
}

void PlacedResourcePool::release() {
  assert(g_rhiDx12);
  g_rhiDx12->m_deallocatorMultiFramePlacedResource_.m_freeDelegate_ = nullptr;
}

void PlacedResourcePool::free(const ComPtr<ID3D12Resource>& data) {
  if (!data) {
    return;
  }

  if (g_rhiDx12->s_kIsUsePlacedResource) {
    ScopedLock s(&m_lock_);

    auto it_find = m_usingPlacedResources_.find(data.Get());
    if (m_usingPlacedResources_.end() != it_find) {
      auto& PendingList = getPendingPlacedResources(
          it_find->second.m_isUploadResource_, it_find->second.m_size_);
      PendingList.push_back(it_find->second);
      m_usingPlacedResources_.erase(it_find);
      return;
    }

    assert(0);  // can't reach here.
  }
}

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12
