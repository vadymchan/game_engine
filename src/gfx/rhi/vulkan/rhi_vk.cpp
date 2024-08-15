#include "gfx/rhi/vulkan/rhi_vk.h"

#include "gfx/rhi/frame_buffer_pool.h"
#include "gfx/rhi/render_target_pool.h"
#include "utils/memory/align.h"

namespace game_engine {

RhiVk* g_rhi_vk = nullptr;

TResourcePool<RenderPassVk, MutexRWLock>        RhiVk::RenderPassPool;
TResourcePool<PipelineStateInfoVk, MutexRWLock> RhiVk::PipelineStatePool;
TResourcePool<SamplerStateInfoVk, MutexRWLock>  RhiVk::SamplerStatePool;
TResourcePool<RasterizationStateInfoVk, MutexRWLock>
                                                 RhiVk::RasterizationStatePool;
TResourcePool<StencilOpStateInfoVk, MutexRWLock> RhiVk::StencilOpStatePool;
TResourcePool<DepthStencilStateInfoVk, MutexRWLock>
                                                RhiVk::DepthStencilStatePool;
TResourcePool<BlendingStateInfoVk, MutexRWLock> RhiVk::BlendingStatePool;
std::unordered_map<uint64_t, ShaderBindingLayoutVk*> RhiVk::ShaderBindingPool;

// TODO: consider whether need it
struct FrameBufferVk : public FrameBuffer {
  bool                                    IsInitialized = false;
  std::vector<std::shared_ptr<Texture> > AllTextures;

  virtual Texture* GetTexture(std::int32_t index = 0) const {
    return AllTextures[index].get();
  }
};

RhiVk::RhiVk() {
  g_rhi_vk = this;
}

bool RhiVk::init(const std::shared_ptr<Window>& window) {
  m_window_ = window;

  // Vulkan Instance
  {
    // TODO: move parameters to config file
    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "Vulkan";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "No Engine";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // extentions
    unsigned int extensionCount = 0;
    if (!SDL_Vulkan_GetInstanceExtensions(
            m_window_->getNativeWindowHandle(), &extensionCount, nullptr)) {
      GlobalLogger::Log(LogLevel::Error,
                        "Failed to get Vulkan instance extension count: "
                            + std::string(SDL_GetError()));
      return false;
    }

    std::vector<const char*> extensions(extensionCount);
    if (!SDL_Vulkan_GetInstanceExtensions(m_window_->getNativeWindowHandle(),
                                          &extensionCount,
                                          extensions.data())) {
      GlobalLogger::Log(LogLevel::Error,
                        "Failed to get Vulkan instance extensions: "
                            + std::string(SDL_GetError()));
      return false;
    }

    GlobalLogger::Log(LogLevel::Info, "Available Vulkan instance extensions:");
    for (const auto& extension : extensions) {
      GlobalLogger::Log(LogLevel::Info, "- " + std::string(extension));
    }

#ifdef ENABLE_VALIDATION_LAYERS
    extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    //-----------------------------------

    // validation layers
#ifdef ENABLE_VALIDATION_LAYERS

    const std::vector<const char*> validationLayers
        = {"VK_LAYER_KHRONOS_validation"};

    auto CheckValidationLayerSupport = [&]() {
      uint32_t layerCount;
      vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

      std::vector<VkLayerProperties> availableLayers(layerCount);
      vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

      for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
          if (strcmp(layerName, layerProperties.layerName) == 0) {
            layerFound = true;
            break;
          }
        }

        if (!layerFound) {
          return false;
        }
      }

      return true;
    };

    bool layersAvailable = CheckValidationLayerSupport();

    if (layersAvailable) {
      createInfo.enabledLayerCount
          = static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();

      // during instance creation
      VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
      populateDebugMessengerCreateInfo(debugCreateInfo);

      createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }

#else
    createInfo.enabledLayerCount = 0;
    createInfo.pNext             = nullptr;
#endif

    //-----------------------------------

    VkResult createInstanceResult
        = vkCreateInstance(&createInfo, nullptr, &m_instance_);

    if (createInstanceResult != VK_SUCCESS) {
      GlobalLogger::Log(LogLevel::Error, "Failed to create Vulkan instance");
      return false;
    }

    // during application runtime
#ifdef ENABLE_VALIDATION_LAYERS

    if (layersAvailable) {
      VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
      populateDebugMessengerCreateInfo(messengerCreateInfo);
      messengerCreateInfo.pUserData = nullptr;

      auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
          vkGetInstanceProcAddr(m_instance_, "vkCreateDebugUtilsMessengerEXT"));

      if (func != nullptr) {
        VkResult result = func(
            m_instance_, &messengerCreateInfo, nullptr, &m_debugMessenger_);
        if (result != VK_SUCCESS) {
          GlobalLogger::Log(LogLevel::Error,
                            "Failed to create debug messenger");
        }
      } else {
        GlobalLogger::Log(
            LogLevel::Error,
            "Failed to get vkCreateDebugUtilsMessengerEXT function pointer");
      }
    }

#endif  // ENABLE_VALIDATION_LAYERS
  }

  // WSI
  {
    auto isSurfaceCreated = SDL_Vulkan_CreateSurface(
        m_window_->getNativeWindowHandle(), m_instance_, &m_surface_);
    if (!isSurfaceCreated) {
      GlobalLogger::Log(LogLevel::Error, "Failed to create Vulkan surface");
      return false;
    }
  }

  // Physical Device
  {
    std::uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance_, &deviceCount, nullptr);
    if (!(deviceCount > 0)) {
      GlobalLogger::Log(LogLevel::Error, "No Vulkan physical devices found");
      return false;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance_, &deviceCount, devices.data());

    for (const auto& device : devices) {
      if (isDeviceSuitable(device, m_surface_)) {
        m_physicalDevice_ = device;
        break;
      }
    }

    if (m_physicalDevice_ == VK_NULL_HANDLE) {
      GlobalLogger::Log(LogLevel::Error,
                        "No suitable Vulkan physical device found");
      return false;
    }
    vkGetPhysicalDeviceProperties(m_physicalDevice_, &m_deviceProperties_);
    // TODO: DeviceProperties2 for rtx + acceleration structures
  }

  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(
      m_physicalDevice_, nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(
      m_physicalDevice_, nullptr, &extensionCount, availableExtensions.data());

  // Logical Device
  {
    QueueFamilyIndices indices
        = FindQueueFamilies(m_physicalDevice_, m_surface_);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.GraphicsFamily.value(),
                                              indices.PresentFamily.value(),
                                              indices.ComputeFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
      VkDeviceQueueCreateInfo queueCreateInfo = {};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount       = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.emplace_back(queueCreateInfo);
    }

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType              = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount
        = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    VkPhysicalDeviceFeatures deviceFeatures = {};  // Enable features as needed
    // Set to true to allow VkSampler to use Anisotropy
    deviceFeatures.samplerAnisotropy = true;
    // Enable Sample shading (also mitigates aliasing inside the texture)
    deviceFeatures.sampleRateShading = true;
    deviceFeatures.multiDrawIndirect = true;
    deviceFeatures.geometryShader    = true;
    deviceFeatures.depthClamp        = true;
    // m_buffer device address requires the 64-bit integer feature to be enabled
    deviceFeatures.shaderInt64  = true;
    createInfo.pEnabledFeatures = &deviceFeatures;

    // TODO: not used now
    // VkPhysicalDeviceDescriptorIndexingFeaturesEXT
    //    physicalDeviceDescriptorIndexingFeatures{};
    // physicalDeviceDescriptorIndexingFeatures.sType
    //    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
    // physicalDeviceDescriptorIndexingFeatures
    //    .shaderInputAttachmentArrayDynamicIndexing
    //    = true;
    // physicalDeviceDescriptorIndexingFeatures
    //    .shaderUniformTexelBufferArrayDynamicIndexing
    //    = true;
    // physicalDeviceDescriptorIndexingFeatures
    //    .shaderStorageTexelBufferArrayDynamicIndexing
    //    = true;
    // physicalDeviceDescriptorIndexingFeatures
    //    .shaderUniformBufferArrayNonUniformIndexing
    //    = true;
    // physicalDeviceDescriptorIndexingFeatures
    //    .shaderSampledImageArrayNonUniformIndexing
    //    = true;
    // physicalDeviceDescriptorIndexingFeatures
    //    .shaderStorageBufferArrayNonUniformIndexing
    //    = true;
    // physicalDeviceDescriptorIndexingFeatures
    //    .shaderStorageImageArrayNonUniformIndexing
    //    = true;
    // physicalDeviceDescriptorIndexingFeatures
    //    .shaderInputAttachmentArrayNonUniformIndexing
    //    = true;
    // physicalDeviceDescriptorIndexingFeatures
    //    .shaderUniformTexelBufferArrayNonUniformIndexing
    //    = true;
    // physicalDeviceDescriptorIndexingFeatures
    //    .shaderStorageTexelBufferArrayNonUniformIndexing
    //    = true;
    // physicalDeviceDescriptorIndexingFeatures
    //    .descriptorBindingUniformBufferUpdateAfterBind
    //    = true;
    // physicalDeviceDescriptorIndexingFeatures
    //    .descriptorBindingSampledImageUpdateAfterBind
    //    = true;
    // physicalDeviceDescriptorIndexingFeatures
    //    .descriptorBindingStorageImageUpdateAfterBind
    //    = true;
    // physicalDeviceDescriptorIndexingFeatures
    //    .descriptorBindingStorageBufferUpdateAfterBind
    //    = true;
    // physicalDeviceDescriptorIndexingFeatures
    //    .descriptorBindingUniformTexelBufferUpdateAfterBind
    //    = true;
    // physicalDeviceDescriptorIndexingFeatures
    //    .descriptorBindingStorageTexelBufferUpdateAfterBind
    //    = true;
    // physicalDeviceDescriptorIndexingFeatures
    //    .descriptorBindingUpdateUnusedWhilePending
    //    = true;
    // physicalDeviceDescriptorIndexingFeatures.descriptorBindingPartiallyBound
    //    = true;
    // physicalDeviceDescriptorIndexingFeatures
    //    .descriptorBindingVariableDescriptorCount
    //    = true;
    // physicalDeviceDescriptorIndexingFeatures.runtimeDescriptorArray = true;
    // physicalDeviceDescriptorIndexingFeatures.pNext = (void*)createInfo.pNext;
    // createInfo.pNext = &physicalDeviceDescriptorIndexingFeatures;

    //// Enable features required for ray tracing using feature chaining via
    /// pNext
    // VkPhysicalDeviceBufferDeviceAddressFeatures
    //     enabledBufferDeviceAddresFeatures{};
    // enabledBufferDeviceAddresFeatures.sType
    //     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    // enabledBufferDeviceAddresFeatures.bufferDeviceAddress = true;
    // enabledBufferDeviceAddresFeatures.pNext = (void*)createInfo.pNext;
    // createInfo.pNext = &enabledBufferDeviceAddresFeatures;

    // VkPhysicalDeviceRayTracingPipelineFeaturesKHR
    //     enabledRayTracingPipelineFeatures{};
    // enabledRayTracingPipelineFeatures.sType
    //     =
    //     VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    // enabledRayTracingPipelineFeatures.rayTracingPipeline = true;
    // enabledRayTracingPipelineFeatures.pNext = (void*)createInfo.pNext;
    // createInfo.pNext = &enabledRayTracingPipelineFeatures;

    // VkPhysicalDeviceAccelerationStructureFeaturesKHR
    //     enabledAccelerationStructureFeatures{};
    // enabledAccelerationStructureFeatures.sType
    //     =
    //     VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    // enabledAccelerationStructureFeatures.accelerationStructure = true;
    // enabledAccelerationStructureFeatures.pNext = (void*)createInfo.pNext;
    // createInfo.pNext = &enabledAccelerationStructureFeatures;

    // VkPhysicalDeviceRayQueryFeaturesKHR enabledRayQueryFeatures{};
    // enabledRayQueryFeatures.sType
    //     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
    // enabledRayQueryFeatures.rayQuery = true;
    // enabledRayQueryFeatures.pNext    = (void*)createInfo.pNext;
    // createInfo.pNext                 = &enabledRayQueryFeatures;

    //// Added CustomBorderColor features
    // VkPhysicalDeviceCustomBorderColorFeaturesEXT
    //     enabledCustomBorderColorFeaturesEXT{};
    // enabledCustomBorderColorFeaturesEXT.sType
    //     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT;
    // enabledCustomBorderColorFeaturesEXT.customBorderColors             =
    // true; enabledCustomBorderColorFeaturesEXT.customBorderColorWithoutFormat
    // = true; enabledCustomBorderColorFeaturesEXT.pNext =
    // (void*)createInfo.pNext; createInfo.pNext =
    // &enabledCustomBorderColorFeaturesEXT;

    std::vector<const char*> deviceExtensions
        = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    // TODO: no need for now
    // Add debug marker extension when it can be enabled
    // for (auto& extension : availableExtensions) {
    //  if (!strcmp(extension.extensionName,
    //              VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
    //    DeviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    //    IsSupportDebugMarker = true;
    //    break;
    //  }
    //}

    createInfo.enabledExtensionCount
        = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

#ifdef ENABLE_VALIDATION_LAYERS
    // redundant (already enabled in vkInstance)
    // createInfo.enabledLayerCount
    //    = static_cast<uint32_t>(validationLayers.size());
    // createInfo.ppEnabledLayerNames
    //    = validationLayers.data();
#else
    createInfo.enabledLayerCount = 0;
#endif

    if (vkCreateDevice(m_physicalDevice_, &createInfo, nullptr, &m_device_)
        != VK_SUCCESS) {
      GlobalLogger::Log(LogLevel::Error,
                        "Failed to create Vulkan logical device");
      return false;
    }

    m_graphicsQueue_.QueueIndex = indices.GraphicsFamily.value();
    m_presentQueue_.QueueIndex  = indices.PresentFamily.value();
    m_computeQueue_.QueueIndex  = indices.ComputeFamily.value();
    vkGetDeviceQueue(
        m_device_, m_graphicsQueue_.QueueIndex, 0, &m_graphicsQueue_.Queue);
    vkGetDeviceQueue(
        m_device_, m_presentQueue_.QueueIndex, 0, &m_presentQueue_.Queue);
    vkGetDeviceQueue(
        m_device_, m_computeQueue_.QueueIndex, 0, &m_computeQueue_.Queue);
  }

  //  	// Check if the Vsync support
  //  {
  //    assert(PhysicalDevice);
  //    assert(Surface);
  //    SwapChainSupportDetails swapChainSupport
  //        = QuerySwapChainSupport(PhysicalDevice, Surface);
  //
  //    for (auto it : swapChainSupport.PresentModes) {
  //      if (it == VK_PRESENT_MODE_IMMEDIATE_KHR) { // TODO: seems that i need
  //      to use VK_PRESENT_MODE_FIFO_KHR
  //        GRHISupportVsync = true;
  //        break;
  //      }
  //    }
  //  }

  m_memoryPool_ = new MemoryPoolVk();

  // TODO: currently not needed
  // Get vkCmdBindShadingRateImageNV function pointer for VRS
  //  vkCmdBindShadingRateImageNV
  //      = reinterpret_cast<PFN_vkCmdBindShadingRateImageNV>(
  //          vkGetDeviceProcAddr(Device, "vkCmdBindShadingRateImageNV"));
  //
  //  // Get debug marker function pointer
  //  if (IsSupportDebugMarker) {
  //    vkDebugMarkerSetObjectTag
  //        = (PFN_vkDebugMarkerSetObjectTagEXT)vkGetDeviceProcAddr(
  //            Device, "vkDebugMarkerSetObjectTagEXT");
  //    vkDebugMarkerSetObjectName
  //        = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(
  //            Device, "vkDebugMarkerSetObjectNameEXT");
  //    vkCmdDebugMarkerBegin =
  //    (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(
  //        Device, "vkCmdDebugMarkerBeginEXT");
  //    vkCmdDebugMarkerEnd = (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(
  //        Device, "vkCmdDebugMarkerEndEXT");
  //    vkCmdDebugMarkerInsert =
  //    (PFN_vkCmdDebugMarkerInsertEXT)vkGetDeviceProcAddr(
  //        Device, "vkCmdDebugMarkerInsertEXT");
  //  }
  //
  // #if SUPPORT_RAYTRACING
  //  vkGetBufferDeviceAddressKHR
  //      = (PFN_vkGetBufferDeviceAddressKHR)vkGetDeviceProcAddr(
  //          Device, "vkGetBufferDeviceAddressKHR");
  //  vkCmdBuildAccelerationStructuresKHR
  //      = (PFN_vkCmdBuildAccelerationStructuresKHR)vkGetDeviceProcAddr(
  //          Device, "vkCmdBuildAccelerationStructuresKHR");
  //  vkBuildAccelerationStructuresKHR
  //      = (PFN_vkBuildAccelerationStructuresKHR)vkGetDeviceProcAddr(
  //          Device, "vkBuildAccelerationStructuresKHR");
  //  vkCreateAccelerationStructureKHR
  //      = (PFN_vkCreateAccelerationStructureKHR)vkGetDeviceProcAddr(
  //          Device, "vkCreateAccelerationStructureKHR");
  //  vkDestroyAccelerationStructureKHR
  //      = (PFN_vkDestroyAccelerationStructureKHR)vkGetDeviceProcAddr(
  //          Device, "vkDestroyAccelerationStructureKHR");
  //  vkGetAccelerationStructureBuildSizesKHR
  //      = (PFN_vkGetAccelerationStructureBuildSizesKHR)vkGetDeviceProcAddr(
  //          Device, "vkGetAccelerationStructureBuildSizesKHR");
  //  vkGetAccelerationStructureDeviceAddressKHR
  //      = (PFN_vkGetAccelerationStructureDeviceAddressKHR)vkGetDeviceProcAddr(
  //          Device, "vkGetAccelerationStructureDeviceAddressKHR");
  //  vkCmdTraceRaysKHR
  //      = (PFN_vkCmdTraceRaysKHR)vkGetDeviceProcAddr(Device,
  //      "vkCmdTraceRaysKHR");
  //  vkGetRayTracingShaderGroupHandlesKHR
  //      = (PFN_vkGetRayTracingShaderGroupHandlesKHR)vkGetDeviceProcAddr(
  //          Device, "vkGetRayTracingShaderGroupHandlesKHR");
  //  vkCreateRayTracingPipelinesKHR
  //      = (PFN_vkCreateRayTracingPipelinesKHR)vkGetDeviceProcAddr(
  //          Device, "vkCreateRayTracingPipelinesKHR");
  // #endif  // SUPPORT_RAYTRACING

  // Swapchain
  m_swapchain_->Create(window);

  m_fenceManager_ = new FenceManagerVk();

  m_commandBufferManager_ = new CommandBufferManagerVk();
  m_commandBufferManager_->CreatePool(m_computeQueue_.QueueIndex);

  // Pipeline cache
  VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
  pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  if (vkCreatePipelineCache(
          m_device_, &pipelineCacheCreateInfo, nullptr, &PipelineCache)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error,
                      "Failed to create Vulkan pipeline cache");
    return false;
  }

  // Ring buffer
  OneFrameUniformRingBuffers.resize(m_swapchain_->GetNumOfSwapchainImages());
  for (auto& iter : OneFrameUniformRingBuffers) {
    iter = new RingBufferVk();
    iter->Create(
        EVulkanBufferBits::UNIFORM_BUFFER,
        16 * 1024 * 1024,
        (uint32_t)m_deviceProperties_.limits.minUniformBufferOffsetAlignment);
  }

  // TODO: not used for now
  // SSBORingBuffers.resize(m_swapchain_->GetNumOfSwapchainImages());
  // for (auto& iter : SSBORingBuffers) {
  //  iter = new RingBufferVk();
  //  iter->Create(
  //      EVulkanBufferBits::STORAGE_BUFFER,
  //      16 * 1024 * 1024,
  //      (uint32_t)m_deviceProperties_.limits.minUniformBufferOffsetAlignment);
  //}

  DescriptorPoolsSingleFrame.resize(m_swapchain_->GetNumOfSwapchainImages());
  for (auto& iter : DescriptorPoolsSingleFrame) {
    iter = new DescriptorPoolVk();
    iter->Create(10'000);
  }

  DescriptorPoolMultiFrame = new DescriptorPoolVk();
  DescriptorPoolMultiFrame->Create(10'000);

  return true;
}

void RhiVk::release() {
  Flush();

  RHI::release();  // TODO: consider whether need to remove

  RenderPassPool.Release();
  SamplerStatePool.Release();
  RasterizationStatePool.Release();
  StencilOpStatePool.Release();
  DepthStencilStatePool.Release();
  BlendingStatePool.Release();
  PipelineStatePool.Release();

  FrameBufferPool::Release();
  RenderTargetPool::Release();

  // TODO: consider removing 
  // delete m_swapchain_;
  // m_swapchain_ = nullptr;
  // TODO: consider using destructor
  m_swapchain_->Release();

  delete m_commandBufferManager_;
  m_commandBufferManager_ = nullptr;

  ShaderBindingLayoutVk::ClearPipelineLayout();

  {
    ScopeWriteLock s(&ShaderBindingPoolLock);
    for (auto& iter : ShaderBindingPool) {
      delete iter.second;
    }
    ShaderBindingPool.clear();
  }

  for (auto& iter : OneFrameUniformRingBuffers) {
    delete iter;
  }
  OneFrameUniformRingBuffers.clear();

  for (auto& iter : DescriptorPoolsSingleFrame) {
    delete iter;
  }
  DescriptorPoolsSingleFrame.clear();

  delete DescriptorPoolMultiFrame;

  vkDestroyPipelineCache(m_device_, PipelineCache, nullptr);

  delete m_fenceManager_;
  m_fenceManager_ = nullptr;

  m_semaphoreManager_.Release();

  if (m_device_) {
    vkDestroyDevice(m_device_, nullptr);
    m_device_ = nullptr;
  }

  if (m_surface_) {
    vkDestroySurfaceKHR(m_instance_, m_surface_, nullptr);
    m_surface_ = nullptr;
  }

  if (m_debugMessenger_) {
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(m_instance_, "vkDestroyDebugUtilsMessengerEXT"));
    if (func != nullptr) {
      func(m_instance_, m_debugMessenger_, nullptr);
    }
    m_debugMessenger_ = nullptr;
  }

  if (m_instance_) {
    vkDestroyInstance(m_instance_, nullptr);
    m_instance_ = nullptr;
  }
}

std::shared_ptr<VertexBuffer> RhiVk::CreateVertexBuffer(
    const std::shared_ptr<VertexStreamData>& streamData) const {
  if (!streamData) {
    return nullptr;
  }

  auto vertexBufferPtr = std::make_shared<VertexBufferVk>();
  vertexBufferPtr->Initialize(streamData);
  return vertexBufferPtr;
}

std::shared_ptr<IndexBuffer> RhiVk::CreateIndexBuffer(
    const std::shared_ptr<IndexStreamData>& streamData) const {
  if (!streamData) {
    return nullptr;
  }

  assert(streamData);
  assert(streamData->stream);
  auto indexBufferPtr = std::make_shared<IndexBufferVk>();
  indexBufferPtr->Initialize(streamData);
  return indexBufferPtr;
}

std::shared_ptr<Texture> RhiVk::CreateTextureFromData(
    const ImageData* InImageData) const {
  assert(InImageData);

  int32_t MipLevel = InImageData->MipLevel;
  if ((MipLevel == 1) && (InImageData->CreateMipmapIfPossible)) {
    MipLevel = TextureVk::GetMipLevels(InImageData->Width, InImageData->Height);
  }

  auto stagingBufferPtr = CreateBuffer(
      EVulkanBufferBits::TRANSFER_SRC,
      EVulkanMemoryBits::HOST_VISIBLE | EVulkanMemoryBits::HOST_COHERENT,
      InImageData->imageBulkData.ImageData.size(),
      EResourceLayout::TRANSFER_SRC);

  stagingBufferPtr->UpdateBuffer(&InImageData->imageBulkData.ImageData[0],
                                 InImageData->imageBulkData.ImageData.size());

  std::shared_ptr<TextureVk> TexturePtr;
  if (InImageData->TextureType == ETextureType::TEXTURE_CUBE) {
    TexturePtr = g_rhi->CreateCubeTexture<TextureVk>(
        (uint32_t)InImageData->Width,
        (uint32_t)InImageData->Height,
        MipLevel,
        InImageData->Format,
        ETextureCreateFlag::TransferSrc | ETextureCreateFlag::TransferDst
            | ETextureCreateFlag::UAV,
        EResourceLayout::SHADER_READ_ONLY,
        InImageData->imageBulkData);
    if (!TexturePtr) {
      GlobalLogger::Log(LogLevel::Error, "Failed to create cube texture");
      return nullptr;
    }
  } else {
    TexturePtr = g_rhi->Create2DTexture<TextureVk>(
        (uint32_t)InImageData->Width,
        (uint32_t)InImageData->Height,
        (uint32_t)InImageData->LayerCount,
        MipLevel,
        InImageData->Format,
        ETextureCreateFlag::TransferSrc | ETextureCreateFlag::TransferDst
            | ETextureCreateFlag::UAV,
        EResourceLayout::SHADER_READ_ONLY,
        InImageData->imageBulkData);
    if (!TexturePtr) {
      GlobalLogger::Log(LogLevel::Error, "Failed to create 2D texture");
      return nullptr;
    }
  }

  return TexturePtr;
}

bool RhiVk::CreateShaderInternal(Shader*           OutShader,
                                 const ShaderInfo& shaderInfo) const {
  auto CreateShaderModule
      = [m_device_ = this->m_device_](
            const std::vector<uint32_t>& code) -> VkShaderModule {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(uint32_t);

    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule = {};
    assert(vkCreateShaderModule(m_device_, &createInfo, nullptr, &shaderModule)
           == VK_SUCCESS);

    return shaderModule;
  };

  auto LoadShaderFromSRV = [](const char* fileName) -> VkShaderModule {
    std::ifstream is(fileName, std::ios::binary | std::ios::in | std::ios::ate);

    if (is.is_open()) {
      size_t size = is.tellg();
      is.seekg(0, std::ios::beg);
      char* shaderCode = new char[size];
      is.read(shaderCode, size);
      is.close();

      assert(size > 0);

      VkShaderModule           shaderModule = nullptr;
      VkShaderModuleCreateInfo moduleCreateInfo{};
      moduleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      moduleCreateInfo.codeSize = size;
      moduleCreateInfo.pCode    = (uint32_t*)shaderCode;

      assert(VK_SUCCESS
             != vkCreateShaderModule(
                 g_rhi_vk->m_device_, &moduleCreateInfo, NULL, &shaderModule));
      delete[] shaderCode;

      return shaderModule;
    }
    return nullptr;
  };

  std::vector<Name> IncludeFilePaths;
  Shader*           shader_vk = OutShader;
  assert(shader_vk->GetPermutationCount());
  {
    assert(!shader_vk->m_compiledShader);
    // TODO: check for memory leak, use smart pointer
    CompiledShaderVk* CurCompiledShader = new CompiledShaderVk();
    shader_vk->m_compiledShader           = CurCompiledShader;

    shader_vk->SetPermutationId(shaderInfo.GetPermutationId());

    std::string PermutationDefines;
    shader_vk->GetPermutationDefines(PermutationDefines);

    VkShaderModule shaderModule{};

    const bool isSpirv
        = !!strstr(shaderInfo.GetShaderFilepath().ToStr(), ".spv");
    if (isSpirv) {
      shaderModule = LoadShaderFromSRV(shaderInfo.GetShaderFilepath().ToStr());
    } else {
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
        startOfInclude      += includePrefixLength;
        size_t endOfInclude  = ShaderText.find("\"", startOfInclude);

        std::filesystem::path includeFilePathObj(
            ShaderText.substr(startOfInclude, endOfInclude - startOfInclude));
        if (includeFilePathObj.is_relative()) {
          includeFilePathObj
              = shaderFilePath.parent_path() / includeFilePathObj;
        }
        std::string includeFilepath = includeFilePathObj.string();

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

      // const wchar_t* ShadingModel = nullptr;
      // switch (shaderInfo.GetShaderType()) {
      //   case EShaderAccessStageFlag::VERTEX:
      //     ShadingModel = TEXT("vs_6_6");
      //     break;
      //   case EShaderAccessStageFlag::GEOMETRY:
      //     ShadingModel = TEXT("gs_6_6");
      //     break;
      //   case EShaderAccessStageFlag::FRAGMENT:
      //     ShadingModel = TEXT("ps_6_6");
      //     break;
      //   case EShaderAccessStageFlag::COMPUTE:
      //     ShadingModel = TEXT("cs_6_6");
      //     break;
      //   case EShaderAccessStageFlag::RAYTRACING:
      //   case EShaderAccessStageFlag::RAYTRACING_RAYGEN:
      //   case EShaderAccessStageFlag::RAYTRACING_MISS:
      //   case EShaderAccessStageFlag::RAYTRACING_CLOSESTHIT:
      //   case EShaderAccessStageFlag::RAYTRACING_ANYHIT:
      //     ShadingModel = TEXT("lib_6_6");
      //     break;
      //   default:
      //     assert(0);
      //     break;
      // }

      // const std::wstring EntryPoint
      //     = ConvertToWchar(shaderInfo.GetEntryPoint());
      // auto ShaderBlob = ShaderCompilerDx12::Get().Compile(
      //     ShaderText.c_str(),
      //     (uint32_t)ShaderText.length(),
      //     ShadingModel,
      //     EntryPoint.c_str(),
      //     false,
      //     {TEXT("-spirv"),
      //      TEXT("-fspv-target-env=vulkan1.1spirv1.4"),
      //      TEXT("-fvk-use-scalar-layout"),
      //      TEXT("-fspv-extension=SPV_EXT_descriptor_indexing"),
      //      TEXT("-fspv-extension=SPV_KHR_ray_tracing"),
      //      TEXT("-fspv-extension=SPV_KHR_ray_query"),
      //      TEXT("-fspv-extension=SPV_EXT_shader_viewport_index_layer")});

      // std::vector<uint8_t> SpirvCode;
      // if (ShaderBlob->GetBufferSize() > 0) {
      //   SpirvCode.resize(ShaderBlob->GetBufferSize());
      //   memcpy(SpirvCode.data(),
      //          ShaderBlob->GetBufferPointer(),
      //          ShaderBlob->GetBufferSize());
      // }

      auto SpirvCode = SpirvUtil::compileHlslCodeToSpirv(
          ShaderText,
          SpirvUtil::getShadercShaderKind(shaderInfo.GetShaderType()),
          shaderInfo.GetEntryPoint().ToStr());

      {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = SpirvCode.size() * sizeof(uint32_t);

        createInfo.pCode = reinterpret_cast<const uint32_t*>(SpirvCode.data());

        assert(
            vkCreateShaderModule(m_device_, &createInfo, nullptr, &shaderModule)
            == VK_SUCCESS);

        // return shaderModule;
      }
    }

    if (!shaderModule) {
      return false;
    }

    VkShaderStageFlagBits ShaderFlagBits;
    // TODO: consider using GetVulkanShaderAccessFlags function
    switch (shaderInfo.GetShaderType()) {
      case EShaderAccessStageFlag::VERTEX:
        ShaderFlagBits = VK_SHADER_STAGE_VERTEX_BIT;
        break;
      case EShaderAccessStageFlag::GEOMETRY:
        ShaderFlagBits = VK_SHADER_STAGE_GEOMETRY_BIT;
        break;
      case EShaderAccessStageFlag::FRAGMENT:
        ShaderFlagBits = VK_SHADER_STAGE_FRAGMENT_BIT;
        break;
      case EShaderAccessStageFlag::COMPUTE:
        ShaderFlagBits = VK_SHADER_STAGE_COMPUTE_BIT;
        break;
      case EShaderAccessStageFlag::RAYTRACING:
        ShaderFlagBits = VK_SHADER_STAGE_ALL;
        break;
      case EShaderAccessStageFlag::RAYTRACING_RAYGEN:
        ShaderFlagBits = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        break;
      case EShaderAccessStageFlag::RAYTRACING_MISS:
        ShaderFlagBits = VK_SHADER_STAGE_MISS_BIT_KHR;
        break;
      case EShaderAccessStageFlag::RAYTRACING_CLOSESTHIT:
        ShaderFlagBits = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        break;
      case EShaderAccessStageFlag::RAYTRACING_ANYHIT:
        ShaderFlagBits = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        break;
      default:
        assert(0);
        break;
    }

    CurCompiledShader->ShaderStage.sType
        = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    CurCompiledShader->ShaderStage.stage  = ShaderFlagBits;
    CurCompiledShader->ShaderStage.module = shaderModule;
    CurCompiledShader->ShaderStage.pName  = shaderInfo.GetEntryPoint().ToStr();
  }
  shader_vk->shaderInfo = shaderInfo;
  shader_vk->shaderInfo.SetIncludeShaderFilePaths(IncludeFilePaths);

  return true;
}

FrameBuffer* RhiVk::CreateFrameBuffer(const FrameBufferInfo& info) const {
  const VkFormat textureFormat      = GetVulkanTextureFormat(info.Format);
  const bool     hasDepthAttachment = IsDepthFormat(info.Format);

  const VkImageUsageFlagBits ImageUsageFlagBit
      = hasDepthAttachment ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                           : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  const VkImageAspectFlagBits ImageAspectFlagBit
      = hasDepthAttachment ? VK_IMAGE_ASPECT_DEPTH_BIT
                           : VK_IMAGE_ASPECT_COLOR_BIT;

  const VkImageTiling TilingMode = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;

  const int32_t mipLevels = info.IsGenerateMipmap ? TextureVk::GetMipLevels(
                                info.Extent.width(), info.Extent.height())
                                                  : 1;

  assert(info.SampleCount >= 1);

  std::shared_ptr<TextureVk> TexturePtr;
  switch (info.TextureType) {
    case ETextureType::TEXTURE_2D:
      TexturePtr = game_engine::Create2DTexture(
          info.Extent.width(),
          info.Extent.height(),
          mipLevels,
          (VkSampleCountFlagBits)info.SampleCount,
          textureFormat,
          TilingMode,
          ImageUsageFlagBit,
          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
          VkImageCreateFlagBits(),
          VK_IMAGE_LAYOUT_UNDEFINED);
      if (!TexturePtr) {
        GlobalLogger::Log(LogLevel::Error, "Failed to create 2D texture");
      }
      break;
    case ETextureType::TEXTURE_2D_ARRAY:
      TexturePtr = game_engine::CreateTexture2DArray(
          info.Extent.width(),
          info.Extent.height(),
          info.LayerCount,
          mipLevels,
          (VkSampleCountFlagBits)info.SampleCount,
          textureFormat,
          TilingMode,
          ImageUsageFlagBit,
          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
          VkImageCreateFlagBits(),
          VK_IMAGE_LAYOUT_UNDEFINED);
      if (!TexturePtr) {
        GlobalLogger::Log(LogLevel::Error, "Failed to create 2D array texture");
      }
      break;
    case ETextureType::TEXTURE_CUBE:
      TexturePtr = game_engine::CreateCubeTexture(
          info.Extent.width(),
          info.Extent.height(),
          mipLevels,
          (VkSampleCountFlagBits)info.SampleCount,
          textureFormat,
          TilingMode,
          ImageUsageFlagBit,
          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
          VkImageCreateFlagBits(),
          VK_IMAGE_LAYOUT_UNDEFINED);
      if (!TexturePtr) {
        GlobalLogger::Log(LogLevel::Error, "Failed to create cube texture");
      }
      break;
    default:
      GlobalLogger::Log(LogLevel::Error,
                        "Unsupported texture type in FramebufferPool");
      return nullptr;
  }

  auto fb_vk  = new FrameBufferVk();
  fb_vk->Info = info;
  fb_vk->Textures.push_back(TexturePtr);
  fb_vk->AllTextures.push_back(TexturePtr);  // TODO: consider to remove

  return fb_vk;
}

std::shared_ptr<RenderTarget> RhiVk::CreateRenderTarget(
    const RenderTargetInfo& info) const {
  const VkFormat textureFormat      = GetVulkanTextureFormat(info.Format);
  const bool     hasDepthAttachment = IsDepthFormat(info.Format);
  const int32_t  mipLevels
      = (info.SampleCount > EMSAASamples::COUNT_1 || !info.IsGenerateMipmap)
          ? 1
          : TextureVk::GetMipLevels(info.Extent.width(), info.Extent.height());
  assert((int32_t)info.SampleCount >= 1);

  // VkImageView                    imageViewUAV = nullptr;
  // std::map<int32_t, VkImageView> imageViewForMipMap;
  // std::map<int32_t, VkImageView> imageViewForMipMapUAV;
  std::shared_ptr<TextureVk> TexturePtr;

  ETextureCreateFlag TextureCreateFlag{};
  if (hasDepthAttachment) {
    TextureCreateFlag |= ETextureCreateFlag::DSV;
  } else {
    TextureCreateFlag |= ETextureCreateFlag::RTV | ETextureCreateFlag::UAV;
  }

  if (info.IsUseAsSubpassInput) {
    TextureCreateFlag |= ETextureCreateFlag::SubpassInput;
  }

  if (info.IsMemoryless) {
    TextureCreateFlag |= ETextureCreateFlag::Memoryless;
  }

  const VkImageAspectFlags ImageAspectFlag = hasDepthAttachment
                                               ? VK_IMAGE_ASPECT_DEPTH_BIT
                                               : VK_IMAGE_ASPECT_COLOR_BIT;

  switch (info.Type) {
    case ETextureType::TEXTURE_2D:
      TexturePtr = g_rhi->Create2DTexture<TextureVk>(info.Extent.width(),
                                                     info.Extent.height(),
                                                     1,
                                                     mipLevels,
                                                     info.Format,
                                                     TextureCreateFlag,
                                                     EResourceLayout::GENERAL);
      // TODO: remove (currently not needed)
      /*imageViewForMipMap[0] = TexturePtr->imageView;

        assert(mipLevels > 0);
        for (int32_t i = 1; i < mipLevels; ++i) {
          imageViewForMipMap[i] = CreateTextureViewForSpecificMipMap(
              TexturePtr->image, textureFormat, ImageAspectFlag, i);
        }*/
      break;
    case ETextureType::TEXTURE_2D_ARRAY:
      TexturePtr = g_rhi->Create2DTexture<TextureVk>(info.Extent.width(),
                                                     info.Extent.height(),
                                                     info.LayerCount,
                                                     mipLevels,
                                                     info.Format,
                                                     TextureCreateFlag,
                                                     EResourceLayout::GENERAL);
      // TODO: remove (currently not needed)
      /*imageViewForMipMap[0] = TexturePtr->imageView;

      assert(mipLevels > 0);
      for (int32_t i = 1; i < mipLevels; ++i) {
        imageViewForMipMap[i]
            = CreateTexture2DArrayViewForSpecificMipMap(TexturePtr->image,
                                                        info.LayerCount,
                                                        textureFormat,
                                                        ImageAspectFlag,
                                                        i);
      }*/
      break;
    case ETextureType::TEXTURE_CUBE:
      assert(info.LayerCount == 6);
      TexturePtr
          = g_rhi->CreateCubeTexture<TextureVk>(info.Extent.width(),
                                                info.Extent.height(),
                                                mipLevels,
                                                info.Format,
                                                TextureCreateFlag,
                                                EResourceLayout::GENERAL);
      // TODO: remove (currently not needed)
      // Create for Shader Resource (TextureCube)
      //{
      //  imageViewForMipMap[0] = TexturePtr->imageView;

      //  assert(mipLevels > 0);
      //  for (int32_t i = 1; i < mipLevels; ++i) {
      //    imageViewForMipMap[i] = CreateTextureCubeViewForSpecificMipMap(
      //        TexturePtr->image, textureFormat, ImageAspectFlag, i);
      //  }
      //}

      // Create for UAV (writing compute shader resource) (Texture2DArray)
      //{
      //  imageViewUAV             = TexturePtr->imageView;
      //  imageViewForMipMapUAV[0] = TexturePtr->imageView;

      //  assert(mipLevels > 0);
      //  for (int32_t i = 1; i < mipLevels; ++i) {
      //    imageViewForMipMapUAV[i]
      //        = CreateTexture2DArrayViewForSpecificMipMap(TexturePtr->image,
      //                                                    info.LayerCount,
      //                                                    textureFormat,
      //                                                    ImageAspectFlag,
      //                                                    i);
      //  }
      //}
      break;
    default:
      GlobalLogger::Log(LogLevel::Error,
                        "Unsupported texture type in FramebufferPool");
      return nullptr;
  }

  // TexturePtr->ViewForMipMap    = imageViewForMipMap;
  // TexturePtr->ViewUAV          = imageViewUAV;
  // TexturePtr->ViewUAVForMipMap = imageViewForMipMapUAV;

  auto RenderTargetPtr        = std::make_shared<RenderTarget>();
  RenderTargetPtr->Info       = info;
  RenderTargetPtr->TexturePtr = TexturePtr;

  return RenderTargetPtr;
}

// TODO: remove commented code (old version)
//  ///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  const VkFormat          textureFormat      = info.Format;
//  const bool              hasDepthAttachment = IsDepthFormat(info.Format);
//  const VkImageUsageFlags AllowingUsageFlag
//      = info.IsMemoryless ? (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
//                             | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
//                             | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)
//                          : VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
//
//  const VkImageUsageFlags ImageUsageFlag
//      = AllowingUsageFlag
//      & ((hasDepthAttachment ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
//                             : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
//                                   | VK_IMAGE_USAGE_STORAGE_BIT)
//         | VK_IMAGE_USAGE_SAMPLED_BIT
//         | (info.IsUseAsSubpassInput ? VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT :
//         0) | (info.IsMemoryless ? VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT :
//         0));
//  const VkImageAspectFlags ImageAspectFlag = hasDepthAttachment
//                                               ? VK_IMAGE_ASPECT_DEPTH_BIT
//                                               : VK_IMAGE_ASPECT_COLOR_BIT;
//
//  // const VkImageTiling TilingMode = IsMobile ?
//  // VkImageTiling::VK_IMAGE_TILING_OPTIMAL :
//  // VkImageTiling::VK_IMAGE_TILING_LINEAR;
//  const VkImageTiling TilingMode = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
//
//  const int32_t mipLevels
//      = (info.SampleCount > VK_SAMPLE_COUNT_1_BIT || !info.IsGenerateMipmap)
//          ? 1
//          : TextureVk::GetMipLevels(info.Extent.width(),
//          info.Extent.height());
//  assert((int32_t)info.SampleCount >= 1);
//
//  // VkImageView                imageViewUAV = nullptr;
//  //  std::map<int32, VkImageView>     imageViewForMipMap;
//  //  std::map<int32, VkImageView>     imageViewForMipMapUAV;
//  std::shared_ptr<TextureVk> TexturePtr;
//
//  ETextureCreateFlag TextureCreateFlag{};
//  if (hasDepthAttachment) {
//    TextureCreateFlag |= ETextureCreateFlag::DSV;
//  } else {
//    TextureCreateFlag |= ETextureCreateFlag::RTV | ETextureCreateFlag::UAV;
//  }
//
//  if (info.IsUseAsSubpassInput) {
//    TextureCreateFlag |= ETextureCreateFlag::SubpassInput;
//  }
//
//  if (info.IsMemoryless) {
//    TextureCreateFlag |= ETextureCreateFlag::Memoryless;
//  }
//
//  const VkImageAspectFlags ImageAspectFlag = hasDepthAttachment
//                                               ? VK_IMAGE_ASPECT_DEPTH_BIT
//                                               : VK_IMAGE_ASPECT_COLOR_BIT;
//
//  switch (info.Type) {
//    case VK_IMAGE_VIEW_TYPE_2D:
//
//      TexturePtr = Create2DTexture(info.Extent.width(),
//                                   info.Extent.height(),
//                                   1,
//                                   mipLevels,
//                                   info.Format,
//                                   TextureCreateFlag,
//                                   VK_IMAGE_LAYOUT_GENERAL);
//      /*imageViewForMipMap[0] = TexturePtr->imageView;
//
//      assert(mipLevels > 0);
//      for (int32_t i = 1; i < mipLevels; ++i) {
//        imageViewForMipMap[i] = CreateTextureViewForSpecificMipMap(
//            TexturePtr->image, textureFormat, ImageAspectFlag, i);
//      }*/
//      break;
//    case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
//
//      TexturePtr = Create2DTexture(info.Extent.width(),
//                                   info.Extent.height(),
//                                   info.LayerCount,
//                                   mipLevels,
//                                   info.Format,
//                                   TextureCreateFlag,
//                                   VK_IMAGE_LAYOUT_GENERAL);
//      /*imageViewForMipMap[0] = TexturePtr->imageView;
//
//      assert(mipLevels > 0);
//      for (int32_t i = 1; i < mipLevels; ++i) {
//        imageViewForMipMap[i]
//            = CreateTexture2DArrayViewForSpecificMipMap(TexturePtr->image,
//                                                        info.LayerCount,
//                                                        textureFormat,
//                                                        ImageAspectFlag,
//                                                        i);
//      }*/
//      break;
//    case VK_IMAGE_VIEW_TYPE_CUBE:
//
//      assert(info.LayerCount == 6);
//      TexturePtr = CreateCubeTexture(info.Extent.width(),
//                                     info.Extent.height(),
//                                     mipLevels,
//                                     info.Format,
//                                     TextureCreateFlag,
//                                     VK_IMAGE_LAYOUT_GENERAL);
//
//      // Create for Shader Resource (TextureCube)
//      //{
//      //  imageViewForMipMap[0] = TexturePtr->imageView;
//
//      //  assert(mipLevels > 0);
//      //  for (int32_t i = 1; i < mipLevels; ++i) {
//      //    imageViewForMipMap[i] = CreateTextureCubeViewForSpecificMipMap(
//      //        TexturePtr->image, textureFormat, ImageAspectFlag, i);
//      //  }
//      //}
//
//      // Create for UAV (writing compute shader resource) (Texture2DArray)
//      //{
//      //  imageViewUAV             = TexturePtr->imageView;
//      //  imageViewForMipMapUAV[0] = TexturePtr->imageView;
//
//      //  assert(mipLevels > 0);
//      //  for (int32_t i = 1; i < mipLevels; ++i) {
//      //    imageViewForMipMapUAV[i]
//      //        = CreateTexture2DArrayViewForSpecificMipMap(TexturePtr->image,
//      //                                                    info.LayerCount,
//      //                                                    textureFormat,
//      //                                                    ImageAspectFlag,
//      //                                                    i);
//      //  }
//      //}
//      break;
//    default:
//      // TODO: log error - Unsupported type texture in FramebufferPool
//      return nullptr;
//  }
//
//  // TexturePtr->ViewForMipMap    = imageViewForMipMap;
//  // TexturePtr->ViewUAV          = imageViewUAV;
//  // TexturePtr->ViewUAVForMipMap = imageViewForMipMapUAV;
//
//  auto RenderTargetPtr        = std::make_shared<RenderTargetVk>();
//  RenderTargetPtr->Info       = info;
//  RenderTargetPtr->TexturePtr = TexturePtr;
//
//  return RenderTargetPtr;
//}

///////////////////////////////////////////////////////////////////////////////////////////////////

ShaderBindingLayout* RhiVk::CreateShaderBindings(
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

    // Try again, to avoid entering creation section simultaneously.
    auto it_find = ShaderBindingPool.find(hash);
    if (ShaderBindingPool.end() != it_find) {
      return it_find->second;
    }

    auto NewShaderBinding = new ShaderBindingLayoutVk();
    NewShaderBinding->Initialize(InShaderBindingArray);
    NewShaderBinding->Hash  = hash;
    ShaderBindingPool[hash] = NewShaderBinding;

    return NewShaderBinding;
  }
}

std::shared_ptr<ShaderBindingInstance> RhiVk::CreateShaderBindingInstance(
    const ShaderBindingArray&       InShaderBindingArray,
    const ShaderBindingInstanceType InType) const {
  auto shaderBindingsLayout = CreateShaderBindings(InShaderBindingArray);
  assert(shaderBindingsLayout);
  return shaderBindingsLayout->CreateShaderBindingInstance(InShaderBindingArray,
                                                           InType);
}

std::shared_ptr<Buffer> RhiVk::CreateBufferInternal(
    uint64_t          InSize,
    uint64_t          InAlignment,
    EBufferCreateFlag InBufferCreateFlag,
    EResourceLayout   InInitialState,
    const void*       InData,
    uint64_t          InDataSize
    /*, const wchar_t*    InResourceName = nullptr*/) const {
  if (InAlignment > 0) {
    InSize = Align(InSize, InAlignment);
  }

  EVulkanBufferBits BufferBits = EVulkanBufferBits::SHADER_DEVICE_ADDRESS
                               | EVulkanBufferBits::TRANSFER_DST
                               | EVulkanBufferBits::TRANSFER_SRC;
  if (!!(EBufferCreateFlag::UAV & InBufferCreateFlag)) {
    BufferBits = BufferBits | EVulkanBufferBits::STORAGE_BUFFER;
  }

  if (!!(EBufferCreateFlag::AccelerationStructureBuildInput
         & InBufferCreateFlag)) {
    // TODO: Ensure the extension for acceleration structure is enabled and
    // this flag is available in your Vulkan version
    BufferBits
        = BufferBits
        | EVulkanBufferBits::ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY;
  }

  if (!!(EBufferCreateFlag::AccelerationStructure & InBufferCreateFlag)) {
    // TODO: Ensure the extension for acceleration structure is enabled and
    // this flag is available in your Vulkan version
    BufferBits = BufferBits | EVulkanBufferBits::ACCELERATION_STRUCTURE_STORAGE;
  }

  if (!!(EBufferCreateFlag::VertexBuffer & InBufferCreateFlag)) {
    BufferBits = BufferBits | EVulkanBufferBits::VERTEX_BUFFER;
  }

  if (!!(EBufferCreateFlag::IndexBuffer & InBufferCreateFlag)) {
    BufferBits = BufferBits | EVulkanBufferBits::INDEX_BUFFER;
  }

  if (!!(EBufferCreateFlag::IndirectCommand & InBufferCreateFlag)) {
    BufferBits = BufferBits | EVulkanBufferBits::INDIRECT_BUFFER;
  }

  if (!!(EBufferCreateFlag::ShaderBindingTable & InBufferCreateFlag)) {
    // TODO: Ensure the extension for ray tracing is enabled and this flag is
    // available in your Vulkan version
    BufferBits = BufferBits | EVulkanBufferBits::SHADER_BINDING_TABLE;
  }

  EVulkanMemoryBits MemoryBits = EVulkanMemoryBits::DEVICE_LOCAL;
  if (!!(EBufferCreateFlag::CPUAccess & InBufferCreateFlag)) {
    MemoryBits
        |= EVulkanMemoryBits::HOST_VISIBLE | EVulkanMemoryBits::HOST_COHERENT;
  }

  auto bufferVk = CreateBuffer(BufferBits, MemoryBits, InSize, InInitialState);
  if (InData && InDataSize > 0) {
    if (!!(EBufferCreateFlag::CPUAccess & InBufferCreateFlag)) {
      bufferVk->UpdateBuffer(InData, InDataSize);
    } else {
      auto stagingBufferPtr
          = g_rhi->CreateRawBuffer<BufferVk>(InDataSize,
                                             InAlignment,
                                             EBufferCreateFlag::CPUAccess,
                                             EResourceLayout::TRANSFER_SRC,
                                             InData,
                                             InDataSize
                                             /*, "StagingBuffer"*/);
      CopyBuffer(*stagingBufferPtr, *bufferVk, InDataSize);
    }
  }
  return std::shared_ptr<BufferVk>(bufferVk);
}

std::shared_ptr<IUniformBufferBlock> RhiVk::CreateUniformBufferBlock(
    Name InName, LifeTimeType InLifeTimeType, size_t InSize) const {
  auto uniformBufferBlockPtr
      = std::make_shared<UniformBufferBlockVk>(InName, InLifeTimeType);
  uniformBufferBlockPtr->Init(InSize);
  return uniformBufferBlockPtr;
}

// Create Images
VkImageUsageFlags RhiVk::GetImageUsageFlags(
    ETextureCreateFlag InTextureCreateFlag) const {
  // TODO: remove this method (currently not needed)

  VkImageUsageFlags UsageFlag = VK_IMAGE_USAGE_SAMPLED_BIT;
  if (!!(InTextureCreateFlag & ETextureCreateFlag::RTV)) {
    UsageFlag |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  }

  if (!!(InTextureCreateFlag & ETextureCreateFlag::DSV)) {
    UsageFlag |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  }

  if (!!(InTextureCreateFlag & ETextureCreateFlag::UAV)) {
    UsageFlag |= VK_IMAGE_USAGE_STORAGE_BIT;
  }

  if (!!(InTextureCreateFlag & ETextureCreateFlag::TransferSrc)) {
    UsageFlag |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  }

  if (!!(InTextureCreateFlag & ETextureCreateFlag::TransferDst)) {
    UsageFlag |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  }

  if (!!(InTextureCreateFlag & ETextureCreateFlag::ShadingRate)) {
    UsageFlag |= VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
  }

  if (!!(InTextureCreateFlag & ETextureCreateFlag::SubpassInput)) {
    UsageFlag |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
  }

  // This should be placed last, because Memoryless has only Color, Depth,
  // Input attachment usages.
  if (!!(InTextureCreateFlag & ETextureCreateFlag::Memoryless)) {
    UsageFlag &= ~(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                   | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                   | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
  }

  return UsageFlag;
}

VkMemoryPropertyFlagBits RhiVk::GetMemoryPropertyFlagBits(
    ETextureCreateFlag InTextureCreateFlag) const {
  // TODO: remove this method (currently not needed)

  VkMemoryPropertyFlagBits PropertyFlagBits{};
  if (!!(InTextureCreateFlag & ETextureCreateFlag::CPUAccess)) {
    PropertyFlagBits
        = (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                                     | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  }
  return PropertyFlagBits;
}

std::shared_ptr<Texture> RhiVk::Create2DTexture(
    uint32_t             InWidth,
    uint32_t             InHeight,
    uint32_t             InArrayLayers,
    uint32_t             InMipLevels,
    ETextureFormat       InFormat,
    ETextureCreateFlag   InTextureCreateFlag,
    EResourceLayout      InImageLayout,
    const ImageBulkData& InImageBulkData,
    const RTClearValue& InClearValue,
    const wchar_t*       InResourceName) const {
  VkImageCreateFlagBits          ImageCreateFlags{};
  const VkMemoryPropertyFlagBits PropertyFlagBits
      = GetMemoryPropertyFlagBits(InTextureCreateFlag);
  const VkImageUsageFlags UsageFlag = GetImageUsageFlags(InTextureCreateFlag);
  assert(!IsDepthFormat(InFormat)
         || (IsDepthFormat(InFormat)
             && (UsageFlag & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)));

  auto TexturePtr
      = game_engine::Create2DTexture(InWidth,
                                     InHeight,
                                     InMipLevels,
                                     VK_SAMPLE_COUNT_1_BIT,
                                     GetVulkanTextureFormat(InFormat),
                                     VK_IMAGE_TILING_OPTIMAL,
                                     UsageFlag,
                                     PropertyFlagBits,
                                     ImageCreateFlags,
                                     VK_IMAGE_LAYOUT_UNDEFINED);

  if (InImageBulkData.ImageData.size() > 0) {
    // todo : recycle temp buffer
    auto stagingBufferPtr = CreateBuffer(
        EVulkanBufferBits::TRANSFER_SRC,
        EVulkanMemoryBits::HOST_VISIBLE | EVulkanMemoryBits::HOST_COHERENT,
        InImageBulkData.ImageData.size(),
        EResourceLayout::TRANSFER_SRC);

    stagingBufferPtr->UpdateBuffer(&InImageBulkData.ImageData[0],
                                   InImageBulkData.ImageData.size());

    auto commandBuffer = BeginSingleTimeCommands();
    assert(TransitionLayout(
        commandBuffer, TexturePtr.get(), EResourceLayout::TRANSFER_DST));

    if (InImageBulkData.SubresourceFootprints.size() > 0) {
      for (int32_t i = 0;
           i < (int32_t)InImageBulkData.SubresourceFootprints.size();
           ++i) {
        const ImageSubResourceData SubResourceData
            = InImageBulkData.SubresourceFootprints[i];

        CopyBufferToTexture(commandBuffer->GetRef(),
                            stagingBufferPtr->m_buffer,
                            stagingBufferPtr->Offset + SubResourceData.Offset,
                            TexturePtr->image,
                            SubResourceData.Width,
                            SubResourceData.Height,
                            SubResourceData.MipLevel,
                            SubResourceData.Depth);
      }
    } else {
      CopyBufferToTexture(commandBuffer->GetRef(),
                          stagingBufferPtr->m_buffer,
                          stagingBufferPtr->Offset,
                          TexturePtr->image,
                          InWidth,
                          InHeight);
    }

    assert(TransitionLayout(commandBuffer, TexturePtr.get(), InImageLayout));

    EndSingleTimeCommands(commandBuffer);
  }

  return TexturePtr;
}

std::shared_ptr<Texture> RhiVk::CreateCubeTexture(
    uint32_t             InWidth,
    uint32_t             InHeight,
    uint32_t             InMipLevels,
    ETextureFormat       InFormat,
    ETextureCreateFlag   InTextureCreateFlag,
    EResourceLayout      InImageLayout,
    const ImageBulkData& InImageBulkData,
    const RTClearValue& InClearValue,
    const wchar_t*       InResourceName) const {
  VkImageCreateFlagBits ImageCreateFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
  const VkMemoryPropertyFlagBits PropertyFlagBits
      = GetMemoryPropertyFlagBits(InTextureCreateFlag);
  const VkImageUsageFlags UsageFlag = GetImageUsageFlags(InTextureCreateFlag);
  assert(!IsDepthFormat(InFormat)
         || (IsDepthFormat(InFormat)
             && (UsageFlag & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)));

  auto TexturePtr
      = game_engine::CreateCubeTexture(InWidth,
                                       InHeight,
                                       InMipLevels,
                                       VK_SAMPLE_COUNT_1_BIT,
                                       GetVulkanTextureFormat(InFormat),
                                       VK_IMAGE_TILING_OPTIMAL,
                                       UsageFlag,
                                       PropertyFlagBits,
                                       ImageCreateFlags,
                                       VK_IMAGE_LAYOUT_UNDEFINED);

  if (InImageBulkData.ImageData.size() > 0) {
    // todo : recycle temp buffer
    auto stagingBufferPtr = game_engine::CreateBuffer(
        EVulkanBufferBits::TRANSFER_SRC,
        EVulkanMemoryBits::HOST_VISIBLE | EVulkanMemoryBits::HOST_COHERENT,
        InImageBulkData.ImageData.size(),
        EResourceLayout::TRANSFER_SRC);

    stagingBufferPtr->UpdateBuffer(&InImageBulkData.ImageData[0],
                                   InImageBulkData.ImageData.size());

    auto commandBuffer = BeginSingleTimeCommands();
    assert(TransitionLayout(
        commandBuffer, TexturePtr.get(), EResourceLayout::TRANSFER_DST));

    if (InImageBulkData.SubresourceFootprints.size() > 0) {
      for (int32_t i = 0;
           i < (int32_t)InImageBulkData.SubresourceFootprints.size();
           ++i) {
        const ImageSubResourceData SubResourceData
            = InImageBulkData.SubresourceFootprints[i];

        CopyBufferToTexture(commandBuffer->GetRef(),
                            stagingBufferPtr->m_buffer,
                            stagingBufferPtr->Offset + SubResourceData.Offset,
                            TexturePtr->image,
                            SubResourceData.Width,
                            SubResourceData.Height,
                            SubResourceData.MipLevel,
                            SubResourceData.Depth);
      }
    } else {
      CopyBufferToTexture(commandBuffer->GetRef(),
                          stagingBufferPtr->m_buffer,
                          stagingBufferPtr->Offset,
                          TexturePtr->image,
                          InWidth,
                          InHeight);
    }

    assert(TransitionLayout(commandBuffer, TexturePtr.get(), InImageLayout));

    EndSingleTimeCommands(commandBuffer);
  }

  return TexturePtr;
}

void RhiVk::DrawArrays(
    const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
    /*EPrimitiveType                               type, - deprecated (used in
       previous rendering api)*/
    int32_t                                     vertStartIndex,
    int32_t                                     vertCount) const {
  assert(InRenderFrameContext);
  assert(InRenderFrameContext->GetActiveCommandBuffer());
  vkCmdDraw((VkCommandBuffer)InRenderFrameContext->GetActiveCommandBuffer()
                ->GetNativeHandle(),
            vertCount,
            1,
            vertStartIndex,
            0);
}

void RhiVk::DrawArraysInstanced(
    const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
    /*EPrimitiveType                               type, - deprecated (used in
       previous rendering api)*/
    int32_t                                     vertStartIndex,
    int32_t                                     vertCount,
    int32_t                                     instanceCount) const {
  assert(InRenderFrameContext);
  assert(InRenderFrameContext->GetActiveCommandBuffer());
  vkCmdDraw((VkCommandBuffer)InRenderFrameContext->GetActiveCommandBuffer()
                ->GetNativeHandle(),
            vertCount,
            instanceCount,
            vertStartIndex,
            0);
}

void RhiVk::DrawElements(
    const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
    /*EPrimitiveType                               type, - deprecated (used in
       previous rendering api)*/
    int32_t                                     elementSize,
    int32_t                                     startIndex,
    int32_t                                     indexCount) const {
  assert(InRenderFrameContext);
  assert(InRenderFrameContext->GetActiveCommandBuffer());
  vkCmdDrawIndexed(
      (VkCommandBuffer)InRenderFrameContext->GetActiveCommandBuffer()
          ->GetNativeHandle(),
      indexCount,
      1,
      startIndex,
      0,
      0);
}

void RhiVk::DrawElementsInstanced(
    const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
    /*EPrimitiveType                               type, - deprecated (used in
       previous rendering api)*/
    int32_t                                     elementSize,
    int32_t                                     startIndex,
    int32_t                                     indexCount,
    int32_t                                     instanceCount) const {
  assert(InRenderFrameContext);
  assert(InRenderFrameContext->GetActiveCommandBuffer());
  vkCmdDrawIndexed(
      (VkCommandBuffer)InRenderFrameContext->GetActiveCommandBuffer()
          ->GetNativeHandle(),
      indexCount,
      instanceCount,
      startIndex,
      0,
      0);
}

void RhiVk::DrawElementsBaseVertex(
    const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
    /*EPrimitiveType                               type, - deprecated (used in
       previous rendering api)*/
    int32_t                                     elementSize,
    int32_t                                     startIndex,
    int32_t                                     indexCount,
    int32_t                                     baseVertexIndex) const {
  assert(InRenderFrameContext);
  assert(InRenderFrameContext->GetActiveCommandBuffer());
  vkCmdDrawIndexed(
      (VkCommandBuffer)InRenderFrameContext->GetActiveCommandBuffer()
          ->GetNativeHandle(),
      indexCount,
      1,
      startIndex,
      baseVertexIndex,
      0);
}

void RhiVk::DrawElementsInstancedBaseVertex(
    const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
    /*EPrimitiveType                               type, - deprecated (used in
       previous rendering api)*/
    int32_t                                     elementSize,
    int32_t                                     startIndex,
    int32_t                                     indexCount,
    int32_t                                     baseVertexIndex,
    int32_t                                     instanceCount) const {
  assert(InRenderFrameContext);
  assert(InRenderFrameContext->GetActiveCommandBuffer());
  vkCmdDrawIndexed(
      (VkCommandBuffer)InRenderFrameContext->GetActiveCommandBuffer()
          ->GetNativeHandle(),
      indexCount,
      instanceCount,
      startIndex,
      baseVertexIndex,
      0);
}

void RhiVk::DrawIndirect(
    const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
    /*EPrimitiveType                               type, - deprecated (used in
       previous rendering api)*/
    Buffer*                                    buffer,
    int32_t                                     startIndex,
    int32_t                                     drawCount) const {
  assert(InRenderFrameContext);
  assert(InRenderFrameContext->GetActiveCommandBuffer());
  vkCmdDrawIndirect(
      (VkCommandBuffer)InRenderFrameContext->GetActiveCommandBuffer()
          ->GetNativeHandle(),
      (VkBuffer)buffer->GetHandle(),
      startIndex * sizeof(VkDrawIndirectCommand),
      drawCount,
      sizeof(VkDrawIndirectCommand));
}

void RhiVk::DrawElementsIndirect(
    const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
    /*EPrimitiveType                               type, - deprecated (used in
       previous rendering api)*/
    Buffer*                                    buffer,
    int32_t                                     startIndex,
    int32_t                                     drawCount) const {
  assert(InRenderFrameContext);
  assert(InRenderFrameContext->GetActiveCommandBuffer());
  vkCmdDrawIndexedIndirect(
      (VkCommandBuffer)InRenderFrameContext->GetActiveCommandBuffer()
          ->GetNativeHandle(),
      (VkBuffer)buffer->GetHandle(),
      startIndex * sizeof(VkDrawIndexedIndirectCommand),
      drawCount,
      sizeof(VkDrawIndexedIndirectCommand));
}

void RhiVk::DispatchCompute(
    const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
    uint32_t                                    numGroupsX,
    uint32_t                                    numGroupsY,
    uint32_t                                    numGroupsZ) const {
  assert(InRenderFrameContext);
  assert(InRenderFrameContext->GetActiveCommandBuffer());
  vkCmdDispatch((VkCommandBuffer)InRenderFrameContext->GetActiveCommandBuffer()
                    ->GetNativeHandle(),
                numGroupsX,
                numGroupsY,
                numGroupsZ);
}

void RhiVk::Flush() const {
  // TODO: Implement the Flush method to submit pending commands to the GPU.
  // Currently, it's identical to Finish method and waits for all queues to be
  // idle, which is a blocking operation. Ideally, Flush should only ensure
  // that commands have been submitted to the GPU for execution without
  // waiting for their completion. This might involve managing command buffers
  // and submissions more explicitly to track what has been submitted but not
  // yet completed.
  vkQueueWaitIdle(m_graphicsQueue_.Queue);
  vkQueueWaitIdle(m_computeQueue_.Queue);
  vkQueueWaitIdle(m_presentQueue_.Queue);
}

void RhiVk::Finish() const {
  vkQueueWaitIdle(m_graphicsQueue_.Queue);
  vkQueueWaitIdle(m_computeQueue_.Queue);
  vkQueueWaitIdle(m_presentQueue_.Queue);
}

// TODO: check whether it's called
void RhiVk::RecreateSwapChain() {
  Flush();

  int32_t width = 0, height = 0;
  SDL_GetWindowSize(m_window_->getNativeWindowHandle(), &width, &height);
  while (width == 0 || height == 0) {
    SDL_GetWindowSize(m_window_->getNativeWindowHandle(), &width, &height);
    SDL_WaitEvent(NULL);
  }

  // SCR_WIDTH  = width;
  // SCR_HEIGHT = height;

  // use std::shared_ptr instead of raw pointers
  delete m_commandBufferManager_;
  m_commandBufferManager_ = new CommandBufferManagerVk();
  m_commandBufferManager_->CreatePool(m_graphicsQueue_.QueueIndex);

  FrameBufferPool::Release();
  RenderTargetPool::Release();
  PipelineStatePool.Release();
  RenderPassPool.Release();

  // delete m_swapchain_;
  // m_swapchain_ = new m_swapchain_();
  m_swapchain_->Create(m_window_);

  Flush();
}

std::shared_ptr<RenderFrameContext> RhiVk::BeginRenderFrame() {
  VkResult acquireNextImageResult = vkAcquireNextImageKHR(
      m_device_,
      (VkSwapchainKHR)m_swapchain_->GetHandle(),
      UINT64_MAX,
      (VkSemaphore)m_swapchain_->GetSwapchainImage(CurrentFrameIndex)
          ->Available->GetHandle(),
      VK_NULL_HANDLE,
      &CurrentFrameIndex);

  VkFence lastCommandBufferFence
      = m_swapchain_->GetSwapchainImage(CurrentFrameIndex)->CommandBufferFence;

  if (acquireNextImageResult != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error,
                      "Failed to acquire next image from swapchain.");
    return nullptr;
  }

  if (acquireNextImageResult == VK_ERROR_OUT_OF_DATE_KHR) {
    GlobalLogger::Log(LogLevel::Warning,
                      "Swapchain is out of date. Recreating swapchain.");
    RecreateSwapChain();
    return nullptr;
  } else if (acquireNextImageResult != VK_SUCCESS
             && acquireNextImageResult != VK_SUBOPTIMAL_KHR) {
    GlobalLogger::Log(LogLevel::Error,
                      "Failed to acquire next image from swapchain.");
    assert(0);
    return nullptr;
  }

  CommandBufferVk* commandBuffer
      = (CommandBufferVk*)
            g_rhi_vk->m_commandBufferManager_->GetOrCreateCommandBuffer();

  if (lastCommandBufferFence != VK_NULL_HANDLE) {
    vkWaitForFences(m_device_, 1, &lastCommandBufferFence, VK_TRUE, UINT64_MAX);
  }

  GetOneFrameUniformRingBuffer()->Reset();
  GetDescriptorPoolForSingleFrame()->Reset();

  m_swapchain_->GetSwapchainImage(CurrentFrameIndex)->CommandBufferFence
      = (VkFence)commandBuffer->GetFenceHandle();

  auto renderFrameContextPtr
      = std::make_shared<RenderFrameContextVk>(commandBuffer);
  // renderFrameContextPtr->UseForwardRenderer =
  // !gOptions.UseDeferredRenderer;
  renderFrameContextPtr->FrameIndex = CurrentFrameIndex;
  renderFrameContextPtr->SceneRenderTargetPtr
      = std::make_shared<SceneRenderTarget>();
  renderFrameContextPtr->SceneRenderTargetPtr->Create(
      m_window_, m_swapchain_->GetSwapchainImage(CurrentFrameIndex));
  renderFrameContextPtr->CurrentWaitSemaphore
      = m_swapchain_->GetSwapchainImage(CurrentFrameIndex)->Available;

  return renderFrameContextPtr;
}

void RhiVk::EndRenderFrame(
    const std::shared_ptr<RenderFrameContext>& renderFrameContextPtr) {
  VkSemaphore signalSemaphore[]
      = {(VkSemaphore)m_swapchain_->GetSwapchainImage(CurrentFrameIndex)
             ->RenderFinished->GetHandle()};
  QueueSubmit(
      renderFrameContextPtr,
      m_swapchain_->GetSwapchainImage(CurrentFrameIndex)->RenderFinished);

  VkPresentInfoKHR presentInfo   = {};
  presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores    = signalSemaphore;

  VkSwapchainKHR swapChains[] = {(VkSwapchainKHR)m_swapchain_->GetHandle()};
  presentInfo.swapchainCount  = 1;
  presentInfo.pSwapchains     = swapChains;
  presentInfo.pImageIndices   = &CurrentFrameIndex;

  presentInfo.pResults = nullptr;  // Optional
  VkResult queuePresentResult
      = vkQueuePresentKHR(m_presentQueue_.Queue, &presentInfo);

  CurrentFrameIndex
      = (CurrentFrameIndex + 1) % m_swapchain_->GetNumOfSwapchainImages();
  renderFrameContextPtr->Destroy();

  if ((queuePresentResult == VK_ERROR_OUT_OF_DATE_KHR)
      || (queuePresentResult == VK_SUBOPTIMAL_KHR) || FramebufferResized) {
    RecreateSwapChain();
    FramebufferResized = false;
    return;
  } else if (queuePresentResult != VK_SUCCESS) {
    assert(0);
    return;
  }
}

void RhiVk::QueueSubmit(
    const std::shared_ptr<RenderFrameContext>& renderFrameContextPtr,
    Semaphore*                                 InSignalSemaphore) {
  auto renderFrameContext = (RenderFrameContextVk*)renderFrameContextPtr.get();
  assert(renderFrameContext);

  assert(renderFrameContext->GetActiveCommandBuffer());

  renderFrameContext->GetActiveCommandBuffer()->End();

  VkCommandBuffer vkCommandBuffer
      = (VkCommandBuffer)renderFrameContext->GetActiveCommandBuffer()
            ->GetNativeHandle();
  VkFence vkFence
      = (VkFence)renderFrameContext->GetActiveCommandBuffer()->GetFenceHandle();

  // Submitting the command buffer
  VkSubmitInfo submitInfo = {};
  submitInfo.sType        = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitsemaphores[]
      = {(VkSemaphore)renderFrameContext->CurrentWaitSemaphore->GetHandle()};
  VkPipelineStageFlags waitStages[]
      = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores    = waitsemaphores;
  submitInfo.pWaitDstStageMask  = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &vkCommandBuffer;

  renderFrameContext->CurrentWaitSemaphore = InSignalSemaphore;

  VkSemaphore signalSemaphores[]
      = {(VkSemaphore)InSignalSemaphore->GetHandle()};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores    = signalSemaphores;

  vkResetFences(m_device_, 1, &vkFence);

  auto queueSubmitResult
      = vkQueueSubmit(m_graphicsQueue_.Queue, 1, &submitInfo, vkFence);
  if ((queueSubmitResult == VK_ERROR_OUT_OF_DATE_KHR)
      || (queueSubmitResult == VK_SUBOPTIMAL_KHR)) {
    GlobalLogger::Log(LogLevel::Warning,
                      "Swapchain out of date or suboptimal, recreating...");
    RecreateSwapChain();
  } else if (queueSubmitResult != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to submit queue");
  }
}

CommandBufferVk* RhiVk::BeginSingleTimeCommands() const {
  return (CommandBufferVk*)m_commandBufferManager_->GetOrCreateCommandBuffer();

  // VkCommandBufferAllocateInfo allocInfo = {};
  // allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  // allocInfo.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  // allocInfo.commandPool = m_commandBufferManager->GetPool();
  // allocInfo.commandBufferCount = 1;

  // VkCommandBuffer commandBuffer;
  // vkAllocateCommandBuffers(m_device_, &allocInfo, &commandBuffer);

  // VkCommandBufferBeginInfo beginInfo = {};
  // beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  // beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  // vkBeginCommandBuffer(commandBuffer, &beginInfo);
  // return commandBuffer;
}

void RhiVk::EndSingleTimeCommands(CommandBuffer* commandBuffer) const {
  // vkEndCommandBuffer(commandBuffer);

  // VkSubmitInfo submitInfo       = {};
  // submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  // submitInfo.commandBufferCount = 1;
  // submitInfo.pCommandBuffers    = &commandBuffer;

  // vkQueueSubmit(m_graphicsQueue_.Queue, 1, &submitInfo, VK_NULL_HANDLE);
  // vkQueueWaitIdle(m_graphicsQueue_.Queue);

  // vkFreeCommandBuffers(
  //     m_device_, m_commandBufferManager->GetPool(), 1, &commandBuffer);

  auto commandBufferVk = (CommandBufferVk*)commandBuffer;

  commandBufferVk->End();

  VkSubmitInfo submitInfo       = {};
  submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &commandBufferVk->GetRef();

  VkFence vkFence = (VkFence)commandBufferVk->GetFenceHandle();

  vkResetFences(m_device_, 1, &vkFence);

  auto Result = vkQueueSubmit(m_graphicsQueue_.Queue, 1, &submitInfo, vkFence);
  assert(VK_SUCCESS == Result);

  Result = vkQueueWaitIdle(m_graphicsQueue_.Queue);
  assert(VK_SUCCESS == Result);

  m_commandBufferManager_->ReturnCommandBuffer(commandBufferVk);
}

void RhiVk::BindGraphicsShaderBindingInstances(
    const CommandBuffer*                InCommandBuffer,
    const PipelineStateInfo*            InPiplineState,
    const ShaderBindingInstanceCombiner& InShaderBindingInstanceCombiner,
    uint32_t                             InFirstSet) const {
  if (InShaderBindingInstanceCombiner.DescriptorSetHandles.NumOfData) {
    assert(InCommandBuffer);
    vkCmdBindDescriptorSets(
        (VkCommandBuffer)InCommandBuffer->GetNativeHandle(),
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        (VkPipelineLayout)InPiplineState->GetPipelineLayoutHandle(),
        InFirstSet,
        InShaderBindingInstanceCombiner.DescriptorSetHandles.NumOfData,
        (const VkDescriptorSet*)&InShaderBindingInstanceCombiner
            .DescriptorSetHandles[0],
        InShaderBindingInstanceCombiner.DynamicOffsets.NumOfData,
        (InShaderBindingInstanceCombiner.DynamicOffsets.NumOfData
             ? &InShaderBindingInstanceCombiner.DynamicOffsets[0]
             : nullptr));
  }
}

bool RhiVk::TransitionLayout(VkCommandBuffer commandBuffer,
                             VkImage         image,
                             VkFormat        format,
                             uint32_t        mipLevels,
                             uint32_t        layoutCount,
                             VkImageLayout   oldLayout,
                             VkImageLayout   newLayout) const {
  if (oldLayout == newLayout) {
    return true;
  }

  VkImageMemoryBarrier barrier = {};
  barrier.sType                = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout            = oldLayout;
  barrier.newLayout            = newLayout;
  barrier.srcQueueFamilyIndex
      = VK_QUEUE_FAMILY_IGNORED;  // todo : need to control this when the
                                  // pipeline stage control is not All stage
                                  // bits
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

  barrier.image = image;
  switch (newLayout) {
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
    case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
      if (IsDepthOnlyFormat(format)) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
      } else if (IsDepthFormat(format)) {
        barrier.subresourceRange.aspectMask
            = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
      } else {
        assert(0);
      }
      break;
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
    case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
      if (IsDepthOnlyFormat(format)) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
      } else {
        assert(0);
      }
      break;
    default:
      if (IsDepthOnlyFormat(format)) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
      } else if (IsDepthFormat(format)) {
        barrier.subresourceRange.aspectMask
            = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
      } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      }
      break;
  }
  // TODO: consider removing
  // if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
  //{
  //     barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  //     if (VulkanBufferUtil::HasStencilComponent(format))
  //         barrier.subresourceRange.aspectMask |=
  //         VK_IMAGE_ASPECT_STENCIL_BIT;
  // }
  // else
  //{
  //     barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  // }
  barrier.subresourceRange.baseMipLevel   = 0;
  barrier.subresourceRange.levelCount     = mipLevels;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount     = layoutCount;
  barrier.srcAccessMask                   = 0;  // TODO
  barrier.dstAccessMask                   = 0;  // TODO

  // todo : need to control pipeline stage
  VkPipelineStageFlags sourceStage      = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
  VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

  // Source layouts (old)
  // Source access mask controls actions that have to be finished on the old
  // layout before it will be transitioned to the new layout
  switch (oldLayout) {
    case VK_IMAGE_LAYOUT_UNDEFINED:
      // Image layout is undefined (or does not matter)
      // Only valid as initial layout
      // No flags required, listed only for completeness
      barrier.srcAccessMask = 0;
      break;

    case VK_IMAGE_LAYOUT_PREINITIALIZED:
      // Image is preinitialized
      // Only valid as initial layout for linear images, preserves memory
      // contents Make sure host writes have been finished
      barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
      break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      // Image is a color attachment
      // Make sure any writes to the color buffer have been finished
      barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
      // Image is a depth/stencil attachment
      // Make sure any writes to the depth/stencil buffer have been finished
      barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
      // Image is a transfer source
      // Make sure any reads from the image have been finished
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      // Image is a transfer destination
      // Make sure any writes to the image have been finished
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      // Image is read by a shader
      // Make sure any shader reads from the image have been finished
      barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
      break;
    default:
      // Other source layouts aren't handled (yet)
      break;
  }

  // Target layouts (new)
  // Destination access mask controls the dependency for the new image layout
  switch (newLayout) {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      // Image will be used as a transfer destination
      // Make sure any writes to the image have been finished
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
      // Image will be used as a transfer source
      // Make sure any reads from the image have been finished
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      // Image will be used as a color attachment
      // Make sure any writes to the color buffer have been finished
      barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
      // Image layout will be used as a depth/stencil attachment
      // Make sure any writes to depth/stencil buffer have been finished
      barrier.dstAccessMask = barrier.dstAccessMask
                            | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      // Image will be read in a shader (sampler, input attachment)
      // Make sure any writes to the image have been finished
      if (barrier.srcAccessMask == 0) {
        barrier.srcAccessMask
            = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
      }
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      break;
    default:
      // Other source layouts aren't handled (yet)
      break;
  }

  vkCmdPipelineBarrier(commandBuffer,
                       sourceStage,
                       destinationStage,
                       0,
                       0,
                       nullptr,
                       0,
                       nullptr,
                       1,
                       &barrier);

  return true;
}

bool RhiVk::TransitionLayout(CommandBuffer* commandBuffer,
                             Texture*       texture,
                             EResourceLayout newLayout) const {
  assert(commandBuffer);
  assert(texture);

  if (texture->GetLayout() == newLayout) {
    return true;
  }

  auto texture_vk = (TextureVk*)texture;
  if (texture_vk->IsDepthFormat()
      && (EResourceLayout::DEPTH_READ_ONLY == newLayout
          || EResourceLayout::SHADER_READ_ONLY == newLayout)) {
    newLayout = EResourceLayout::DEPTH_STENCIL_READ_ONLY;
  }

  if (TransitionLayout((VkCommandBuffer)commandBuffer->GetNativeHandle(),
                       texture_vk->image,
                       GetVulkanTextureFormat(texture_vk->format),
                       texture_vk->mipLevels,
                       texture_vk->layerCount,
                       GetVulkanImageLayout(texture_vk->imageLayout),
                       GetVulkanImageLayout(newLayout))) {
    ((TextureVk*)texture)->imageLayout = newLayout;
    return true;
  }
  return true;
}

void RhiVk::BindComputeShaderBindingInstances(
    const CommandBuffer*                InCommandBuffer,
    const PipelineStateInfo*            InPiplineState,
    const ShaderBindingInstanceCombiner& InShaderBindingInstanceCombiner,
    uint32_t                             InFirstSet) const {
  if (InShaderBindingInstanceCombiner.DescriptorSetHandles.NumOfData) {
    assert(InCommandBuffer);
    vkCmdBindDescriptorSets(
        (VkCommandBuffer)InCommandBuffer->GetNativeHandle(),
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        (VkPipelineLayout)InPiplineState->GetPipelineLayoutHandle(),
        InFirstSet,
        InShaderBindingInstanceCombiner.DescriptorSetHandles.NumOfData,
        (const VkDescriptorSet*)&InShaderBindingInstanceCombiner
            .DescriptorSetHandles[0],
        InShaderBindingInstanceCombiner.DynamicOffsets.NumOfData,
        (InShaderBindingInstanceCombiner.DynamicOffsets.NumOfData
             ? &InShaderBindingInstanceCombiner.DynamicOffsets[0]
             : nullptr));
  }
}

void RhiVk::NextSubpass(const CommandBuffer* commandBuffer) const {
  assert(commandBuffer);
  vkCmdNextSubpass((VkCommandBuffer)commandBuffer->GetNativeHandle(),
                   VK_SUBPASS_CONTENTS_INLINE);
}

// TODO: uncomment
// private:
const std::vector<const char*> DeviceExtensions
    = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

void RhiVk::populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
  createInfo       = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
#if VALIDATION_LAYER_VERBOSE
                             | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
#endif
                             | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                             | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                         | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                         | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debugCallback;
}

bool RhiVk::isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(device, &deviceProperties);

  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

  QueueFamilyIndices indices = FindQueueFamilies(device, surface);

  bool extensionsSupported = CheckDeviceExtensionSupport(device);

  bool swapChainAdequate = false;
  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport
        = QuerySwapChainSupport(device, surface);
    swapChainAdequate = !swapChainSupport.Formats.empty()
                     && !swapChainSupport.PresentModes.empty();
  }

  return indices.IsComplete() && extensionsSupported && swapChainAdequate;
}

bool RhiVk::CheckDeviceExtensionSupport(VkPhysicalDevice device) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(
      device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(
      device, nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredExtensions(DeviceExtensions.begin(),
                                           DeviceExtensions.end());
  for (const auto& extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

}  // namespace game_engine
