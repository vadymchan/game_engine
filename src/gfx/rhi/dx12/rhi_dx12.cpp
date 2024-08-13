#include "gfx/rhi/dx12/rhi_dx12.h"

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
  math::Matrix4f M;
  int32_t        TexIndex = 0;
};

RhiDx12*                                         g_rhi_dx12 = nullptr;
std::unordered_map<size_t, ShaderBindingLayout*>  RhiDx12::ShaderBindingPool;
TResourcePool<SamplerStateInfoDx12, MutexRWLock> RhiDx12::SamplerStatePool;
TResourcePool<RasterizationStateInfoDx12, MutexRWLock>
    RhiDx12::RasterizationStatePool;
TResourcePool<StencilOpStateInfoDx12, MutexRWLock>
    RhiDx12::StencilOpStatePool;
TResourcePool<DepthStencilStateInfoDx12, MutexRWLock>
    RhiDx12::DepthStencilStatePool;
TResourcePool<BlendingStateInfoDx12, MutexRWLock>
    RhiDx12::BlendingStatePool;
TResourcePool<PipelineStateInfoDx12, MutexRWLock>
                                             RhiDx12::PipelineStatePool;
TResourcePool<RenderPassDx12, MutexRWLock> RhiDx12::RenderPassPool;

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
  g_rhi_dx12 = this;
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
//                           TEXT("DX12"),
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

int32_t GetHardwareAdapter(IDXGIFactory1*  InFactory,
                           IDXGIAdapter1** InAdapter,
                           bool requestHighPerformanceAdapter = false) {
  *InAdapter = nullptr;

  uint32_t              adapterIndex = 0;
  ComPtr<IDXGIAdapter1> adapter;
  ComPtr<IDXGIFactory6> factory6;
  HRESULT               hr = InFactory->QueryInterface(IID_PPV_ARGS(&factory6));
  assert(SUCCEEDED(hr));

  if (SUCCEEDED(hr)) {
    const auto GpuPreference = requestHighPerformanceAdapter
                                 ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE
                                 : DXGI_GPU_PREFERENCE_UNSPECIFIED;

    while (S_OK
           != factory6->EnumAdapterByGpuPreference(
               adapterIndex, GpuPreference, IID_PPV_ARGS(&adapter))) {
      ++adapterIndex;

      if (!adapter) {
        continue;
      }

      DXGI_ADAPTER_DESC1 desc;
      adapter->GetDesc1(&desc);

      if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
        // Do not select the basic render driver adapter. Use warpAdapter if you
        // want a software renderer.
        continue;
      }

      break;
    }
  } else {
    while (S_OK != InFactory->EnumAdapters1(adapterIndex, &adapter)) {
      ++adapterIndex;

      DXGI_ADAPTER_DESC1 desc;
      adapter->GetDesc1(&desc);

      if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
        continue;
      }

      break;
    }
  }

  *InAdapter = adapter.Detach();
  return adapterIndex;
}

void RhiDx12::WaitForGPU() const {
  assert(m_swapchain_);

  auto Queue = m_commandBufferManager->GetCommandQueue();
  assert(Queue);

  if (m_commandBufferManager && m_commandBufferManager->m_fence) {
    m_commandBufferManager->m_fence->SignalWithNextFenceValue(Queue.Get(), true);
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

  HRESULT hr_factory_as = factory.As(&Factory);
  if (SUCCEEDED(hr_factory_as)) {
    BOOL    allowTearing = false;
    HRESULT hr_feature_support
        = Factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                                       &allowTearing,
                                       sizeof(allowTearing));
  }

  bool UseWarpDevice = false;  // Whether to use software rasterizer
  if (UseWarpDevice) {
    HRESULT hr_enum_warp = factory->EnumWarpAdapter(IID_PPV_ARGS(&Adapter));
    assert(SUCCEEDED(hr_enum_warp));
    if (FAILED(hr_enum_warp)) {
      return false;
    }

    HRESULT hr_create_device = D3D12CreateDevice(
        Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&Device));
    assert(SUCCEEDED(hr_create_device));
    if (FAILED(hr_create_device)) {
      return false;
    }
  } else {
    ComPtr<IDXGIAdapter1> hardwareAdapter;
    const int32_t         ResultAdapterID
        = GetHardwareAdapter(factory.Get(), &hardwareAdapter);
    hardwareAdapter.As(&Adapter);

    HRESULT hr_create_device_hw = D3D12CreateDevice(
        Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&Device));
    assert(SUCCEEDED(hr_create_device_hw));
    if (FAILED(hr_create_device_hw)) {
      return false;
    } else {
      DXGI_ADAPTER_DESC desc;
      Adapter->GetDesc(&desc);
      AdapterID   = ResultAdapterID;
      AdapterName = desc.Description;

#ifdef _DEBUG
      wchar_t buff[256] = {};
      swprintf_s(buff,
                 L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n",
                 AdapterID,
                 desc.VendorId,
                 desc.DeviceId,
                 desc.Description);
      OutputDebugStringW(buff);

      SetupDebugLayerSettings(Device.Get());
#endif
    }
  }

  m_placedResourcePool.Init();
  // TODO: consider remove nested smart pointers (probably need changes in
  // DeallocatorMultiFrameResource)
  DeallocatorMultiFrameStandaloneResource.FreeDelegate
      = [](std::shared_ptr<ComPtr<ID3D12Resource>> InData) { InData->Reset(); };

  //////////////////////////////////////////////////////////////////////////
  // PlacedResouce test
  {
    D3D12_HEAP_DESC heapDesc;
    heapDesc.SizeInBytes                     = GDefaultPlacedResourceHeapSize;
    heapDesc.Properties.Type                 = D3D12_HEAP_TYPE_DEFAULT;
    heapDesc.Properties.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapDesc.Properties.CreationNodeMask     = 1;
    heapDesc.Properties.VisibleNodeMask      = 1;
    heapDesc.Alignment                       = 0;
    heapDesc.Flags                           = D3D12_HEAP_FLAG_NONE;

    HRESULT hr_create_heap_default = Device->CreateHeap(
        &heapDesc, IID_PPV_ARGS(&PlacedResourceDefaultHeap));
    assert(SUCCEEDED(hr_create_heap_default));

    if (FAILED(hr_create_heap_default)) {
      return false;
    }
  }

  {
    D3D12_HEAP_DESC heapDesc;
    heapDesc.SizeInBytes                     = GDefaultPlacedResourceHeapSize;
    heapDesc.Properties.Type                 = D3D12_HEAP_TYPE_UPLOAD;
    heapDesc.Properties.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapDesc.Properties.CreationNodeMask     = 1;
    heapDesc.Properties.VisibleNodeMask      = 1;
    heapDesc.Alignment                       = 0;
    heapDesc.Flags                           = D3D12_HEAP_FLAG_NONE;

    HRESULT hr_create_heap_upload = Device->CreateHeap(
        &heapDesc, IID_PPV_ARGS(&PlacedResourceUploadHeap));
    assert(SUCCEEDED(hr_create_heap_upload));

    if (FAILED(hr_create_heap_upload)) {
      return false;
    }
  }
  //////////////////////////////////////////////////////////////////////////

  // 2. Command
  m_commandBufferManager = new CommandBufferManagerDx12();
  m_commandBufferManager->Initialize(Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
  CopyCommandBufferManager = new CommandBufferManagerDx12();
  CopyCommandBufferManager->Initialize(Device, D3D12_COMMAND_LIST_TYPE_COPY);

  // 4. Heap
  RTVDescriptorHeaps.Initialize(EDescriptorHeapTypeDX12::RTV);
  DSVDescriptorHeaps.Initialize(EDescriptorHeapTypeDX12::DSV);
  DescriptorHeaps.Initialize(EDescriptorHeapTypeDX12::CBV_SRV_UAV);
  SamplerDescriptorHeaps.Initialize(EDescriptorHeapTypeDX12::SAMPLER);

  // 3. Swapchain
  // m_swapchain_ = new SwapchainDx12();
  m_swapchain_->Create(m_window_);
  CurrentFrameIndex = m_swapchain_->GetCurrentBackBufferIndex();

  OneFrameUniformRingBuffers.resize(m_swapchain_->GetNumOfSwapchainImages());
  for (RingBufferDx12*& iter : OneFrameUniformRingBuffers) {
    iter = new RingBufferDx12();
    // iter->Create(16 * 1024 * 1024);
    iter->Create(65'536);
  }

  // 7. Create sync object
  WaitForGPU();

  // 8. Raytracing device and commandlist
  D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureSupportData{};
  HRESULT                           hr_check_feature_support
      = Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5,
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
  // g_ImGUI->Initialize((float)m_window_->getSize().width(),
  //                     (float)m_window_->getSize().height());

  //////////////////////////////////////////////////////////////////////////

  ShowWindow(m_hWnd, SW_SHOW);

#if SUPPORT_RAYTRACING
  RaytracingScene = CreateRaytracingScene();
#endif  // SUPPORT_RAYTRACING

  return true;
}

void RhiDx12::release() {
  WaitForGPU();

  RHI::release();

  if (m_commandBufferManager) {
    m_commandBufferManager->Release();
  }

  if (CopyCommandBufferManager) {
    CopyCommandBufferManager->Release();
  }

  SamplerStatePool.Release();

  {
    ScopeWriteLock s(&ShaderBindingPoolLock);
    for (auto& iter : ShaderBindingPool) {
      delete iter.second;
    }
    ShaderBindingPool.clear();
  }

  //////////////////////////////////////////////////////////////////////////
  // 8. Raytracing device and commandlist
  Device.Reset();

  //////////////////////////////////////////////////////////////////////////
  // 4. Heap
  RTVDescriptorHeaps.Release();
  DSVDescriptorHeaps.Release();
  DescriptorHeaps.Release();
  SamplerDescriptorHeaps.Release();

  OnlineDescriptorHeapManager.Release();

  //////////////////////////////////////////////////////////////////////////
  // 3. Swapchain
  // TODO: consider using destructor
  m_swapchain_->Release();
  // delete m_swapchain_;

  //////////////////////////////////////////////////////////////////////////
  // 2. Command
  delete m_commandBufferManager;

  m_placedResourcePool.Release();

  //////////////////////////////////////////////////////////////////////////
  // 1. Device
  Device.Reset();
}

bool RhiDx12::OnHandleResized(uint32_t InWidth,
                                uint32_t InHeight,
                                bool     InIsMinimized) {
  assert(InWidth > 0);
  assert(InHeight > 0);

  {
    char szTemp[126];
    sprintf_s(szTemp,
              sizeof(szTemp),
              "Called OnHandleResized %d %d\n",
              InWidth,
              InHeight);
    OutputDebugStringA(szTemp);
  }

  WaitForGPU();

  m_swapchain_->Resize(InWidth, InHeight);
  CurrentFrameIndex = m_swapchain_->GetCurrentBackBufferIndex();

  return true;
}

CommandBufferDx12* RhiDx12::BeginSingleTimeCommands() const {
  assert(m_commandBufferManager);
  return m_commandBufferManager->GetOrCreateCommandBuffer();
}

void RhiDx12::EndSingleTimeCommands(CommandBuffer* commandBuffer) const {
  auto commandBufferDx12 = (CommandBufferDx12*)commandBuffer;

  assert(m_commandBufferManager);
  m_commandBufferManager->ExecuteCommandList(commandBufferDx12, true);
  m_commandBufferManager->ReturnCommandBuffer(commandBufferDx12);
}

CommandBufferDx12* RhiDx12::BeginSingleTimeCopyCommands() const {
  assert(CopyCommandBufferManager);
  return CopyCommandBufferManager->GetOrCreateCommandBuffer();
}

void RhiDx12::EndSingleTimeCopyCommands(
    CommandBufferDx12* commandBuffer) const {
  assert(CopyCommandBufferManager);
  CopyCommandBufferManager->ExecuteCommandList(commandBuffer, true);
  CopyCommandBufferManager->ReturnCommandBuffer(commandBuffer);
}

std::shared_ptr<Texture> RhiDx12::CreateTextureFromData(
    const ImageData* InImageData) const {
  assert(InImageData);

  const int32_t         MipLevel = InImageData->MipLevel;
  const EResourceLayout Layout   = EResourceLayout::GENERAL;

  std::shared_ptr<TextureDx12> TexturePtr;
  if (InImageData->TextureType == ETextureType::TEXTURE_CUBE) {
    TexturePtr
        = g_rhi->CreateCubeTexture<TextureDx12>(InImageData->Width,
                                                InImageData->Height,
                                                MipLevel,
                                                InImageData->Format,
                                                ETextureCreateFlag::UAV,
                                                Layout,
                                                InImageData->imageBulkData);
  } else {
    TexturePtr
        = g_rhi->Create2DTexture<TextureDx12>(InImageData->Width,
                                              InImageData->Height,
                                              InImageData->LayerCount,
                                              MipLevel,
                                              InImageData->Format,
                                              ETextureCreateFlag::UAV,
                                              Layout,
                                              InImageData->imageBulkData);
  }
  TexturePtr->sRGB = InImageData->sRGB;
  return TexturePtr;
}

ShaderBindingLayout* RhiDx12::CreateShaderBindings(
    const ShaderBindingArray& InShaderBindingArray) const {
  size_t hash = InShaderBindingArray.GetHash();

  {
    ScopeReadLock sr(&ShaderBindingPoolLock);

    auto it_find = ShaderBindingPool.find(hash);
    if (ShaderBindingPool.end() != it_find) {
      return it_find->second;
    }
  }

  {
    ScopeWriteLock sw(&ShaderBindingPoolLock);

    // Try again, to avoid entering creation section simultanteously.
    auto it_find = ShaderBindingPool.find(hash);
    if (ShaderBindingPool.end() != it_find) {
      return it_find->second;
    }

    auto NewShaderBinding = new ShaderBindingLayoutDx12();
    NewShaderBinding->Initialize(InShaderBindingArray);
    NewShaderBinding->Hash  = hash;
    ShaderBindingPool[hash] = NewShaderBinding;

    return NewShaderBinding;
  }
}

SamplerStateInfo* RhiDx12::CreateSamplerState(
    const SamplerStateInfo& initializer) const {
  return SamplerStatePool.GetOrCreate(initializer);
}

RasterizationStateInfo* RhiDx12::CreateRasterizationState(
    const RasterizationStateInfo& initializer) const {
  return RasterizationStatePool.GetOrCreate(initializer);
}

StencilOpStateInfo* RhiDx12::CreateStencilOpStateInfo(
    const StencilOpStateInfo& initializer) const {
  return StencilOpStatePool.GetOrCreate(initializer);
}

DepthStencilStateInfo* RhiDx12::CreateDepthStencilState(
    const DepthStencilStateInfo& initializer) const {
  return DepthStencilStatePool.GetOrCreate(initializer);
}

BlendingStateInfo* RhiDx12::CreateBlendingState(
    const BlendingStateInfo& initializer) const {
  return BlendingStatePool.GetOrCreate(initializer);
}

// TODO: consider rewriting this method for DX12 shader creation
bool RhiDx12::CreateShaderInternal(Shader*           OutShader,
                                   const ShaderInfo& shaderInfo) const {
  std::vector<Name> IncludeFilePaths;
  Shader*           shader_dx12 = OutShader;
  assert(shader_dx12->GetPermutationCount());
  {
    assert(!shader_dx12->m_compiledShader);
    CompiledShaderDx12* CurCompiledShader = new CompiledShaderDx12();
    shader_dx12->m_compiledShader         = CurCompiledShader;

    // Prepare for compilation by setting the PermutationId.
    shader_dx12->SetPermutationId(shaderInfo.GetPermutationId());

    std::string PermutationDefines;
    shader_dx12->GetPermutationDefines(PermutationDefines);

    const wchar_t* ShadingModel = nullptr;
    switch (shaderInfo.GetShaderType()) {
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
          = !!strstr(shaderInfo.GetShaderFilepath().ToStr(), ".hlsl");

      File ShaderFile;
      if (!ShaderFile.OpenFile(shaderInfo.GetShaderFilepath().ToStr(),
                               FileType::TEXT,
                               ReadWriteType::READ)) {
        return false;
      }
      ShaderFile.ReadFileToBuffer(false);
      std::string ShaderText;

      if (shaderInfo.GetPreProcessors().GetStringLength() > 0) {
        ShaderText += shaderInfo.GetPreProcessors().ToStr();
        ShaderText += "\r\n";
      }
      ShaderText += PermutationDefines;
      ShaderText += "\r\n";

      ShaderText += ShaderFile.GetBuffer();

      // Find relative file path
      constexpr char    includePrefixString[] = "#include \"";
      constexpr int32_t includePrefixLength   = sizeof(includePrefixString) - 1;

      const std::filesystem::path shaderFilePath(
          shaderInfo.GetShaderFilepath().ToStr());
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
        if (!IncludeShaderFile.OpenFile(
                includeFilepath.c_str(), FileType::TEXT, ReadWriteType::READ)) {
          return false;
        }
        IncludeShaderFile.ReadFileToBuffer(false);
        ShaderText.replace(
            ReplaceStartPos, ReplaceCount, IncludeShaderFile.GetBuffer());
        IncludeShaderFile.CloseFile();
      }

      const std::wstring EntryPoint
          = ConvertToWchar(shaderInfo.GetEntryPoint());

      auto& dxcUtil = game_engine::DxcUtil::GetInstance();
      if (FAILED(dxcUtil.Initialize())) {
        std::cerr << "Failed to initialize DxcUtil" << std::endl;
        return false;
      }

      CurCompiledShader->ShaderBlob
          = dxcUtil.compileHlslCodeToDxil(ShaderText, ShadingModel, EntryPoint);

      if (!CurCompiledShader->ShaderBlob) {
        return false;
      }
    }
  }
  shader_dx12->shaderInfo = shaderInfo;
  shader_dx12->shaderInfo.SetIncludeShaderFilePaths(IncludeFilePaths);

  return true;
}

RenderPass* RhiDx12::GetOrCreateRenderPass(
    const std::vector<Attachment>& colorAttachments,
    const math::Vector2Di&         offset,
    const math::Vector2Di&         extent) const {
  return RenderPassPool.GetOrCreate(
      RenderPassDx12(colorAttachments, offset, extent));
}

RenderPass* RhiDx12::GetOrCreateRenderPass(
    const std::vector<Attachment>& colorAttachments,
    const Attachment&              depthAttachment,
    const math::Vector2Di&         offset,
    const math::Vector2Di&         extent) const {
  return RenderPassPool.GetOrCreate(
      RenderPassDx12(colorAttachments, depthAttachment, offset, extent));
}

RenderPass* RhiDx12::GetOrCreateRenderPass(
    const std::vector<Attachment>& colorAttachments,
    const Attachment&              depthAttachment,
    const Attachment&              colorResolveAttachment,
    const math::Vector2Di&         offset,
    const math::Vector2Di&         extent) const {
  return RenderPassPool.GetOrCreate(RenderPassDx12(colorAttachments,
                                                   depthAttachment,
                                                   colorResolveAttachment,
                                                   offset,
                                                   extent));
}

RenderPass* RhiDx12::GetOrCreateRenderPass(
    const RenderPassInfo&  renderPassInfo,
    const math::Vector2Di& offset,
    const math::Vector2Di& extent) const {
  return RenderPassPool.GetOrCreate(
      RenderPassDx12(renderPassInfo, offset, extent));
}

PipelineStateInfo* RhiDx12::CreatePipelineStateInfo(
    const PipelineStateFixedInfo*    InPipelineStateFixed,
    const GraphicsPipelineShader     InShader,
    const VertexBufferArray&         InVertexBufferArray,
    const RenderPass*                InRenderPass,
    const ShaderBindingLayoutArray& InShaderBindingArray,
    const PushConstant*              InPushConstant,
    int32_t                          InSubpassIndex) const {
  return PipelineStatePool.GetOrCreateMove(
      std::move(PipelineStateInfo(InPipelineStateFixed,
                                  InShader,
                                  InVertexBufferArray,
                                  InRenderPass,
                                  InShaderBindingArray,
                                  InPushConstant,
                                  InSubpassIndex)));
}

PipelineStateInfo* RhiDx12::CreateComputePipelineStateInfo(
    const Shader*                    shader,
    const ShaderBindingLayoutArray& InShaderBindingArray,
    const PushConstant*              pushConstant) const {
  return PipelineStatePool.GetOrCreateMove(
      std::move(PipelineStateInfo(shader, InShaderBindingArray, pushConstant)));
}

void RhiDx12::RemovePipelineStateInfo(size_t InHash) {
  PipelineStatePool.Release(InHash);
}

std::shared_ptr<RenderFrameContext> RhiDx12::BeginRenderFrame() {
  // SCOPE_CPU_PROFILE(BeginRenderFrame);

  //////////////////////////////////////////////////////////////////////////
  // Acquire new swapchain image
  SwapchainImageDx12* CurrentSwapchainImage
      = (SwapchainImageDx12*)m_swapchain_->GetCurrentSwapchainImage();
  assert(m_commandBufferManager);
  m_commandBufferManager->m_fence->WaitForFenceValue(
      CurrentSwapchainImage->FenceValue);
  //////////////////////////////////////////////////////////////////////////

  GetOneFrameUniformRingBuffer()->Reset();

  CommandBufferDx12* commandBuffer
      = (CommandBufferDx12*)m_commandBufferManager->GetOrCreateCommandBuffer();

  auto renderFrameContextPtr
      = std::make_shared<RenderFrameContextDx12>(commandBuffer);
  // TODO: remove constant for UseForwardRenderer
  renderFrameContextPtr->UseForwardRenderer = true;
  renderFrameContextPtr->FrameIndex         = CurrentFrameIndex;
  renderFrameContextPtr->SceneRenderTargetPtr
      = std::make_shared<SceneRenderTarget>();
  renderFrameContextPtr->SceneRenderTargetPtr->Create(m_window_,
                                                      CurrentSwapchainImage);

  return renderFrameContextPtr;
}

void RhiDx12::EndRenderFrame(
    const std::shared_ptr<RenderFrameContext>& renderFrameContextPtr) {
  // SCOPE_CPU_PROFILE(EndRenderFrame);

  CommandBufferDx12* commandBuffer
      = (CommandBufferDx12*)renderFrameContextPtr->GetActiveCommandBuffer();

  SwapchainImageDx12* CurrentSwapchainImage
      = (SwapchainImageDx12*)m_swapchain_->GetCurrentSwapchainImage();
  g_rhi->TransitionLayout(commandBuffer,
                          CurrentSwapchainImage->TexturePtr.get(),
                          EResourceLayout::PRESENT_SRC);

  m_commandBufferManager->ExecuteCommandList(commandBuffer);

  CurrentSwapchainImage->FenceValue
      = commandBuffer->Owner->m_fence->SignalWithNextFenceValue(
          m_commandBufferManager->GetCommandQueue().Get());

  HRESULT hr = S_OK;
  if (g_rhi->IsSupportVSync()) {
    // Wait for VSync, the application will sleep until the next VSync.
    // This is done to save cycles of frames that do not appear on the screen.
    hr = m_swapchain_->SwapChain->Present(1, 0);
  } else {
    hr = m_swapchain_->SwapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
  }

  assert(hr == S_OK);

  // CurrentFrameIndex = (CurrentFrameIndex + 1) % m_swapchain_->Images.size();
  CurrentFrameIndex = m_swapchain_->GetCurrentBackBufferIndex();
  renderFrameContextPtr->Destroy();
}

std::shared_ptr<IUniformBufferBlock> RhiDx12::CreateUniformBufferBlock(
    Name InName, LifeTimeType InLifeTimeType, size_t InSize /*= 0*/) const {
  auto uniformBufferBlockPtr
      = std::make_shared<UniformBufferBlockDx12>(InName, InLifeTimeType);
  uniformBufferBlockPtr->Init(InSize);
  return uniformBufferBlockPtr;
}

void RhiDx12::BindGraphicsShaderBindingInstances(
    const CommandBuffer*                 InCommandBuffer,
    const PipelineStateInfo*             InPiplineState,
    const ShaderBindingInstanceCombiner& InShaderBindingInstanceCombiner,
    uint32_t                             InFirstSet) const {
  // This part will use the previously created one if it is structured.
  if (InShaderBindingInstanceCombiner.m_shaderBindingInstanceArray) {
    auto commandBufferDx12 = (CommandBufferDx12*)InCommandBuffer;
    assert(commandBufferDx12);

    const ShaderBindingInstanceArray& m_shaderBindingInstanceArray
        = *(InShaderBindingInstanceCombiner.m_shaderBindingInstanceArray);
    commandBufferDx12->CommandList->SetGraphicsRootSignature(
        ShaderBindingLayoutDx12::CreateRootSignature(
            m_shaderBindingInstanceArray));

    int32_t RootParameterIndex     = 0;
    int32_t NumOfDescriptor        = 0;
    int32_t NumOfSamplerDescriptor = 0;

    // Check current online descriptor is enough to allocate descriptors, if
    // not, allocate descriptor blocks for current commandlist
    {
      for (int32_t i = 0; i < m_shaderBindingInstanceArray.NumOfData; ++i) {
        ShaderBindingInstanceDx12* Instance
            = (ShaderBindingInstanceDx12*)m_shaderBindingInstanceArray[i];
        NumOfDescriptor        += (int32_t)Instance->Descriptors.size();
        NumOfSamplerDescriptor += (int32_t)Instance->SamplerDescriptors.size();
      }

      assert(Device);

      // Check if the descriptor is sufficient, if not, allocate a new one.
      bool NeedSetDescriptorHeapsAgain = false;
      if (!commandBufferDx12->OnlineDescriptorHeap->CanAllocate(
              NumOfDescriptor)) {
        const ID3D12DescriptorHeap* PrevDescriptorHeap
            = commandBufferDx12->OnlineDescriptorHeap->GetHeap();

        commandBufferDx12->OnlineDescriptorHeap->Release();
        commandBufferDx12->OnlineDescriptorHeap
            = ((RhiDx12*)this)
                  ->OnlineDescriptorHeapManager.Alloc(
                      EDescriptorHeapTypeDX12::CBV_SRV_UAV);
        assert(commandBufferDx12->OnlineDescriptorHeap->CanAllocate(
            NumOfDescriptor));

        if (PrevDescriptorHeap
            != commandBufferDx12->OnlineDescriptorHeap->GetHeap()) {
          NeedSetDescriptorHeapsAgain = true;
        }
      }

      // Check if the sampler descriptor is sufficient, if not, allocate a new
      // one.
      const ID3D12DescriptorHeap* PrevOnlineSamplerDescriptorHeap
          = commandBufferDx12->OnlineDescriptorHeap->GetHeap();
      if (!commandBufferDx12->OnlineSamplerDescriptorHeap->CanAllocate(
              NumOfSamplerDescriptor)) {
        const ID3D12DescriptorHeap* PrevDescriptorHeap
            = commandBufferDx12->OnlineSamplerDescriptorHeap->GetHeap();

        commandBufferDx12->OnlineSamplerDescriptorHeap->Release();
        commandBufferDx12->OnlineSamplerDescriptorHeap
            = ((RhiDx12*)this)
                  ->OnlineDescriptorHeapManager.Alloc(
                      EDescriptorHeapTypeDX12::SAMPLER);
        assert(commandBufferDx12->OnlineSamplerDescriptorHeap->CanAllocate(
            NumOfSamplerDescriptor));

        if (PrevDescriptorHeap
            != commandBufferDx12->OnlineSamplerDescriptorHeap->GetHeap()) {
          NeedSetDescriptorHeapsAgain = true;
        }
      }

      // If Descriptor is newly allocated, replace OnlineDescriptorHeap with
      // SetDescriptorHeaps
      if (NeedSetDescriptorHeapsAgain) {
        assert(commandBufferDx12->OnlineDescriptorHeap
                   && commandBufferDx12->OnlineSamplerDescriptorHeap
               || (!commandBufferDx12->OnlineDescriptorHeap
                   && !commandBufferDx12->OnlineSamplerDescriptorHeap));
        if (commandBufferDx12->OnlineDescriptorHeap
            && commandBufferDx12->OnlineSamplerDescriptorHeap) {
          ID3D12DescriptorHeap* ppHeaps[]
              = {commandBufferDx12->OnlineDescriptorHeap->GetHeap(),
                 commandBufferDx12->OnlineSamplerDescriptorHeap->GetHeap()};
          commandBufferDx12->CommandList->SetDescriptorHeaps(_countof(ppHeaps),
                                                             ppHeaps);
        }
      }
    }

    const D3D12_GPU_DESCRIPTOR_HANDLE FirstGPUDescriptorHandle
        = commandBufferDx12->OnlineDescriptorHeap->GetGPUHandle(
            commandBufferDx12->OnlineDescriptorHeap->GetNumOfAllocated());
    const D3D12_GPU_DESCRIPTOR_HANDLE FirstGPUSamplerDescriptorHandle
        = commandBufferDx12->OnlineSamplerDescriptorHeap->GetGPUHandle(
            commandBufferDx12->OnlineSamplerDescriptorHeap
                ->GetNumOfAllocated());

    for (int32_t i = 0; i < m_shaderBindingInstanceArray.NumOfData; ++i) {
      ShaderBindingInstanceDx12* Instance
          = (ShaderBindingInstanceDx12*)m_shaderBindingInstanceArray[i];
      ShaderBindingLayoutDx12* Layout
          = (ShaderBindingLayoutDx12*)(Instance->ShaderBindingsLayouts);

      Instance->CopyToOnlineDescriptorHeap(commandBufferDx12);
      Instance->BindGraphics(commandBufferDx12, RootParameterIndex);
    }

    // DescriptorTable is always bound last.
    if (NumOfDescriptor > 0) {
      commandBufferDx12->CommandList->SetGraphicsRootDescriptorTable(
          RootParameterIndex++,
          FirstGPUDescriptorHandle);  // StructuredBuffer test, I will use
                                      // descriptor index based on GPU handle
                                      // start of SRVDescriptorHeap
    }

    if (NumOfSamplerDescriptor > 0) {
      commandBufferDx12->CommandList->SetGraphicsRootDescriptorTable(
          RootParameterIndex++,
          FirstGPUSamplerDescriptorHandle);  // SamplerState test
    }
  }
}

void RhiDx12::BindComputeShaderBindingInstances(
    const CommandBuffer*                 InCommandBuffer,
    const PipelineStateInfo*             InPiplineState,
    const ShaderBindingInstanceCombiner& InShaderBindingInstanceCombiner,
    uint32_t                             InFirstSet) const {
  // This part will use the previously created one if it is structured.
  if (InShaderBindingInstanceCombiner.m_shaderBindingInstanceArray) {
    auto commandBufferDx12 = (CommandBufferDx12*)InCommandBuffer;
    assert(commandBufferDx12);

    const ShaderBindingInstanceArray& m_shaderBindingInstanceArray
        = *(InShaderBindingInstanceCombiner.m_shaderBindingInstanceArray);
    commandBufferDx12->CommandList->SetComputeRootSignature(
        ShaderBindingLayoutDx12::CreateRootSignature(
            m_shaderBindingInstanceArray));

    int32_t RootParameterIndex   = 0;
    bool    HasDescriptor        = false;
    bool    HasSamplerDescriptor = false;

    const D3D12_GPU_DESCRIPTOR_HANDLE FirstGPUDescriptorHandle
        = commandBufferDx12->OnlineDescriptorHeap->GetGPUHandle(
            commandBufferDx12->OnlineDescriptorHeap->GetNumOfAllocated());
    const D3D12_GPU_DESCRIPTOR_HANDLE FirstGPUSamplerDescriptorHandle
        = commandBufferDx12->OnlineSamplerDescriptorHeap->GetGPUHandle(
            commandBufferDx12->OnlineSamplerDescriptorHeap
                ->GetNumOfAllocated());

    for (int32_t i = 0; i < m_shaderBindingInstanceArray.NumOfData; ++i) {
      ShaderBindingInstanceDx12* Instance
          = (ShaderBindingInstanceDx12*)m_shaderBindingInstanceArray[i];
      ShaderBindingLayoutDx12* Layout
          = (ShaderBindingLayoutDx12*)(Instance->ShaderBindingsLayouts);

      Instance->CopyToOnlineDescriptorHeap(commandBufferDx12);

      HasDescriptor        |= Instance->Descriptors.size() > 0;
      HasSamplerDescriptor |= Instance->SamplerDescriptors.size() > 0;

      Instance->BindCompute(commandBufferDx12, RootParameterIndex);
    }

    // DescriptorTable is always bound last.
    if (HasDescriptor) {
      commandBufferDx12->CommandList->SetComputeRootDescriptorTable(
          RootParameterIndex++,
          FirstGPUDescriptorHandle);  // StructuredBuffer test, I will use
                                      // descriptor index based on GPU handle
                                      // start of SRVDescriptorHeap
    }

    if (HasSamplerDescriptor) {
      commandBufferDx12->CommandList->SetComputeRootDescriptorTable(
          RootParameterIndex++,
          FirstGPUSamplerDescriptorHandle);  // SamplerState test
    }
  }
}

void RhiDx12::BindRaytracingShaderBindingInstances(
    const CommandBuffer*                 InCommandBuffer,
    const PipelineStateInfo*             InPiplineState,
    const ShaderBindingInstanceCombiner& InShaderBindingInstanceCombiner,
    uint32_t                             InFirstSet) const {
  BindComputeShaderBindingInstances(InCommandBuffer,
                                    InPiplineState,
                                    InShaderBindingInstanceCombiner,
                                    InFirstSet);
}

std::shared_ptr<VertexBuffer> RhiDx12::CreateVertexBuffer(
    const std::shared_ptr<VertexStreamData>& streamData) const {
  if (!streamData) {
    return nullptr;
  }

  auto vertexBufferPtr = std::make_shared<VertexBufferDx12>();
  vertexBufferPtr->Initialize(streamData);
  return vertexBufferPtr;
}

std::shared_ptr<IndexBuffer> RhiDx12::CreateIndexBuffer(
    const std::shared_ptr<IndexStreamData>& streamData) const {
  if (!streamData) {
    return nullptr;
  }

  assert(streamData);
  assert(streamData->stream);

  auto indexBufferPtr = std::make_shared<IndexBufferDx12>();
  indexBufferPtr->Initialize(streamData);
  return indexBufferPtr;
}

std::shared_ptr<Texture> RhiDx12::Create2DTexture(
    uint32_t             InWidth,
    uint32_t             InHeight,
    uint32_t             InArrayLayers,
    uint32_t             InMipLevels,
    ETextureFormat       InFormat,
    ETextureCreateFlag   InTextureCreateFlag,
    EResourceLayout      InImageLayout,
    const ImageBulkData& InImageBulkData,
    const RTClearValue&  InClearValue,
    const wchar_t*       InResourceName) const {
  auto TexturePtr = CreateTexture(InWidth,
                                  InHeight,
                                  InArrayLayers,
                                  InMipLevels,
                                  1,
                                  ETextureType::TEXTURE_2D,
                                  InFormat,
                                  InTextureCreateFlag,
                                  InImageLayout,
                                  InClearValue,
                                  InResourceName);
  if (InImageBulkData.ImageData.size() > 0) {
    // TODO: recycle temp buffer
    auto BufferPtr = CreateBuffer(InImageBulkData.ImageData.size(),
                                  0,
                                  EBufferCreateFlag::CPUAccess,
                                  EResourceLayout::READ_ONLY,
                                  &InImageBulkData.ImageData[0],
                                  InImageBulkData.ImageData.size());
    assert(BufferPtr);

    CommandBufferDx12* commandList = BeginSingleTimeCopyCommands();
    if (InImageBulkData.SubresourceFootprints.size() > 0) {
      CopyBufferToTexture(commandList->Get(),
                          BufferPtr->m_buffer->Get(),
                          TexturePtr->m_texture->Get(),
                          InImageBulkData.SubresourceFootprints);
    } else {
      CopyBufferToTexture(commandList->Get(),
                          BufferPtr->m_buffer->Get(),
                          0,
                          TexturePtr->m_texture->Get());
    }

    EndSingleTimeCopyCommands(commandList);
  }
  return TexturePtr;
}

std::shared_ptr<Texture> RhiDx12::CreateCubeTexture(
    uint32_t             InWidth,
    uint32_t             InHeight,
    uint32_t             InMipLevels,
    ETextureFormat       InFormat,
    ETextureCreateFlag   InTextureCreateFlag,
    EResourceLayout      InImageLayout,
    const ImageBulkData& InImageBulkData,
    const RTClearValue&  InClearValue,
    const wchar_t*       InResourceName) const {
  auto TexturePtr = CreateTexture(InWidth,
                                  InHeight,
                                  6,
                                  InMipLevels,
                                  1,
                                  ETextureType::TEXTURE_CUBE,
                                  InFormat,
                                  InTextureCreateFlag,
                                  InImageLayout,
                                  InClearValue,
                                  InResourceName);
  if (InImageBulkData.ImageData.size() > 0) {
    // TODO: recycle temp buffer
    auto BufferPtr = CreateBuffer(InImageBulkData.ImageData.size(),
                                  0,
                                  EBufferCreateFlag::CPUAccess,
                                  EResourceLayout::READ_ONLY,
                                  &InImageBulkData.ImageData[0],
                                  InImageBulkData.ImageData.size());
    assert(BufferPtr);

    CommandBufferDx12* commandList = BeginSingleTimeCopyCommands();
    if (InImageBulkData.SubresourceFootprints.size() > 0) {
      CopyBufferToTexture(commandList->Get(),
                          BufferPtr->m_buffer->Get(),
                          TexturePtr->m_texture->Get(),
                          InImageBulkData.SubresourceFootprints);
    } else {
      CopyBufferToTexture(commandList->Get(),
                          BufferPtr->m_buffer->Get(),
                          0,
                          TexturePtr->m_texture->Get());
    }

    EndSingleTimeCopyCommands(commandList);
  }
  return TexturePtr;
}

std::shared_ptr<ShaderBindingInstance> RhiDx12::CreateShaderBindingInstance(
    const ShaderBindingArray&       InShaderBindingArray,
    const ShaderBindingInstanceType InType) const {
  auto shaderBindingsLayout = CreateShaderBindings(InShaderBindingArray);
  assert(shaderBindingsLayout);
  return shaderBindingsLayout->CreateShaderBindingInstance(InShaderBindingArray,
                                                           InType);
}

void RhiDx12::DrawArrays(
    const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
    // EPrimitiveType                              type,
    int32_t                                    vertStartIndex,
    int32_t                                    vertCount) const {
  auto commandBufferDx12
      = (CommandBufferDx12*)InRenderFrameContext->GetActiveCommandBuffer();
  assert(commandBufferDx12);

  commandBufferDx12->CommandList->DrawInstanced(
      vertCount, 1, vertStartIndex, 0);
}

void RhiDx12::DrawArraysInstanced(
    const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
    // EPrimitiveType                              type,
    int32_t                                    vertStartIndex,
    int32_t                                    vertCount,
    int32_t                                    instanceCount) const {
  auto commandBufferDx12
      = (CommandBufferDx12*)InRenderFrameContext->GetActiveCommandBuffer();
  assert(commandBufferDx12);

  commandBufferDx12->CommandList->DrawInstanced(
      vertCount, instanceCount, vertStartIndex, 0);
}

void RhiDx12::DrawElements(
    const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
    // EPrimitiveType                              type,
    int32_t                                    elementSize,
    int32_t                                    startIndex,
    int32_t                                    indexCount) const {
  auto commandBufferDx12
      = (CommandBufferDx12*)InRenderFrameContext->GetActiveCommandBuffer();
  assert(commandBufferDx12);

  commandBufferDx12->CommandList->DrawIndexedInstanced(
      indexCount, 1, startIndex, 0, 0);
}

void RhiDx12::DrawElementsInstanced(
    const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
    // EPrimitiveType                              type,
    int32_t                                    elementSize,
    int32_t                                    startIndex,
    int32_t                                    indexCount,
    int32_t                                    instanceCount) const {
  auto commandBufferDx12
      = (CommandBufferDx12*)InRenderFrameContext->GetActiveCommandBuffer();
  assert(commandBufferDx12);

  commandBufferDx12->CommandList->DrawIndexedInstanced(
      indexCount, instanceCount, startIndex, 0, 0);
}

void RhiDx12::DrawElementsBaseVertex(
    const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
    // EPrimitiveType                              type,
    int32_t                                    elementSize,
    int32_t                                    startIndex,
    int32_t                                    indexCount,
    int32_t                                    baseVertexIndex) const {
  auto commandBufferDx12
      = (CommandBufferDx12*)InRenderFrameContext->GetActiveCommandBuffer();
  assert(commandBufferDx12);

  commandBufferDx12->CommandList->DrawIndexedInstanced(
      indexCount, 1, startIndex, baseVertexIndex, 0);
}

void RhiDx12::DrawElementsInstancedBaseVertex(
    const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
    // EPrimitiveType                              type,
    int32_t                                    elementSize,
    int32_t                                    startIndex,
    int32_t                                    indexCount,
    int32_t                                    baseVertexIndex,
    int32_t                                    instanceCount) const {
  auto commandBufferDx12
      = (CommandBufferDx12*)InRenderFrameContext->GetActiveCommandBuffer();
  assert(commandBufferDx12);

  commandBufferDx12->CommandList->DrawIndexedInstanced(
      indexCount, instanceCount, startIndex, baseVertexIndex, 0);
}

void RhiDx12::DrawIndirect(
    const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
    // EPrimitiveType                              type,
    Buffer*                                    buffer,
    int32_t                                    startIndex,
    int32_t                                    drawCount) const {
  auto commandBufferDx12
      = (CommandBufferDx12*)InRenderFrameContext->GetActiveCommandBuffer();
  assert(commandBufferDx12);

  assert(0);
}

void RhiDx12::DrawElementsIndirect(
    const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
    // EPrimitiveType                              type,
    Buffer*                                    buffer,
    int32_t                                    startIndex,
    int32_t                                    drawCount) const {
  auto commandBufferDx12
      = (CommandBufferDx12*)InRenderFrameContext->GetActiveCommandBuffer();
  assert(commandBufferDx12);

  assert(0);
}

void RhiDx12::DispatchCompute(
    const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
    uint32_t                                   numGroupsX,
    uint32_t                                   numGroupsY,
    uint32_t                                   numGroupsZ) const {
  auto commandBufferDx12
      = (CommandBufferDx12*)InRenderFrameContext->GetActiveCommandBuffer();
  assert(commandBufferDx12);
  assert(commandBufferDx12->CommandList);
  assert(numGroupsX * numGroupsY * numGroupsZ > 0);

  commandBufferDx12->CommandList->Dispatch(numGroupsX, numGroupsY, numGroupsZ);
}

std::shared_ptr<RenderTarget> RhiDx12::CreateRenderTarget(
    const RenderTargetInfo& info) const {
  const uint16_t MipLevels
      = info.IsGenerateMipmap
          ? static_cast<uint32_t>(std::floor(std::log2(
                std::max<int>(info.Extent.width(), info.Extent.height()))))
                + 1
          : 1;

  auto TexturePtr
      = CreateTexture(info.Extent.width(),
                      info.Extent.height(),
                      info.LayerCount,
                      MipLevels,
                      (uint32_t)info.SampleCount,
                      info.Type,
                      info.Format,
                      (ETextureCreateFlag::RTV | info.TextureCreateFlag),
                      EResourceLayout::UNDEFINED,
                      info.m_rtClearValue,
                      // TODO: use info.ResourceName instead of string literal
                      L"RenderTarget");

  auto RenderTargetPtr = std::make_shared<RenderTarget>();
  assert(RenderTargetPtr);
  RenderTargetPtr->Info       = info;
  RenderTargetPtr->TexturePtr = TexturePtr;

  return RenderTargetPtr;
}

bool RhiDx12::TransitionLayout_Internal(CommandBuffer*        commandBuffer,
                                        ID3D12Resource*       resource,
                                        D3D12_RESOURCE_STATES srcLayout,
                                        D3D12_RESOURCE_STATES dstLayout) const {
  assert(commandBuffer);
  assert(resource);

  if (srcLayout == dstLayout) {
    return true;
  }

  auto commandBufferDx12 = (CommandBufferDx12*)commandBuffer;

  D3D12_RESOURCE_BARRIER barrier = {};
  barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource   = resource;
  barrier.Transition.StateBefore = srcLayout;
  barrier.Transition.StateAfter  = dstLayout;
  barrier.Transition.Subresource
      = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;  // TODO: each subresource
                                                  // control
  commandBufferDx12->CommandList->ResourceBarrier(1, &barrier);

  return true;
}

bool RhiDx12::TransitionLayout(CommandBuffer*  commandBuffer,
                               Texture*        texture,
                               EResourceLayout newLayout) const {
  assert(commandBuffer);
  assert(texture);

  auto       textureDx12 = (TextureDx12*)texture;
  const auto SrcLayout   = GetDX12ResourceLayout(textureDx12->Layout);
  const auto DstLayout   = GetDX12ResourceLayout(newLayout);
  if (SrcLayout == DstLayout) {
    return true;
  }

  textureDx12->Layout = newLayout;
  return TransitionLayout_Internal(
      commandBuffer, textureDx12->m_texture->Get(), SrcLayout, DstLayout);
}

bool RhiDx12::TransitionLayoutImmediate(Texture*        texture,
                                        EResourceLayout newLayout) const {
  assert(texture);
  if (texture->GetLayout() != newLayout) {
    auto       textureDx12 = (TextureDx12*)texture;
    const auto SrcLayout   = GetDX12ResourceLayout(textureDx12->Layout);
    const auto DstLayout   = GetDX12ResourceLayout(newLayout);
    if (SrcLayout == DstLayout) {
      return true;
    }

    CommandBufferDx12* commandBuffer = BeginSingleTimeCommands();
    assert(commandBuffer);

    if (commandBuffer) {
      const bool ret = TransitionLayout_Internal(
          commandBuffer, textureDx12->m_texture->Get(), SrcLayout, DstLayout);
      textureDx12->Layout = newLayout;

      EndSingleTimeCommands(commandBuffer);
      return ret;
    }
  }

  return false;
}

void RhiDx12::UAVBarrier(CommandBuffer* commandBuffer, Texture* texture) const {
  assert(commandBuffer);
  auto commandBufferDx12 = (CommandBufferDx12*)commandBuffer;
  auto texture_dx12      = (TextureDx12*)texture;
  assert(texture_dx12->m_texture);

  D3D12_RESOURCE_BARRIER uavBarrier = {};
  uavBarrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_UAV;
  uavBarrier.UAV.pResource          = texture_dx12->m_texture->Get();
  assert(uavBarrier.UAV.pResource);
  commandBufferDx12->CommandList->ResourceBarrier(1, &uavBarrier);
}

void RhiDx12::UAVBarrier(CommandBuffer* commandBuffer, Buffer* buffer) const {
  assert(commandBuffer);
  auto commandBufferDx12 = (CommandBufferDx12*)commandBuffer;
  auto buffer_dx12       = (BufferDx12*)buffer;
  assert(buffer_dx12->m_buffer);

  D3D12_RESOURCE_BARRIER uavBarrier = {};
  uavBarrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_UAV;
  uavBarrier.UAV.pResource          = buffer_dx12->m_buffer->Get();
  assert(uavBarrier.UAV.pResource);
  commandBufferDx12->CommandList->ResourceBarrier(1, &uavBarrier);
}

void RhiDx12::UAVBarrierImmediate(Texture* texture) const {
  CommandBufferDx12* commandBuffer = BeginSingleTimeCommands();
  assert(commandBuffer);

  if (commandBuffer) {
    UAVBarrier(commandBuffer, texture);
    EndSingleTimeCommands(commandBuffer);
  }
}

void RhiDx12::UAVBarrierImmediate(Buffer* buffer) const {
  CommandBufferDx12* commandBuffer = BeginSingleTimeCommands();
  assert(commandBuffer);

  if (commandBuffer) {
    UAVBarrier(commandBuffer, buffer);
    EndSingleTimeCommands(commandBuffer);
  }
}

bool RhiDx12::TransitionLayout(CommandBuffer* commandBuffer,
                                 Buffer*        buffer,
                                 EResourceLayout newLayout) const {
  assert(commandBuffer);
  assert(buffer);
  assert(buffer->GetLayout() != newLayout);

  auto       bufferDx12 = (BufferDx12*)buffer;
  const auto SrcLayout  = GetDX12ResourceLayout(bufferDx12->Layout);
  const auto DstLayout  = GetDX12ResourceLayout(newLayout);
  if (SrcLayout == DstLayout) {
    return true;
  }

  bufferDx12->Layout = newLayout;
  return TransitionLayout_Internal(
      commandBuffer, bufferDx12->m_buffer->Get(), SrcLayout, DstLayout);
}

bool RhiDx12::TransitionLayoutImmediate(Buffer*        buffer,
                                          EResourceLayout newLayout) const {
  assert(buffer);
  if (buffer->GetLayout() != newLayout) {
    auto       bufferDx12 = (BufferDx12*)buffer;
    const auto SrcLayout   = GetDX12ResourceLayout(bufferDx12->Layout);
    const auto DstLayout   = GetDX12ResourceLayout(newLayout);
    if (SrcLayout == DstLayout) {
      return true;
    }

    CommandBufferDx12* commandBuffer = BeginSingleTimeCommands();
    assert(commandBuffer);

    if (commandBuffer) {
      const bool ret = TransitionLayout_Internal(
          commandBuffer, bufferDx12->m_buffer->Get(), SrcLayout, DstLayout);
      bufferDx12->Layout = newLayout;

      EndSingleTimeCommands(commandBuffer);
      return ret;
    }
  }

  return false;
}

// void RhiDx12::BeginDebugEvent(
//     CommandBuffer* InCommandBuffer,
//     const char*     InName,
//     const Vector4&  InColor /*= Vector4::ColorGreen*/) const {
//   CommandBufferDx12* CommandList = (CommandBufferDx12*)InCommandBuffer;
//   assert(CommandList);
//   assert(!CommandList->IsClosed);
//
//   PIXBeginEvent(CommandList->Get(),
//                 PIX_COLOR((BYTE)(255 * InColor.x),
//                           (BYTE)(255 * InColor.y),
//                           (BYTE)(255 * InColor.z)),
//                 InName);
// }
//
// void RhiDx12::EndDebugEvent(CommandBuffer* InCommandBuffer) const {
//   CommandBufferDx12* CommandList = (CommandBufferDx12*)InCommandBuffer;
//   assert(CommandList);
//   assert(!CommandList->IsClosed);
//
//   PIXEndEvent(CommandList->Get());
// }

void RhiDx12::Flush() const {
  WaitForGPU();
}

void RhiDx12::Finish() const {
  WaitForGPU();
}

std::shared_ptr<Buffer> RhiDx12::CreateStructuredBuffer(
    uint64_t          InSize,
    uint64_t          InAlignment,
    uint64_t          InStride,
    EBufferCreateFlag InBufferCreateFlag,
    EResourceLayout   InInitialState,
    const void*       InData,
    uint64_t          InDataSize
    /*, const wchar_t*    InResourceName*/) const {
  auto BufferPtr
      = CreateBuffer(InSize,
                     InStride,
                     InBufferCreateFlag,
                     InInitialState,
                     InData,
                     InDataSize,
                     // TODO: consider why using L prefix in string constant
                     TEXT(L"StructuredBuffer"));

  CreateShaderResourceView_StructuredBuffer(
      BufferPtr.get(), (uint32_t)InStride, (uint32_t)(InSize / InStride));

  if (!!(EBufferCreateFlag::UAV & InBufferCreateFlag)) {
    CreateUnorderedAccessView_StructuredBuffer(
        BufferPtr.get(), (uint32_t)InStride, (uint32_t)(InSize / InStride));
  }

  return std::shared_ptr<Buffer>(BufferPtr);
}

std::shared_ptr<Buffer> RhiDx12::CreateRawBuffer(
    uint64_t          InSize,
    uint64_t          InAlignment,
    EBufferCreateFlag InBufferCreateFlag,
    EResourceLayout   InInitialState,
    const void*       InData,
    uint64_t          InDataSize
    /*, const wchar_t*    InResourceName*/) const {
  auto BufferPtr = CreateBuffer(InSize,
                                InAlignment,
                                InBufferCreateFlag,
                                InInitialState,
                                InData,
                                InDataSize,
                                TEXT(L"RawBuffer"));

  CreateShaderResourceView_Raw(BufferPtr.get(), (uint32_t)InSize);

  if (!!(EBufferCreateFlag::UAV & InBufferCreateFlag)) {
    CreateUnorderedAccessView_Raw(BufferPtr.get(), (uint32_t)InSize);
  }

  return BufferPtr;
}

std::shared_ptr<Buffer> RhiDx12::CreateFormattedBuffer(
    uint64_t          InSize,
    uint64_t          InAlignment,
    ETextureFormat    InFormat,
    EBufferCreateFlag InBufferCreateFlag,
    EResourceLayout   InInitialState,
    const void*       InData,
    uint64_t          InDataSize
    /*, const wchar_t*    InResourceName*/) const {
  auto BufferPtr = CreateBuffer(InSize,
                                InAlignment,
                                InBufferCreateFlag,
                                InInitialState,
                                InData,
                                InDataSize,
                                TEXT(L"FormattedBuffer"));

  CreateShaderResourceView_Formatted(
      BufferPtr.get(), InFormat, (uint32_t)InSize);

  if (!!(EBufferCreateFlag::UAV & InBufferCreateFlag)) {
    CreateUnorderedAccessView_Formatted(
        BufferPtr.get(), InFormat, (uint32_t)InSize);
  }

  return BufferPtr;
}

bool RhiDx12::IsSupportVSync() const {
  return false;
}

//////////////////////////////////////////////////////////////////////////
// PlacedResourcePool
void PlacedResourcePool::Init() {
  // The allocator should be able to allocate memory larger than the
  // PlacedResourceSizeThreshold.
  assert(g_rhi_dx12->GPlacedResourceSizeThreshold
         <= MemorySize[(int32_t)EPoolSizeType::MAX - 1]);

  assert(g_rhi_dx12);
  // TODO: consider remove nested smart pointers (probably need changes in
  // DeallocatorMultiFrameResource)
  g_rhi_dx12->DeallocatorMultiFramePlacedResource.FreeDelegate = std::bind(
      &PlacedResourcePool::FreedFromPendingDelegateForCreatedResource,
      this,
      std::placeholders::_1);
}

void PlacedResourcePool::Release() {
  assert(g_rhi_dx12);
  g_rhi_dx12->DeallocatorMultiFramePlacedResource.FreeDelegate = nullptr;
}

void PlacedResourcePool::Free(const ComPtr<ID3D12Resource>& InData) {
  if (!InData) {
    return;
  }

  if (g_rhi_dx12->GIsUsePlacedResource) {
    ScopedLock s(&Lock);

    auto it_find = UsingPlacedResources.find(InData.Get());
    if (UsingPlacedResources.end() != it_find) {
      auto& PendingList = GetPendingPlacedResources(
          it_find->second.IsUploadResource, it_find->second.Size);
      PendingList.push_back(it_find->second);
      UsingPlacedResources.erase(it_find);
      return;
    }

    assert(0);  // can't reach here.
  }
}

}  // namespace game_engine