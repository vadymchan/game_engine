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
// Whether to use temporary Descriptor and Buffer only for the current frame
#define USE_ONE_FRAME_BUFFER_AND_DESCRIPTOR (USE_INLINE_DESCRIPTOR && 1)

namespace game_engine {

struct jSimpleConstantBuffer {
  math::Matrix4f M;
  int32_t        TexIndex = 0;
};

jRHI_DX12*                                         g_rhi_dx12 = nullptr;
std::unordered_map<size_t, jShaderBindingLayout*>  jRHI_DX12::ShaderBindingPool;
TResourcePool<jSamplerStateInfo_DX12, MutexRWLock> jRHI_DX12::SamplerStatePool;
TResourcePool<jRasterizationStateInfo_DX12, MutexRWLock>
    jRHI_DX12::RasterizationStatePool;
TResourcePool<jStencilOpStateInfo_DX12, MutexRWLock>
    jRHI_DX12::StencilOpStatePool;
TResourcePool<jDepthStencilStateInfo_DX12, MutexRWLock>
    jRHI_DX12::DepthStencilStatePool;
TResourcePool<jBlendingStateInfo_DX12, MutexRWLock>
    jRHI_DX12::BlendingStatePool;
TResourcePool<jPipelineStateInfo_DX12, MutexRWLock>
                                             jRHI_DX12::PipelineStatePool;
TResourcePool<jRenderPass_DX12, MutexRWLock> jRHI_DX12::RenderPassPool;

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
// jRHI_DX12
//////////////////////////////////////////////////////////////////////////
jRHI_DX12::jRHI_DX12() {
  g_rhi_dx12 = this;
}

jRHI_DX12::~jRHI_DX12() {
}

// TODO: consider whether parameter window is needed (m_window_ is presented in
// jRHI_DX12 class)
// HWND jRHI_DX12::CreateMainWindow(const std::shared_ptr<Window>& window) const
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

void jRHI_DX12::WaitForGPU() const {
  assert(Swapchain);

  auto Queue = CommandBufferManager->GetCommandQueue();
  assert(Queue);

  if (CommandBufferManager && CommandBufferManager->Fence) {
    CommandBufferManager->Fence->SignalWithNextFenceValue(Queue.Get(), true);
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

bool jRHI_DX12::init(const std::shared_ptr<Window>& window) {
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

  PlacedResourcePool.Init();
  // TODO: consider remove nested smart pointers (probably need changes in
  // jDeallocatorMultiFrameResource)
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
  CommandBufferManager = new jCommandBufferManager_DX12();
  CommandBufferManager->Initialize(Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
  CopyCommandBufferManager = new jCommandBufferManager_DX12();
  CopyCommandBufferManager->Initialize(Device, D3D12_COMMAND_LIST_TYPE_COPY);

  // 4. Heap
  RTVDescriptorHeaps.Initialize(EDescriptorHeapTypeDX12::RTV);
  DSVDescriptorHeaps.Initialize(EDescriptorHeapTypeDX12::DSV);
  DescriptorHeaps.Initialize(EDescriptorHeapTypeDX12::CBV_SRV_UAV);
  SamplerDescriptorHeaps.Initialize(EDescriptorHeapTypeDX12::SAMPLER);

  // 3. Swapchain
  // Swapchain = new jSwapchain_DX12();
  Swapchain->Create(m_window_);
  CurrentFrameIndex = Swapchain->GetCurrentBackBufferIndex();

  OneFrameUniformRingBuffers.resize(Swapchain->GetNumOfSwapchainImages());
  for (jRingBuffer_DX12*& iter : OneFrameUniformRingBuffers) {
    iter = new jRingBuffer_DX12();
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

  // QueryPoolTime = new jQueryPoolTime_DX12();
  // QueryPoolTime->Create();

  // g_ImGUI = new jImGUI_DX12();
  // g_ImGUI->Initialize((float)m_window_->getSize().width(),
  //                     (float)m_window_->getSize().height());

  //////////////////////////////////////////////////////////////////////////

  ShowWindow(m_hWnd, SW_SHOW);

#if SUPPORT_RAYTRACING
  RaytracingScene = CreateRaytracingScene();
#endif  // SUPPORT_RAYTRACING

  return true;
}

void jRHI_DX12::release() {
  WaitForGPU();

  jRHI::release();

  if (CommandBufferManager) {
    CommandBufferManager->Release();
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
  Swapchain->Release();
  // delete Swapchain;

  //////////////////////////////////////////////////////////////////////////
  // 2. Command
  delete CommandBufferManager;

  PlacedResourcePool.Release();

  //////////////////////////////////////////////////////////////////////////
  // 1. Device
  Device.Reset();
}

bool jRHI_DX12::OnHandleResized(uint32_t InWidth,
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

  Swapchain->Resize(InWidth, InHeight);
  CurrentFrameIndex = Swapchain->GetCurrentBackBufferIndex();

  return true;
}

jCommandBuffer_DX12* jRHI_DX12::BeginSingleTimeCommands() const {
  assert(CommandBufferManager);
  return CommandBufferManager->GetOrCreateCommandBuffer();
}

void jRHI_DX12::EndSingleTimeCommands(jCommandBuffer* commandBuffer) const {
  auto CommandBuffer_DX12 = (jCommandBuffer_DX12*)commandBuffer;

  assert(CommandBufferManager);
  CommandBufferManager->ExecuteCommandList(CommandBuffer_DX12, true);
  CommandBufferManager->ReturnCommandBuffer(CommandBuffer_DX12);
}

jCommandBuffer_DX12* jRHI_DX12::BeginSingleTimeCopyCommands() const {
  assert(CopyCommandBufferManager);
  return CopyCommandBufferManager->GetOrCreateCommandBuffer();
}

void jRHI_DX12::EndSingleTimeCopyCommands(
    jCommandBuffer_DX12* commandBuffer) const {
  assert(CopyCommandBufferManager);
  CopyCommandBufferManager->ExecuteCommandList(commandBuffer, true);
  CopyCommandBufferManager->ReturnCommandBuffer(commandBuffer);
}

std::shared_ptr<jTexture> jRHI_DX12::CreateTextureFromData(
    const ImageData* InImageData) const {
  assert(InImageData);

  const int32_t         MipLevel = InImageData->MipLevel;
  const EResourceLayout Layout   = EResourceLayout::GENERAL;

  std::shared_ptr<jTexture_DX12> TexturePtr;
  if (InImageData->TextureType == ETextureType::TEXTURE_CUBE) {
    TexturePtr
        = g_rhi->CreateCubeTexture<jTexture_DX12>(InImageData->Width,
                                                  InImageData->Height,
                                                  MipLevel,
                                                  InImageData->Format,
                                                  ETextureCreateFlag::UAV,
                                                  Layout,
                                                  InImageData->imageBulkData);
  } else {
    TexturePtr
        = g_rhi->Create2DTexture<jTexture_DX12>(InImageData->Width,
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

jShaderBindingLayout* jRHI_DX12::CreateShaderBindings(
    const jShaderBindingArray& InShaderBindingArray) const {
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

    auto NewShaderBinding = new jShaderBindingLayout_DX12();
    NewShaderBinding->Initialize(InShaderBindingArray);
    NewShaderBinding->Hash  = hash;
    ShaderBindingPool[hash] = NewShaderBinding;

    return NewShaderBinding;
  }
}

jSamplerStateInfo* jRHI_DX12::CreateSamplerState(
    const jSamplerStateInfo& initializer) const {
  return SamplerStatePool.GetOrCreate(initializer);
}

jRasterizationStateInfo* jRHI_DX12::CreateRasterizationState(
    const jRasterizationStateInfo& initializer) const {
  return RasterizationStatePool.GetOrCreate(initializer);
}

jStencilOpStateInfo* jRHI_DX12::CreateStencilOpStateInfo(
    const jStencilOpStateInfo& initializer) const {
  return StencilOpStatePool.GetOrCreate(initializer);
}

jDepthStencilStateInfo* jRHI_DX12::CreateDepthStencilState(
    const jDepthStencilStateInfo& initializer) const {
  return DepthStencilStatePool.GetOrCreate(initializer);
}

jBlendingStateInfo* jRHI_DX12::CreateBlendingState(
    const jBlendingStateInfo& initializer) const {
  return BlendingStatePool.GetOrCreate(initializer);
}

// TODO: consider rewriting this method for DX12 shader creation
bool jRHI_DX12::CreateShaderInternal(Shader*           OutShader,
                                     const ShaderInfo& shaderInfo) const {
  std::vector<Name> IncludeFilePaths;
  Shader*           shader_dx12 = OutShader;
  assert(shader_dx12->GetPermutationCount());
  {
    assert(!shader_dx12->CompiledShader);
    jCompiledShader_DX12* CurCompiledShader = new jCompiledShader_DX12();
    shader_dx12->CompiledShader             = CurCompiledShader;

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

jRenderPass* jRHI_DX12::GetOrCreateRenderPass(
    const std::vector<jAttachment>& colorAttachments,
    const math::Vector2Di&          offset,
    const math::Vector2Di&          extent) const {
  return RenderPassPool.GetOrCreate(
      jRenderPass_DX12(colorAttachments, offset, extent));
}

jRenderPass* jRHI_DX12::GetOrCreateRenderPass(
    const std::vector<jAttachment>& colorAttachments,
    const jAttachment&              depthAttachment,
    const math::Vector2Di&          offset,
    const math::Vector2Di&          extent) const {
  return RenderPassPool.GetOrCreate(
      jRenderPass_DX12(colorAttachments, depthAttachment, offset, extent));
}

jRenderPass* jRHI_DX12::GetOrCreateRenderPass(
    const std::vector<jAttachment>& colorAttachments,
    const jAttachment&              depthAttachment,
    const jAttachment&              colorResolveAttachment,
    const math::Vector2Di&          offset,
    const math::Vector2Di&          extent) const {
  return RenderPassPool.GetOrCreate(jRenderPass_DX12(colorAttachments,
                                                     depthAttachment,
                                                     colorResolveAttachment,
                                                     offset,
                                                     extent));
}

jRenderPass* jRHI_DX12::GetOrCreateRenderPass(
    const jRenderPassInfo& renderPassInfo,
    const math::Vector2Di& offset,
    const math::Vector2Di& extent) const {
  return RenderPassPool.GetOrCreate(
      jRenderPass_DX12(renderPassInfo, offset, extent));
}

jPipelineStateInfo* jRHI_DX12::CreatePipelineStateInfo(
    const jPipelineStateFixedInfo*   InPipelineStateFixed,
    const GraphicsPipelineShader     InShader,
    const jVertexBufferArray&        InVertexBufferArray,
    const jRenderPass*               InRenderPass,
    const jShaderBindingLayoutArray& InShaderBindingArray,
    const jPushConstant*             InPushConstant,
    int32_t                          InSubpassIndex) const {
  return PipelineStatePool.GetOrCreateMove(
      std::move(jPipelineStateInfo(InPipelineStateFixed,
                                   InShader,
                                   InVertexBufferArray,
                                   InRenderPass,
                                   InShaderBindingArray,
                                   InPushConstant,
                                   InSubpassIndex)));
}

jPipelineStateInfo* jRHI_DX12::CreateComputePipelineStateInfo(
    const Shader*                    shader,
    const jShaderBindingLayoutArray& InShaderBindingArray,
    const jPushConstant*             pushConstant) const {
  return PipelineStatePool.GetOrCreateMove(std::move(
      jPipelineStateInfo(shader, InShaderBindingArray, pushConstant)));
}

void jRHI_DX12::RemovePipelineStateInfo(size_t InHash) {
  PipelineStatePool.Release(InHash);
}

std::shared_ptr<jRenderFrameContext> jRHI_DX12::BeginRenderFrame() {
  // SCOPE_CPU_PROFILE(BeginRenderFrame);

  //////////////////////////////////////////////////////////////////////////
  // Acquire new swapchain image
  jSwapchainImage_DX12* CurrentSwapchainImage
      = (jSwapchainImage_DX12*)Swapchain->GetCurrentSwapchainImage();
  assert(CommandBufferManager);
  CommandBufferManager->Fence->WaitForFenceValue(
      CurrentSwapchainImage->FenceValue);
  //////////////////////////////////////////////////////////////////////////

  GetOneFrameUniformRingBuffer()->Reset();

  jCommandBuffer_DX12* commandBuffer
      = (jCommandBuffer_DX12*)CommandBufferManager->GetOrCreateCommandBuffer();

  auto renderFrameContextPtr
      = std::make_shared<jRenderFrameContext_DX12>(commandBuffer);
  // TODO: remove constant for UseForwardRenderer
  renderFrameContextPtr->UseForwardRenderer = true;
  renderFrameContextPtr->FrameIndex         = CurrentFrameIndex;
  renderFrameContextPtr->SceneRenderTargetPtr
      = std::make_shared<SceneRenderTarget>();
  renderFrameContextPtr->SceneRenderTargetPtr->Create(m_window_,
                                                      CurrentSwapchainImage);

  return renderFrameContextPtr;
}

void jRHI_DX12::EndRenderFrame(
    const std::shared_ptr<jRenderFrameContext>& renderFrameContextPtr) {
  // SCOPE_CPU_PROFILE(EndRenderFrame);

  jCommandBuffer_DX12* CommandBuffer
      = (jCommandBuffer_DX12*)renderFrameContextPtr->GetActiveCommandBuffer();

  jSwapchainImage_DX12* CurrentSwapchainImage
      = (jSwapchainImage_DX12*)Swapchain->GetCurrentSwapchainImage();
  g_rhi->TransitionLayout(CommandBuffer,
                          CurrentSwapchainImage->TexturePtr.get(),
                          EResourceLayout::PRESENT_SRC);

  CommandBufferManager->ExecuteCommandList(CommandBuffer);

  CurrentSwapchainImage->FenceValue
      = CommandBuffer->Owner->Fence->SignalWithNextFenceValue(
          CommandBufferManager->GetCommandQueue().Get());

  HRESULT hr = S_OK;
  if (g_rhi->IsSupportVSync()) {
    // Wait for VSync, the application will sleep until the next VSync.
    // This is done to save cycles of frames that do not appear on the screen.
    hr = Swapchain->SwapChain->Present(1, 0);
  } else {
    hr = Swapchain->SwapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
  }

  assert(hr == S_OK);

  // CurrentFrameIndex = (CurrentFrameIndex + 1) % Swapchain->Images.size();
  CurrentFrameIndex = Swapchain->GetCurrentBackBufferIndex();
  renderFrameContextPtr->Destroy();
}

std::shared_ptr<IUniformBufferBlock> jRHI_DX12::CreateUniformBufferBlock(
    Name InName, LifeTimeType InLifeTimeType, size_t InSize /*= 0*/) const {
  auto uniformBufferBlockPtr
      = std::make_shared<jUniformBufferBlock_DX12>(InName, InLifeTimeType);
  uniformBufferBlockPtr->Init(InSize);
  return uniformBufferBlockPtr;
}

void jRHI_DX12::BindGraphicsShaderBindingInstances(
    const jCommandBuffer*                InCommandBuffer,
    const jPipelineStateInfo*            InPiplineState,
    const ShaderBindingInstanceCombiner& InShaderBindingInstanceCombiner,
    uint32_t                             InFirstSet) const {
  // This part will use the previously created one if it is structured.
  if (InShaderBindingInstanceCombiner.shaderBindingInstanceArray) {
    auto CommandBuffer_DX12 = (jCommandBuffer_DX12*)InCommandBuffer;
    assert(CommandBuffer_DX12);

    const jShaderBindingInstanceArray& ShaderBindingInstanceArray
        = *(InShaderBindingInstanceCombiner.shaderBindingInstanceArray);
    CommandBuffer_DX12->CommandList->SetGraphicsRootSignature(
        jShaderBindingLayout_DX12::CreateRootSignature(
            ShaderBindingInstanceArray));

    int32_t RootParameterIndex     = 0;
    int32_t NumOfDescriptor        = 0;
    int32_t NumOfSamplerDescriptor = 0;

    // Check current online descriptor is enough to allocate descriptors, if
    // not, allocate descriptor blocks for current commandlist
    {
      for (int32_t i = 0; i < ShaderBindingInstanceArray.NumOfData; ++i) {
        jShaderBindingInstance_DX12* Instance
            = (jShaderBindingInstance_DX12*)ShaderBindingInstanceArray[i];
        NumOfDescriptor        += (int32_t)Instance->Descriptors.size();
        NumOfSamplerDescriptor += (int32_t)Instance->SamplerDescriptors.size();
      }

      assert(Device);

      // Check if the descriptor is sufficient, if not, allocate a new one.
      bool NeedSetDescriptorHeapsAgain = false;
      if (!CommandBuffer_DX12->OnlineDescriptorHeap->CanAllocate(
              NumOfDescriptor)) {
        const ID3D12DescriptorHeap* PrevDescriptorHeap
            = CommandBuffer_DX12->OnlineDescriptorHeap->GetHeap();

        CommandBuffer_DX12->OnlineDescriptorHeap->Release();
        CommandBuffer_DX12->OnlineDescriptorHeap
            = ((jRHI_DX12*)this)
                  ->OnlineDescriptorHeapManager.Alloc(
                      EDescriptorHeapTypeDX12::CBV_SRV_UAV);
        assert(CommandBuffer_DX12->OnlineDescriptorHeap->CanAllocate(
            NumOfDescriptor));

        if (PrevDescriptorHeap
            != CommandBuffer_DX12->OnlineDescriptorHeap->GetHeap()) {
          NeedSetDescriptorHeapsAgain = true;
        }
      }

      // Check if the sampler descriptor is sufficient, if not, allocate a new
      // one.
      const ID3D12DescriptorHeap* PrevOnlineSamplerDescriptorHeap
          = CommandBuffer_DX12->OnlineDescriptorHeap->GetHeap();
      if (!CommandBuffer_DX12->OnlineSamplerDescriptorHeap->CanAllocate(
              NumOfSamplerDescriptor)) {
        const ID3D12DescriptorHeap* PrevDescriptorHeap
            = CommandBuffer_DX12->OnlineSamplerDescriptorHeap->GetHeap();

        CommandBuffer_DX12->OnlineSamplerDescriptorHeap->Release();
        CommandBuffer_DX12->OnlineSamplerDescriptorHeap
            = ((jRHI_DX12*)this)
                  ->OnlineDescriptorHeapManager.Alloc(
                      EDescriptorHeapTypeDX12::SAMPLER);
        assert(CommandBuffer_DX12->OnlineSamplerDescriptorHeap->CanAllocate(
            NumOfSamplerDescriptor));

        if (PrevDescriptorHeap
            != CommandBuffer_DX12->OnlineSamplerDescriptorHeap->GetHeap()) {
          NeedSetDescriptorHeapsAgain = true;
        }
      }

      // If Descriptor is newly allocated, replace OnlineDescriptorHeap with
      // SetDescriptorHeaps
      if (NeedSetDescriptorHeapsAgain) {
        assert(CommandBuffer_DX12->OnlineDescriptorHeap
                   && CommandBuffer_DX12->OnlineSamplerDescriptorHeap
               || (!CommandBuffer_DX12->OnlineDescriptorHeap
                   && !CommandBuffer_DX12->OnlineSamplerDescriptorHeap));
        if (CommandBuffer_DX12->OnlineDescriptorHeap
            && CommandBuffer_DX12->OnlineSamplerDescriptorHeap) {
          ID3D12DescriptorHeap* ppHeaps[]
              = {CommandBuffer_DX12->OnlineDescriptorHeap->GetHeap(),
                 CommandBuffer_DX12->OnlineSamplerDescriptorHeap->GetHeap()};
          CommandBuffer_DX12->CommandList->SetDescriptorHeaps(_countof(ppHeaps),
                                                              ppHeaps);
        }
      }
    }

    const D3D12_GPU_DESCRIPTOR_HANDLE FirstGPUDescriptorHandle
        = CommandBuffer_DX12->OnlineDescriptorHeap->GetGPUHandle(
            CommandBuffer_DX12->OnlineDescriptorHeap->GetNumOfAllocated());
    const D3D12_GPU_DESCRIPTOR_HANDLE FirstGPUSamplerDescriptorHandle
        = CommandBuffer_DX12->OnlineSamplerDescriptorHeap->GetGPUHandle(
            CommandBuffer_DX12->OnlineSamplerDescriptorHeap
                ->GetNumOfAllocated());

    for (int32_t i = 0; i < ShaderBindingInstanceArray.NumOfData; ++i) {
      jShaderBindingInstance_DX12* Instance
          = (jShaderBindingInstance_DX12*)ShaderBindingInstanceArray[i];
      jShaderBindingLayout_DX12* Layout
          = (jShaderBindingLayout_DX12*)(Instance->ShaderBindingsLayouts);

      Instance->CopyToOnlineDescriptorHeap(CommandBuffer_DX12);
      Instance->BindGraphics(CommandBuffer_DX12, RootParameterIndex);
    }

    // DescriptorTable is always bound last.
    if (NumOfDescriptor > 0) {
      CommandBuffer_DX12->CommandList->SetGraphicsRootDescriptorTable(
          RootParameterIndex++,
          FirstGPUDescriptorHandle);  // StructuredBuffer test, I will use
                                      // descriptor index based on GPU handle
                                      // start of SRVDescriptorHeap
    }

    if (NumOfSamplerDescriptor > 0) {
      CommandBuffer_DX12->CommandList->SetGraphicsRootDescriptorTable(
          RootParameterIndex++,
          FirstGPUSamplerDescriptorHandle);  // SamplerState test
    }
  }
}

void jRHI_DX12::BindComputeShaderBindingInstances(
    const jCommandBuffer*                InCommandBuffer,
    const jPipelineStateInfo*            InPiplineState,
    const ShaderBindingInstanceCombiner& InShaderBindingInstanceCombiner,
    uint32_t                             InFirstSet) const {
  // This part will use the previously created one if it is structured.
  if (InShaderBindingInstanceCombiner.shaderBindingInstanceArray) {
    auto CommandBuffer_DX12 = (jCommandBuffer_DX12*)InCommandBuffer;
    assert(CommandBuffer_DX12);

    const jShaderBindingInstanceArray& ShaderBindingInstanceArray
        = *(InShaderBindingInstanceCombiner.shaderBindingInstanceArray);
    CommandBuffer_DX12->CommandList->SetComputeRootSignature(
        jShaderBindingLayout_DX12::CreateRootSignature(
            ShaderBindingInstanceArray));

    int32_t RootParameterIndex   = 0;
    bool    HasDescriptor        = false;
    bool    HasSamplerDescriptor = false;

    const D3D12_GPU_DESCRIPTOR_HANDLE FirstGPUDescriptorHandle
        = CommandBuffer_DX12->OnlineDescriptorHeap->GetGPUHandle(
            CommandBuffer_DX12->OnlineDescriptorHeap->GetNumOfAllocated());
    const D3D12_GPU_DESCRIPTOR_HANDLE FirstGPUSamplerDescriptorHandle
        = CommandBuffer_DX12->OnlineSamplerDescriptorHeap->GetGPUHandle(
            CommandBuffer_DX12->OnlineSamplerDescriptorHeap
                ->GetNumOfAllocated());

    for (int32_t i = 0; i < ShaderBindingInstanceArray.NumOfData; ++i) {
      jShaderBindingInstance_DX12* Instance
          = (jShaderBindingInstance_DX12*)ShaderBindingInstanceArray[i];
      jShaderBindingLayout_DX12* Layout
          = (jShaderBindingLayout_DX12*)(Instance->ShaderBindingsLayouts);

      Instance->CopyToOnlineDescriptorHeap(CommandBuffer_DX12);

      HasDescriptor        |= Instance->Descriptors.size() > 0;
      HasSamplerDescriptor |= Instance->SamplerDescriptors.size() > 0;

      Instance->BindCompute(CommandBuffer_DX12, RootParameterIndex);
    }

    // DescriptorTable is always bound last.
    if (HasDescriptor) {
      CommandBuffer_DX12->CommandList->SetComputeRootDescriptorTable(
          RootParameterIndex++,
          FirstGPUDescriptorHandle);  // StructuredBuffer test, I will use
                                      // descriptor index based on GPU handle
                                      // start of SRVDescriptorHeap
    }

    if (HasSamplerDescriptor) {
      CommandBuffer_DX12->CommandList->SetComputeRootDescriptorTable(
          RootParameterIndex++,
          FirstGPUSamplerDescriptorHandle);  // SamplerState test
    }
  }
}

void jRHI_DX12::BindRaytracingShaderBindingInstances(
    const jCommandBuffer*                InCommandBuffer,
    const jPipelineStateInfo*            InPiplineState,
    const ShaderBindingInstanceCombiner& InShaderBindingInstanceCombiner,
    uint32_t                             InFirstSet) const {
  BindComputeShaderBindingInstances(InCommandBuffer,
                                    InPiplineState,
                                    InShaderBindingInstanceCombiner,
                                    InFirstSet);
}

std::shared_ptr<jVertexBuffer> jRHI_DX12::CreateVertexBuffer(
    const std::shared_ptr<VertexStreamData>& streamData) const {
  if (!streamData) {
    return nullptr;
  }

  auto vertexBufferPtr = std::make_shared<jVertexBuffer_DX12>();
  vertexBufferPtr->Initialize(streamData);
  return vertexBufferPtr;
}

std::shared_ptr<jIndexBuffer> jRHI_DX12::CreateIndexBuffer(
    const std::shared_ptr<IndexStreamData>& streamData) const {
  if (!streamData) {
    return nullptr;
  }

  assert(streamData);
  assert(streamData->stream);

  auto indexBufferPtr = std::make_shared<jIndexBuffer_DX12>();
  indexBufferPtr->Initialize(streamData);
  return indexBufferPtr;
}

std::shared_ptr<jTexture> jRHI_DX12::Create2DTexture(
    uint32_t             InWidth,
    uint32_t             InHeight,
    uint32_t             InArrayLayers,
    uint32_t             InMipLevels,
    ETextureFormat       InFormat,
    ETextureCreateFlag   InTextureCreateFlag,
    EResourceLayout      InImageLayout,
    const ImageBulkData& InImageBulkData,
    const jRTClearValue& InClearValue,
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

    jCommandBuffer_DX12* commandList = BeginSingleTimeCopyCommands();
    if (InImageBulkData.SubresourceFootprints.size() > 0) {
      CopyBufferToTexture(commandList->Get(),
                          BufferPtr->Buffer->Get(),
                          TexturePtr->Texture->Get(),
                          InImageBulkData.SubresourceFootprints);
    } else {
      CopyBufferToTexture(commandList->Get(),
                          BufferPtr->Buffer->Get(),
                          0,
                          TexturePtr->Texture->Get());
    }

    EndSingleTimeCopyCommands(commandList);
  }
  return TexturePtr;
}

std::shared_ptr<jTexture> jRHI_DX12::CreateCubeTexture(
    uint32_t             InWidth,
    uint32_t             InHeight,
    uint32_t             InMipLevels,
    ETextureFormat       InFormat,
    ETextureCreateFlag   InTextureCreateFlag,
    EResourceLayout      InImageLayout,
    const ImageBulkData& InImageBulkData,
    const jRTClearValue& InClearValue,
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

    jCommandBuffer_DX12* commandList = BeginSingleTimeCopyCommands();
    if (InImageBulkData.SubresourceFootprints.size() > 0) {
      CopyBufferToTexture(commandList->Get(),
                          BufferPtr->Buffer->Get(),
                          TexturePtr->Texture->Get(),
                          InImageBulkData.SubresourceFootprints);
    } else {
      CopyBufferToTexture(commandList->Get(),
                          BufferPtr->Buffer->Get(),
                          0,
                          TexturePtr->Texture->Get());
    }

    EndSingleTimeCopyCommands(commandList);
  }
  return TexturePtr;
}

std::shared_ptr<jShaderBindingInstance> jRHI_DX12::CreateShaderBindingInstance(
    const jShaderBindingArray&       InShaderBindingArray,
    const jShaderBindingInstanceType InType) const {
  auto shaderBindingsLayout = CreateShaderBindings(InShaderBindingArray);
  assert(shaderBindingsLayout);
  return shaderBindingsLayout->CreateShaderBindingInstance(InShaderBindingArray,
                                                           InType);
}

void jRHI_DX12::DrawArrays(
    const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
    // EPrimitiveType                              type,
    int32_t                                     vertStartIndex,
    int32_t                                     vertCount) const {
  auto CommandBuffer_DX12
      = (jCommandBuffer_DX12*)InRenderFrameContext->GetActiveCommandBuffer();
  assert(CommandBuffer_DX12);

  CommandBuffer_DX12->CommandList->DrawInstanced(
      vertCount, 1, vertStartIndex, 0);
}

void jRHI_DX12::DrawArraysInstanced(
    const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
    // EPrimitiveType                              type,
    int32_t                                     vertStartIndex,
    int32_t                                     vertCount,
    int32_t                                     instanceCount) const {
  auto CommandBuffer_DX12
      = (jCommandBuffer_DX12*)InRenderFrameContext->GetActiveCommandBuffer();
  assert(CommandBuffer_DX12);

  CommandBuffer_DX12->CommandList->DrawInstanced(
      vertCount, instanceCount, vertStartIndex, 0);
}

void jRHI_DX12::DrawElements(
    const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
    // EPrimitiveType                              type,
    int32_t                                     elementSize,
    int32_t                                     startIndex,
    int32_t                                     indexCount) const {
  auto CommandBuffer_DX12
      = (jCommandBuffer_DX12*)InRenderFrameContext->GetActiveCommandBuffer();
  assert(CommandBuffer_DX12);

  CommandBuffer_DX12->CommandList->DrawIndexedInstanced(
      indexCount, 1, startIndex, 0, 0);
}

void jRHI_DX12::DrawElementsInstanced(
    const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
    // EPrimitiveType                              type,
    int32_t                                     elementSize,
    int32_t                                     startIndex,
    int32_t                                     indexCount,
    int32_t                                     instanceCount) const {
  auto CommandBuffer_DX12
      = (jCommandBuffer_DX12*)InRenderFrameContext->GetActiveCommandBuffer();
  assert(CommandBuffer_DX12);

  CommandBuffer_DX12->CommandList->DrawIndexedInstanced(
      indexCount, instanceCount, startIndex, 0, 0);
}

void jRHI_DX12::DrawElementsBaseVertex(
    const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
    // EPrimitiveType                              type,
    int32_t                                     elementSize,
    int32_t                                     startIndex,
    int32_t                                     indexCount,
    int32_t                                     baseVertexIndex) const {
  auto CommandBuffer_DX12
      = (jCommandBuffer_DX12*)InRenderFrameContext->GetActiveCommandBuffer();
  assert(CommandBuffer_DX12);

  CommandBuffer_DX12->CommandList->DrawIndexedInstanced(
      indexCount, 1, startIndex, baseVertexIndex, 0);
}

void jRHI_DX12::DrawElementsInstancedBaseVertex(
    const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
    // EPrimitiveType                              type,
    int32_t                                     elementSize,
    int32_t                                     startIndex,
    int32_t                                     indexCount,
    int32_t                                     baseVertexIndex,
    int32_t                                     instanceCount) const {
  auto CommandBuffer_DX12
      = (jCommandBuffer_DX12*)InRenderFrameContext->GetActiveCommandBuffer();
  assert(CommandBuffer_DX12);

  CommandBuffer_DX12->CommandList->DrawIndexedInstanced(
      indexCount, instanceCount, startIndex, baseVertexIndex, 0);
}

void jRHI_DX12::DrawIndirect(
    const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
    // EPrimitiveType                              type,
    jBuffer*                                    buffer,
    int32_t                                     startIndex,
    int32_t                                     drawCount) const {
  auto CommandBuffer_DX12
      = (jCommandBuffer_DX12*)InRenderFrameContext->GetActiveCommandBuffer();
  assert(CommandBuffer_DX12);

  assert(0);
}

void jRHI_DX12::DrawElementsIndirect(
    const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
    // EPrimitiveType                              type,
    jBuffer*                                    buffer,
    int32_t                                     startIndex,
    int32_t                                     drawCount) const {
  auto CommandBuffer_DX12
      = (jCommandBuffer_DX12*)InRenderFrameContext->GetActiveCommandBuffer();
  assert(CommandBuffer_DX12);

  assert(0);
}

void jRHI_DX12::DispatchCompute(
    const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
    uint32_t                                    numGroupsX,
    uint32_t                                    numGroupsY,
    uint32_t                                    numGroupsZ) const {
  auto CommandBuffer_DX12
      = (jCommandBuffer_DX12*)InRenderFrameContext->GetActiveCommandBuffer();
  assert(CommandBuffer_DX12);
  assert(CommandBuffer_DX12->CommandList);
  assert(numGroupsX * numGroupsY * numGroupsZ > 0);

  CommandBuffer_DX12->CommandList->Dispatch(numGroupsX, numGroupsY, numGroupsZ);
}

std::shared_ptr<jRenderTarget> jRHI_DX12::CreateRenderTarget(
    const jRenderTargetInfo& info) const {
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
                      info.RTClearValue,
                      // TODO: use info.ResourceName instead of string literal
                      L"RenderTarget");

  auto RenderTargetPtr = std::make_shared<jRenderTarget>();
  assert(RenderTargetPtr);
  RenderTargetPtr->Info       = info;
  RenderTargetPtr->TexturePtr = TexturePtr;

  return RenderTargetPtr;
}

bool jRHI_DX12::TransitionLayout_Internal(
    jCommandBuffer*       commandBuffer,
    ID3D12Resource*       resource,
    D3D12_RESOURCE_STATES srcLayout,
    D3D12_RESOURCE_STATES dstLayout) const {
  assert(commandBuffer);
  assert(resource);

  if (srcLayout == dstLayout) {
    return true;
  }

  auto CommandBuffer_DX12 = (jCommandBuffer_DX12*)commandBuffer;

  D3D12_RESOURCE_BARRIER barrier = {};
  barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource   = resource;
  barrier.Transition.StateBefore = srcLayout;
  barrier.Transition.StateAfter  = dstLayout;
  barrier.Transition.Subresource
      = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;  // TODO: each subresource
                                                  // control
  CommandBuffer_DX12->CommandList->ResourceBarrier(1, &barrier);

  return true;
}

bool jRHI_DX12::TransitionLayout(jCommandBuffer* commandBuffer,
                                 jTexture*       texture,
                                 EResourceLayout newLayout) const {
  assert(commandBuffer);
  assert(texture);

  auto       Texture_DX12 = (jTexture_DX12*)texture;
  const auto SrcLayout    = GetDX12ResourceLayout(Texture_DX12->Layout);
  const auto DstLayout    = GetDX12ResourceLayout(newLayout);
  if (SrcLayout == DstLayout) {
    return true;
  }

  Texture_DX12->Layout = newLayout;
  return TransitionLayout_Internal(
      commandBuffer, Texture_DX12->Texture->Get(), SrcLayout, DstLayout);
}

bool jRHI_DX12::TransitionLayoutImmediate(jTexture*       texture,
                                          EResourceLayout newLayout) const {
  assert(texture);
  if (texture->GetLayout() != newLayout) {
    auto       Texture_DX12 = (jTexture_DX12*)texture;
    const auto SrcLayout    = GetDX12ResourceLayout(Texture_DX12->Layout);
    const auto DstLayout    = GetDX12ResourceLayout(newLayout);
    if (SrcLayout == DstLayout) {
      return true;
    }

    jCommandBuffer_DX12* commandBuffer = BeginSingleTimeCommands();
    assert(commandBuffer);

    if (commandBuffer) {
      const bool ret = TransitionLayout_Internal(
          commandBuffer, Texture_DX12->Texture->Get(), SrcLayout, DstLayout);
      Texture_DX12->Layout = newLayout;

      EndSingleTimeCommands(commandBuffer);
      return ret;
    }
  }

  return false;
}

void jRHI_DX12::UAVBarrier(jCommandBuffer* commandBuffer,
                           jTexture*       texture) const {
  assert(commandBuffer);
  auto commandBuffer_DX12 = (jCommandBuffer_DX12*)commandBuffer;
  auto texture_dx12       = (jTexture_DX12*)texture;
  assert(texture_dx12->Texture);

  D3D12_RESOURCE_BARRIER uavBarrier = {};
  uavBarrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_UAV;
  uavBarrier.UAV.pResource          = texture_dx12->Texture->Get();
  assert(uavBarrier.UAV.pResource);
  commandBuffer_DX12->CommandList->ResourceBarrier(1, &uavBarrier);
}

void jRHI_DX12::UAVBarrier(jCommandBuffer* commandBuffer,
                           jBuffer*        buffer) const {
  assert(commandBuffer);
  auto commandBuffer_DX12 = (jCommandBuffer_DX12*)commandBuffer;
  auto buffer_dx12        = (jBuffer_DX12*)buffer;
  assert(buffer_dx12->Buffer);

  D3D12_RESOURCE_BARRIER uavBarrier = {};
  uavBarrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_UAV;
  uavBarrier.UAV.pResource          = buffer_dx12->Buffer->Get();
  assert(uavBarrier.UAV.pResource);
  commandBuffer_DX12->CommandList->ResourceBarrier(1, &uavBarrier);
}

void jRHI_DX12::UAVBarrierImmediate(jTexture* texture) const {
  jCommandBuffer_DX12* commandBuffer = BeginSingleTimeCommands();
  assert(commandBuffer);

  if (commandBuffer) {
    UAVBarrier(commandBuffer, texture);
    EndSingleTimeCommands(commandBuffer);
  }
}

void jRHI_DX12::UAVBarrierImmediate(jBuffer* buffer) const {
  jCommandBuffer_DX12* commandBuffer = BeginSingleTimeCommands();
  assert(commandBuffer);

  if (commandBuffer) {
    UAVBarrier(commandBuffer, buffer);
    EndSingleTimeCommands(commandBuffer);
  }
}

bool jRHI_DX12::TransitionLayout(jCommandBuffer* commandBuffer,
                                 jBuffer*        buffer,
                                 EResourceLayout newLayout) const {
  assert(commandBuffer);
  assert(buffer);
  assert(buffer->GetLayout() != newLayout);

  auto       Buffer_DX12 = (jBuffer_DX12*)buffer;
  const auto SrcLayout   = GetDX12ResourceLayout(Buffer_DX12->Layout);
  const auto DstLayout   = GetDX12ResourceLayout(newLayout);
  if (SrcLayout == DstLayout) {
    return true;
  }

  Buffer_DX12->Layout = newLayout;
  return TransitionLayout_Internal(
      commandBuffer, Buffer_DX12->Buffer->Get(), SrcLayout, DstLayout);
}

bool jRHI_DX12::TransitionLayoutImmediate(jBuffer*        buffer,
                                          EResourceLayout newLayout) const {
  assert(buffer);
  if (buffer->GetLayout() != newLayout) {
    auto       Buffer_DX12 = (jBuffer_DX12*)buffer;
    const auto SrcLayout   = GetDX12ResourceLayout(Buffer_DX12->Layout);
    const auto DstLayout   = GetDX12ResourceLayout(newLayout);
    if (SrcLayout == DstLayout) {
      return true;
    }

    jCommandBuffer_DX12* commandBuffer = BeginSingleTimeCommands();
    assert(commandBuffer);

    if (commandBuffer) {
      const bool ret = TransitionLayout_Internal(
          commandBuffer, Buffer_DX12->Buffer->Get(), SrcLayout, DstLayout);
      Buffer_DX12->Layout = newLayout;

      EndSingleTimeCommands(commandBuffer);
      return ret;
    }
  }

  return false;
}

// void jRHI_DX12::BeginDebugEvent(
//     jCommandBuffer* InCommandBuffer,
//     const char*     InName,
//     const Vector4&  InColor /*= Vector4::ColorGreen*/) const {
//   jCommandBuffer_DX12* CommandList = (jCommandBuffer_DX12*)InCommandBuffer;
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
// void jRHI_DX12::EndDebugEvent(jCommandBuffer* InCommandBuffer) const {
//   jCommandBuffer_DX12* CommandList = (jCommandBuffer_DX12*)InCommandBuffer;
//   assert(CommandList);
//   assert(!CommandList->IsClosed);
//
//   PIXEndEvent(CommandList->Get());
// }

void jRHI_DX12::Flush() const {
  WaitForGPU();
}

void jRHI_DX12::Finish() const {
  WaitForGPU();
}

std::shared_ptr<jBuffer> jRHI_DX12::CreateStructuredBuffer(
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

  return std::shared_ptr<jBuffer>(BufferPtr);
}

std::shared_ptr<jBuffer> jRHI_DX12::CreateRawBuffer(
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

std::shared_ptr<jBuffer> jRHI_DX12::CreateFormattedBuffer(
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

bool jRHI_DX12::IsSupportVSync() const {
  return false;
}

//////////////////////////////////////////////////////////////////////////
// jPlacedResourcePool
void jPlacedResourcePool::Init() {
  // The allocator should be able to allocate memory larger than the
  // PlacedResourceSizeThreshold.
  assert(g_rhi_dx12->GPlacedResourceSizeThreshold
         <= MemorySize[(int32_t)EPoolSizeType::MAX - 1]);

  assert(g_rhi_dx12);
  // TODO: consider remove nested smart pointers (probably need changes in
  // jDeallocatorMultiFrameResource)
  g_rhi_dx12->DeallocatorMultiFramePlacedResource.FreeDelegate = std::bind(
      &jPlacedResourcePool::FreedFromPendingDelegateForCreatedResource,
      this,
      std::placeholders::_1);
}

void jPlacedResourcePool::Release() {
  assert(g_rhi_dx12);
  g_rhi_dx12->DeallocatorMultiFramePlacedResource.FreeDelegate = nullptr;
}

void jPlacedResourcePool::Free(const ComPtr<ID3D12Resource>& InData) {
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