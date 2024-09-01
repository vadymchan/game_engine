#include "gfx/rhi/vulkan/rhi_vk.h"

#include "gfx/rhi/frame_buffer_pool.h"
#include "gfx/rhi/render_target_pool.h"
#include "utils/memory/align.h"

namespace game_engine {

RhiVk* g_rhiVk = nullptr;

TResourcePool<RenderPassVk, MutexRWLock>        RhiVk::s_renderPassPool;
TResourcePool<PipelineStateInfoVk, MutexRWLock> RhiVk::s_pipelineStatePool;
TResourcePool<SamplerStateInfoVk, MutexRWLock>  RhiVk::s_samplerStatePool;
TResourcePool<RasterizationStateInfoVk, MutexRWLock>
    RhiVk::s_rasterizationStatePool;
TResourcePool<StencilOpStateInfoVk, MutexRWLock> RhiVk::s_stencilOpStatePool;
TResourcePool<DepthStencilStateInfoVk, MutexRWLock>
                                                RhiVk::s_depthStencilStatePool;
TResourcePool<BlendingStateInfoVk, MutexRWLock> RhiVk::s_blendingStatePool;
std::unordered_map<uint64_t, ShaderBindingLayoutVk*> RhiVk::s_shaderBindingPool;

// TODO: consider whether need it
struct FrameBufferVk : public FrameBuffer {
  bool                                   m_isInitialized = false;
  std::vector<std::shared_ptr<Texture> > m_allTextures;

  virtual Texture* getTexture(std::int32_t index = 0) const {
    return m_allTextures[index].get();
  }
};

RhiVk::RhiVk() {
  g_rhiVk = this;
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
        = g_findQueueFamilies(m_physicalDevice_, m_surface_);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.m_graphicsFamily_.value(),
                                              indices.m_presentFamily_.value(),
                                              indices.m_computeFamily_.value()};

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

    m_graphicsQueue_.m_queueIndex_ = indices.m_graphicsFamily_.value();
    m_presentQueue_.m_queueIndex_  = indices.m_presentFamily_.value();
    m_computeQueue_.m_queueIndex_  = indices.m_computeFamily_.value();
    vkGetDeviceQueue(m_device_,
                     m_graphicsQueue_.m_queueIndex_,
                     0,
                     &m_graphicsQueue_.m_queue_);
    vkGetDeviceQueue(
        m_device_, m_presentQueue_.m_queueIndex_, 0, &m_presentQueue_.m_queue_);
    vkGetDeviceQueue(
        m_device_, m_computeQueue_.m_queueIndex_, 0, &m_computeQueue_.m_queue_);
  }

  //  	// Check if the Vsync support
  //  {
  //    assert(PhysicalDevice);
  //    assert(Surface);
  //    SwapChainSupportDetails swapChainSupport
  //        = g_querySwapChainSupport(PhysicalDevice, Surface);
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
  // get vkCmdBindShadingRateImageNV function pointer for VRS
  //  vkCmdBindShadingRateImageNV
  //      = reinterpret_cast<PFN_vkCmdBindShadingRateImageNV>(
  //          vkGetDeviceProcAddr(Device, "vkCmdBindShadingRateImageNV"));
  //
  //  // get debug marker function pointer
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
  m_swapchain_->create(window);

  m_fenceManager_ = new FenceManagerVk();

  m_commandBufferManager_ = new CommandBufferManagerVk();
  m_commandBufferManager_->createPool(m_computeQueue_.m_queueIndex_);

  // Pipeline cache
  VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
  pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  if (vkCreatePipelineCache(
          m_device_, &pipelineCacheCreateInfo, nullptr, &m_pipelineCache_)
      != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error,
                      "Failed to create Vulkan pipeline cache");
    return false;
  }

  // Ring buffer
  m_oneFrameUniformRingBuffers_.resize(m_swapchain_->getNumOfSwapchainImages());
  for (auto& iter : m_oneFrameUniformRingBuffers_) {
    iter = new RingBufferVk();
    iter->create(
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

  m_descriptorPoolsSingleFrame_.resize(m_swapchain_->getNumOfSwapchainImages());
  for (auto& iter : m_descriptorPoolsSingleFrame_) {
    iter = new DescriptorPoolVk();
    iter->create(10'000);
  }

  m_descriptorPoolMultiFrame_ = new DescriptorPoolVk();
  m_descriptorPoolMultiFrame_->create(10'000);

  return true;
}

void RhiVk::release() {
  flush();

  RHI::release();  // TODO: consider whether need to remove

  s_renderPassPool.release();
  s_samplerStatePool.release();
  s_rasterizationStatePool.release();
  s_stencilOpStatePool.release();
  s_depthStencilStatePool.release();
  s_blendingStatePool.release();
  s_pipelineStatePool.release();

  FrameBufferPool::s_release();
  RenderTargetPool::s_release();

  // TODO: consider removing
  // delete m_swapchain_;
  // m_swapchain_ = nullptr;
  // TODO: consider using destructor
  m_swapchain_->release();

  delete m_commandBufferManager_;
  m_commandBufferManager_ = nullptr;

  ShaderBindingLayoutVk::s_clearPipelineLayout();

  {
    ScopeWriteLock s(&m_shaderBindingPoolLock_);
    for (auto& iter : s_shaderBindingPool) {
      delete iter.second;
    }
    s_shaderBindingPool.clear();
  }

  for (auto& iter : m_oneFrameUniformRingBuffers_) {
    delete iter;
  }
  m_oneFrameUniformRingBuffers_.clear();

  for (auto& iter : m_descriptorPoolsSingleFrame_) {
    delete iter;
  }
  m_descriptorPoolsSingleFrame_.clear();

  delete m_descriptorPoolMultiFrame_;

  vkDestroyPipelineCache(m_device_, m_pipelineCache_, nullptr);

  delete m_fenceManager_;
  m_fenceManager_ = nullptr;

  m_semaphoreManager_.release();

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

std::shared_ptr<VertexBuffer> RhiVk::createVertexBuffer(
    const std::shared_ptr<VertexStreamData>& streamData) const {
  if (!streamData) {
    return nullptr;
  }

  auto vertexBufferPtr = std::make_shared<VertexBufferVk>();
  vertexBufferPtr->initialize(streamData);
  return vertexBufferPtr;
}

std::shared_ptr<IndexBuffer> RhiVk::createIndexBuffer(
    const std::shared_ptr<IndexStreamData>& streamData) const {
  if (!streamData) {
    return nullptr;
  }

  assert(streamData);
  assert(streamData->m_stream_);
  auto indexBufferPtr = std::make_shared<IndexBufferVk>();
  indexBufferPtr->initialize(streamData);
  return indexBufferPtr;
}

std::shared_ptr<Texture> RhiVk::createTextureFromData(
    const ImageData* imageData) const {
  assert(imageData);

  int32_t MipLevel = imageData->m_mipLevel_;
  if ((MipLevel == 1) && (imageData->m_createMipmapIfPossible_)) {
    MipLevel
        = TextureVk::s_getMipLevels(imageData->m_width_, imageData->m_height_);
  }

  auto stagingBufferPtr = g_createBuffer(
      EVulkanBufferBits::TRANSFER_SRC,
      EVulkanMemoryBits::HOST_VISIBLE | EVulkanMemoryBits::HOST_COHERENT,
      imageData->m_imageBulkData_.m_imageData_.size(),
      EResourceLayout::TRANSFER_SRC);

  stagingBufferPtr->updateBuffer(
      &imageData->m_imageBulkData_.m_imageData_[0],
      imageData->m_imageBulkData_.m_imageData_.size());

  std::shared_ptr<TextureVk> TexturePtr;
  if (imageData->m_textureType_ == ETextureType::TEXTURE_CUBE) {
    TexturePtr = g_rhi->createCubeTexture<TextureVk>(
        (uint32_t)imageData->m_width_,
        (uint32_t)imageData->m_height_,
        MipLevel,
        imageData->m_format_,
        ETextureCreateFlag::TransferSrc | ETextureCreateFlag::TransferDst
            | ETextureCreateFlag::UAV,
        EResourceLayout::SHADER_READ_ONLY,
        imageData->m_imageBulkData_);
    if (!TexturePtr) {
      GlobalLogger::Log(LogLevel::Error, "Failed to create cube texture");
      return nullptr;
    }
  } else {
    TexturePtr = g_rhi->create2DTexture<TextureVk>(
        (uint32_t)imageData->m_width_,
        (uint32_t)imageData->m_height_,
        (uint32_t)imageData->m_layerCount_,
        MipLevel,
        imageData->m_format_,
        ETextureCreateFlag::TransferSrc | ETextureCreateFlag::TransferDst
            | ETextureCreateFlag::UAV,
        EResourceLayout::SHADER_READ_ONLY,
        imageData->m_imageBulkData_);
    if (!TexturePtr) {
      GlobalLogger::Log(LogLevel::Error, "Failed to create 2D texture");
      return nullptr;
    }
  }

  return TexturePtr;
}

bool RhiVk::createShaderInternal(Shader*           shader,
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
                 g_rhiVk->m_device_, &moduleCreateInfo, NULL, &shaderModule));
      delete[] shaderCode;

      return shaderModule;
    }
    return nullptr;
  };

  std::vector<Name> IncludeFilePaths;
  Shader*           shader_vk = shader;
  assert(shader_vk->getPermutationCount());
  {
    assert(!shader_vk->m_compiledShader);
    // TODO: check for memory leak, use smart pointer
    CompiledShaderVk* CurCompiledShader = new CompiledShaderVk();
    shader_vk->m_compiledShader         = CurCompiledShader;

    shader_vk->setPermutationId(shaderInfo.getPermutationId());

    std::string PermutationDefines;
    shader_vk->getPermutationDefines(PermutationDefines);

    VkShaderModule shaderModule{};

    const bool isSpirv
        = !!strstr(shaderInfo.getShaderFilepath().toStr(), ".spv");
    if (isSpirv) {
      shaderModule = LoadShaderFromSRV(shaderInfo.getShaderFilepath().toStr());
    } else {
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
        if (!IncludeShaderFile.openFile(
                includeFilepath.c_str(), FileType::Text, ReadWriteType::Read)) {
          return false;
        }
        IncludeShaderFile.readFileToBuffer(false);
        ShaderText.replace(
            ReplaceStartPos, ReplaceCount, IncludeShaderFile.getBuffer());
        IncludeShaderFile.closeFile();
      }

      // const wchar_t* ShadingModel = nullptr;
      // switch (shaderInfo.getShaderType()) {
      //   case EShaderAccessStageFlag::VERTEX:
      //     ShadingModel = Text("vs_6_6");
      //     break;
      //   case EShaderAccessStageFlag::GEOMETRY:
      //     ShadingModel = Text("gs_6_6");
      //     break;
      //   case EShaderAccessStageFlag::FRAGMENT:
      //     ShadingModel = Text("ps_6_6");
      //     break;
      //   case EShaderAccessStageFlag::COMPUTE:
      //     ShadingModel = Text("cs_6_6");
      //     break;
      //   case EShaderAccessStageFlag::RAYTRACING:
      //   case EShaderAccessStageFlag::RAYTRACING_RAYGEN:
      //   case EShaderAccessStageFlag::RAYTRACING_MISS:
      //   case EShaderAccessStageFlag::RAYTRACING_CLOSESTHIT:
      //   case EShaderAccessStageFlag::RAYTRACING_ANYHIT:
      //     ShadingModel = Text("lib_6_6");
      //     break;
      //   default:
      //     assert(0);
      //     break;
      // }

      // const std::wstring EntryPoint
      //     = g_convertToWchar(shaderInfo.getEntryPoint());
      // auto ShaderBlob = ShaderCompilerDx12::get().Compile(
      //     ShaderText.c_str(),
      //     (uint32_t)ShaderText.length(),
      //     ShadingModel,
      //     EntryPoint.c_str(),
      //     false,
      //     {Text("-spirv"),
      //      Text("-fspv-target-env=vulkan1.1spirv1.4"),
      //      Text("-fvk-use-scalar-layout"),
      //      Text("-fspv-extension=SPV_EXT_descriptor_indexing"),
      //      Text("-fspv-extension=SPV_KHR_ray_tracing"),
      //      Text("-fspv-extension=SPV_KHR_ray_query"),
      //      Text("-fspv-extension=SPV_EXT_shader_viewport_index_layer")});

      // std::vector<uint8_t> SpirvCode;
      // if (ShaderBlob->getBufferSize() > 0) {
      //   SpirvCode.resize(ShaderBlob->getBufferSize());
      //   memcpy(SpirvCode.data(),
      //          ShaderBlob->GetBufferPointer(),
      //          ShaderBlob->getBufferSize());
      // }

      auto SpirvCode = SpirvUtil::s_compileHlslCodeToSpirv(
          ShaderText,
          SpirvUtil::s_getShadercShaderKind(shaderInfo.getShaderType()),
          shaderInfo.getEntryPoint().toStr());

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
    switch (shaderInfo.getShaderType()) {
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

    CurCompiledShader->m_shaderStage_.sType
        = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    CurCompiledShader->m_shaderStage_.stage  = ShaderFlagBits;
    CurCompiledShader->m_shaderStage_.module = shaderModule;
    CurCompiledShader->m_shaderStage_.pName
        = shaderInfo.getEntryPoint().toStr();
  }
  shader_vk->m_shaderInfo_ = shaderInfo;
  shader_vk->m_shaderInfo_.setIncludeShaderFilePaths(IncludeFilePaths);

  return true;
}

FrameBuffer* RhiVk::createFrameBuffer(const FrameBufferInfo& info) const {
  const VkFormat textureFormat      = g_getVulkanTextureFormat(info.m_format_);
  const bool     hasDepthAttachment = s_isDepthFormat(info.m_format_);

  const VkImageUsageFlagBits ImageUsageFlagBit
      = hasDepthAttachment ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                           : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  const VkImageAspectFlagBits ImageAspectFlagBit
      = hasDepthAttachment ? VK_IMAGE_ASPECT_DEPTH_BIT
                           : VK_IMAGE_ASPECT_COLOR_BIT;

  const VkImageTiling TilingMode = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;

  const int32_t mipLevels = info.m_isGenerateMipmap ? TextureVk::s_getMipLevels(
                                info.m_extent_.width(), info.m_extent_.height())
                                                    : 1;

  assert(info.m_sampleCount_ >= 1);

  std::shared_ptr<TextureVk> TexturePtr;
  switch (info.m_textureType_) {
    case ETextureType::TEXTURE_2D:
      TexturePtr = game_engine::g_create2DTexture(
          info.m_extent_.width(),
          info.m_extent_.height(),
          mipLevels,
          (VkSampleCountFlagBits)info.m_sampleCount_,
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
      TexturePtr = game_engine::g_createTexture2DArray(
          info.m_extent_.width(),
          info.m_extent_.height(),
          info.m_layerCount_,
          mipLevels,
          (VkSampleCountFlagBits)info.m_sampleCount_,
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
      TexturePtr = game_engine::g_createCubeTexture(
          info.m_extent_.width(),
          info.m_extent_.height(),
          mipLevels,
          (VkSampleCountFlagBits)info.m_sampleCount_,
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

  auto fb_vk     = new FrameBufferVk();
  fb_vk->m_info_ = info;
  fb_vk->m_textures_.push_back(TexturePtr);
  fb_vk->m_allTextures.push_back(TexturePtr);  // TODO: consider to remove

  return fb_vk;
}

std::shared_ptr<RenderTarget> RhiVk::createRenderTarget(
    const RenderTargetInfo& info) const {
  const VkFormat textureFormat      = g_getVulkanTextureFormat(info.m_format_);
  const bool     hasDepthAttachment = s_isDepthFormat(info.m_format_);
  const int32_t  mipLevels
      = (info.m_sampleCount_ > EMSAASamples::COUNT_1
         || !info.m_isGenerateMipmap_)
          ? 1
          : TextureVk::s_getMipLevels(info.m_extent_.width(),
                                    info.m_extent_.height());
  assert((int32_t)info.m_sampleCount_ >= 1);

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

  if (info.m_isUseAsSubpassInput_) {
    TextureCreateFlag |= ETextureCreateFlag::SubpassInput;
  }

  if (info.m_isMemoryless_) {
    TextureCreateFlag |= ETextureCreateFlag::Memoryless;
  }

  const VkImageAspectFlags ImageAspectFlag = hasDepthAttachment
                                               ? VK_IMAGE_ASPECT_DEPTH_BIT
                                               : VK_IMAGE_ASPECT_COLOR_BIT;

  switch (info.m_rype_) {
    case ETextureType::TEXTURE_2D:
      TexturePtr = g_rhi->create2DTexture<TextureVk>(info.m_extent_.width(),
                                                     info.m_extent_.height(),
                                                     1,
                                                     mipLevels,
                                                     info.m_format_,
                                                     TextureCreateFlag,
                                                     EResourceLayout::GENERAL);
      // TODO: remove (currently not needed)
      /*imageViewForMipMap[0] = TexturePtr->imageView;

        assert(mipLevels > 0);
        for (int32_t i = 1; i < mipLevels; ++i) {
          imageViewForMipMap[i] = g_createTextureViewForSpecificMipMap(
              TexturePtr->image, textureFormat, ImageAspectFlag, i);
        }*/
      break;
    case ETextureType::TEXTURE_2D_ARRAY:
      TexturePtr = g_rhi->create2DTexture<TextureVk>(info.m_extent_.width(),
                                                     info.m_extent_.height(),
                                                     info.m_layerCount_,
                                                     mipLevels,
                                                     info.m_format_,
                                                     TextureCreateFlag,
                                                     EResourceLayout::GENERAL);
      // TODO: remove (currently not needed)
      /*imageViewForMipMap[0] = TexturePtr->imageView;

      assert(mipLevels > 0);
      for (int32_t i = 1; i < mipLevels; ++i) {
        imageViewForMipMap[i]
            = g_createTexture2DArrayViewForSpecificMipMap(TexturePtr->image,
                                                        info.LayerCount,
                                                        textureFormat,
                                                        ImageAspectFlag,
                                                        i);
      }*/
      break;
    case ETextureType::TEXTURE_CUBE:
      assert(info.m_layerCount_ == 6);
      TexturePtr
          = g_rhi->createCubeTexture<TextureVk>(info.m_extent_.width(),
                                                info.m_extent_.height(),
                                                mipLevels,
                                                info.m_format_,
                                                TextureCreateFlag,
                                                EResourceLayout::GENERAL);
      // TODO: remove (currently not needed)
      // Create for Shader Resource (TextureCube)
      //{
      //  imageViewForMipMap[0] = TexturePtr->imageView;

      //  assert(mipLevels > 0);
      //  for (int32_t i = 1; i < mipLevels; ++i) {
      //    imageViewForMipMap[i] = g_createTextureCubeViewForSpecificMipMap(
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
      //        = g_createTexture2DArrayViewForSpecificMipMap(TexturePtr->image,
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

  auto RenderTargetPtr           = std::make_shared<RenderTarget>();
  RenderTargetPtr->m_info_       = info;
  RenderTargetPtr->m_texturePtr_ = TexturePtr;

  return RenderTargetPtr;
}

// TODO: remove commented code (old version)
//  ///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  const VkFormat          textureFormat      = info.Format;
//  const bool              hasDepthAttachment = g_isDepthFormat(info.Format);
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
//          : TextureVk::s_getMipLevels(info.Extent.width(),
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
//      TexturePtr = g_create2DTexture(info.Extent.width(),
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
//        imageViewForMipMap[i] = g_createTextureViewForSpecificMipMap(
//            TexturePtr->image, textureFormat, ImageAspectFlag, i);
//      }*/
//      break;
//    case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
//
//      TexturePtr = g_create2DTexture(info.Extent.width(),
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
//            = g_createTexture2DArrayViewForSpecificMipMap(TexturePtr->image,
//                                                        info.LayerCount,
//                                                        textureFormat,
//                                                        ImageAspectFlag,
//                                                        i);
//      }*/
//      break;
//    case VK_IMAGE_VIEW_TYPE_CUBE:
//
//      assert(info.LayerCount == 6);
//      TexturePtr = g_createCubeTexture(info.Extent.width(),
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
//      //    imageViewForMipMap[i] = g_createTextureCubeViewForSpecificMipMap(
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
//      //        = g_createTexture2DArrayViewForSpecificMipMap(TexturePtr->image,
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

ShaderBindingLayout* RhiVk::createShaderBindings(
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

    // Try again, to avoid entering creation section simultaneously.
    auto it_find = s_shaderBindingPool.find(hash);
    if (s_shaderBindingPool.end() != it_find) {
      return it_find->second;
    }

    auto NewShaderBinding = new ShaderBindingLayoutVk();
    NewShaderBinding->initialize(shaderBindingArray);
    NewShaderBinding->m_hash_ = hash;
    s_shaderBindingPool[hash] = NewShaderBinding;

    return NewShaderBinding;
  }
}

std::shared_ptr<ShaderBindingInstance> RhiVk::createShaderBindingInstance(
    const ShaderBindingArray&       shaderBindingArray,
    const ShaderBindingInstanceType type) const {
  auto shaderBindingsLayout = createShaderBindings(shaderBindingArray);
  assert(shaderBindingsLayout);
  return shaderBindingsLayout->createShaderBindingInstance(shaderBindingArray,
                                                           type);
}

std::shared_ptr<IBuffer> RhiVk::createBufferInternal(
    uint64_t          size,
    uint64_t          alignment,
    EBufferCreateFlag bufferCreateFlag,
    EResourceLayout   initialState,
    const void*       data,
    uint64_t          dataSize
    /*, const wchar_t*    resourceName = nullptr*/) const {
  if (alignment > 0) {
    size = g_align(size, alignment);
  }

  EVulkanBufferBits BufferBits = EVulkanBufferBits::SHADER_DEVICE_ADDRESS
                               | EVulkanBufferBits::TRANSFER_DST
                               | EVulkanBufferBits::TRANSFER_SRC;
  if (!!(EBufferCreateFlag::UAV & bufferCreateFlag)) {
    BufferBits = BufferBits | EVulkanBufferBits::STORAGE_BUFFER;
  }

  if (!!(EBufferCreateFlag::AccelerationStructureBuildInput
         & bufferCreateFlag)) {
    // TODO: Ensure the extension for acceleration structure is enabled and
    // this flag is available in your Vulkan version
    BufferBits
        = BufferBits
        | EVulkanBufferBits::ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY;
  }

  if (!!(EBufferCreateFlag::AccelerationStructure & bufferCreateFlag)) {
    // TODO: Ensure the extension for acceleration structure is enabled and
    // this flag is available in your Vulkan version
    BufferBits = BufferBits | EVulkanBufferBits::ACCELERATION_STRUCTURE_STORAGE;
  }

  if (!!(EBufferCreateFlag::VertexBuffer & bufferCreateFlag)) {
    BufferBits = BufferBits | EVulkanBufferBits::VERTEX_BUFFER;
  }

  if (!!(EBufferCreateFlag::IndexBuffer & bufferCreateFlag)) {
    BufferBits = BufferBits | EVulkanBufferBits::INDEX_BUFFER;
  }

  if (!!(EBufferCreateFlag::IndirectCommand & bufferCreateFlag)) {
    BufferBits = BufferBits | EVulkanBufferBits::INDIRECT_BUFFER;
  }

  if (!!(EBufferCreateFlag::ShaderBindingTable & bufferCreateFlag)) {
    // TODO: Ensure the extension for ray tracing is enabled and this flag is
    // available in your Vulkan version
    BufferBits = BufferBits | EVulkanBufferBits::SHADER_BINDING_TABLE;
  }

  EVulkanMemoryBits MemoryBits = EVulkanMemoryBits::DEVICE_LOCAL;
  if (!!(EBufferCreateFlag::CPUAccess & bufferCreateFlag)) {
    MemoryBits
        |= EVulkanMemoryBits::HOST_VISIBLE | EVulkanMemoryBits::HOST_COHERENT;
  }

  auto bufferVk = g_createBuffer(BufferBits, MemoryBits, size, initialState);
  if (data && dataSize > 0) {
    if (!!(EBufferCreateFlag::CPUAccess & bufferCreateFlag)) {
      bufferVk->updateBuffer(data, dataSize);
    } else {
      auto stagingBufferPtr
          = g_rhi->createRawBuffer<BufferVk>(dataSize,
                                             alignment,
                                             EBufferCreateFlag::CPUAccess,
                                             EResourceLayout::TRANSFER_SRC,
                                             data,
                                             dataSize
                                             /*, "StagingBuffer"*/);
      g_copyBuffer(*stagingBufferPtr, *bufferVk, dataSize);
    }
  }
  return std::shared_ptr<BufferVk>(bufferVk);
}

std::shared_ptr<IUniformBufferBlock> RhiVk::createUniformBufferBlock(
    Name name, LifeTimeType lifeTimeType, size_t size) const {
  auto uniformBufferBlockPtr
      = std::make_shared<UniformBufferBlockVk>(name, lifeTimeType);
  uniformBufferBlockPtr->init(size);
  return uniformBufferBlockPtr;
}

// Create Images
VkImageUsageFlags RhiVk::getImageUsageFlags(
    ETextureCreateFlag textureCreateFlag) const {
  // TODO: remove this method (currently not needed)

  VkImageUsageFlags UsageFlag = VK_IMAGE_USAGE_SAMPLED_BIT;
  if (!!(textureCreateFlag & ETextureCreateFlag::RTV)) {
    UsageFlag |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  }

  if (!!(textureCreateFlag & ETextureCreateFlag::DSV)) {
    UsageFlag |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  }

  if (!!(textureCreateFlag & ETextureCreateFlag::UAV)) {
    UsageFlag |= VK_IMAGE_USAGE_STORAGE_BIT;
  }

  if (!!(textureCreateFlag & ETextureCreateFlag::TransferSrc)) {
    UsageFlag |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  }

  if (!!(textureCreateFlag & ETextureCreateFlag::TransferDst)) {
    UsageFlag |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  }

  if (!!(textureCreateFlag & ETextureCreateFlag::ShadingRate)) {
    UsageFlag |= VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
  }

  if (!!(textureCreateFlag & ETextureCreateFlag::SubpassInput)) {
    UsageFlag |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
  }

  // This should be placed last, because Memoryless has only Color, Depth,
  // Input attachment usages.
  if (!!(textureCreateFlag & ETextureCreateFlag::Memoryless)) {
    UsageFlag &= ~(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                   | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                   | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
  }

  return UsageFlag;
}

VkMemoryPropertyFlagBits RhiVk::getMemoryPropertyFlagBits(
    ETextureCreateFlag textureCreateFlag) const {
  // TODO: remove this method (currently not needed)

  VkMemoryPropertyFlagBits PropertyFlagBits{};
  if (!!(textureCreateFlag & ETextureCreateFlag::CPUAccess)) {
    PropertyFlagBits
        = (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                                     | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  }
  return PropertyFlagBits;
}

std::shared_ptr<Texture> RhiVk::create2DTexture(
    uint32_t             witdh,
    uint32_t             height,
    uint32_t             arrayLayers,
    uint32_t             mipLevels,
    ETextureFormat       format,
    ETextureCreateFlag   textureCreateFlag,
    EResourceLayout      imageLayout,
    const ImageBulkData& imageBulkData,
    const RtClearValue&  clearValue,
    const wchar_t*       resourceName) const {
  VkImageCreateFlagBits          ImageCreateFlags{};
  const VkMemoryPropertyFlagBits PropertyFlagBits
      = getMemoryPropertyFlagBits(textureCreateFlag);
  const VkImageUsageFlags UsageFlag = getImageUsageFlags(textureCreateFlag);
  assert(!s_isDepthFormat(format)
         || (s_isDepthFormat(format)
             && (UsageFlag & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)));

  auto TexturePtr = game_engine::g_create2DTexture(witdh,
                                                 height,
                                                 mipLevels,
                                                 VK_SAMPLE_COUNT_1_BIT,
                                                 g_getVulkanTextureFormat(format),
                                                 VK_IMAGE_TILING_OPTIMAL,
                                                 UsageFlag,
                                                 PropertyFlagBits,
                                                 ImageCreateFlags,
                                                 VK_IMAGE_LAYOUT_UNDEFINED);

  if (imageBulkData.m_imageData_.size() > 0) {
    // todo : recycle temp buffer
    auto stagingBufferPtr = g_createBuffer(
        EVulkanBufferBits::TRANSFER_SRC,
        EVulkanMemoryBits::HOST_VISIBLE | EVulkanMemoryBits::HOST_COHERENT,
        imageBulkData.m_imageData_.size(),
        EResourceLayout::TRANSFER_SRC);

    stagingBufferPtr->updateBuffer(&imageBulkData.m_imageData_[0],
                                   imageBulkData.m_imageData_.size());

    auto commandBuffer = beginSingleTimeCommands();
    assert(transitionLayout(
        commandBuffer, TexturePtr.get(), EResourceLayout::TRANSFER_DST));

    if (imageBulkData.m_subresourceFootprints_.size() > 0) {
      for (int32_t i = 0;
           i < (int32_t)imageBulkData.m_subresourceFootprints_.size();
           ++i) {
        const ImageSubResourceData SubResourceData
            = imageBulkData.m_subresourceFootprints_[i];

        g_copyBufferToTexture(
            commandBuffer->getRef(),
            stagingBufferPtr->m_buffer_,
            stagingBufferPtr->m_offset_ + SubResourceData.m_offset_,
            TexturePtr->m_image_,
            SubResourceData.m_width_,
            SubResourceData.m_height_,
            SubResourceData.m_mipLevel_,
            SubResourceData.m_depth_);
      }
    } else {
      g_copyBufferToTexture(commandBuffer->getRef(),
                          stagingBufferPtr->m_buffer_,
                          stagingBufferPtr->m_offset_,
                          TexturePtr->m_image_,
                          witdh,
                          height);
    }

    assert(transitionLayout(commandBuffer, TexturePtr.get(), imageLayout));

    endSingleTimeCommands(commandBuffer);
  }

  return TexturePtr;
}

std::shared_ptr<Texture> RhiVk::createCubeTexture(
    uint32_t             witdh,
    uint32_t             height,
    uint32_t             mipLevels,
    ETextureFormat       format,
    ETextureCreateFlag   textureCreateFlag,
    EResourceLayout      imageLayout,
    const ImageBulkData& imageBulkData,
    const RtClearValue&  clearValue,
    const wchar_t*       resourceName) const {
  VkImageCreateFlagBits ImageCreateFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
  const VkMemoryPropertyFlagBits PropertyFlagBits
      = getMemoryPropertyFlagBits(textureCreateFlag);
  const VkImageUsageFlags UsageFlag = getImageUsageFlags(textureCreateFlag);
  assert(!s_isDepthFormat(format)
         || (s_isDepthFormat(format)
             && (UsageFlag & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)));

  auto TexturePtr
      = game_engine::g_createCubeTexture(witdh,
                                       height,
                                       mipLevels,
                                       VK_SAMPLE_COUNT_1_BIT,
                                       g_getVulkanTextureFormat(format),
                                       VK_IMAGE_TILING_OPTIMAL,
                                       UsageFlag,
                                       PropertyFlagBits,
                                       ImageCreateFlags,
                                       VK_IMAGE_LAYOUT_UNDEFINED);

  if (imageBulkData.m_imageData_.size() > 0) {
    // todo : recycle temp buffer
    auto stagingBufferPtr = game_engine::g_createBuffer(
        EVulkanBufferBits::TRANSFER_SRC,
        EVulkanMemoryBits::HOST_VISIBLE | EVulkanMemoryBits::HOST_COHERENT,
        imageBulkData.m_imageData_.size(),
        EResourceLayout::TRANSFER_SRC);

    stagingBufferPtr->updateBuffer(&imageBulkData.m_imageData_[0],
                                   imageBulkData.m_imageData_.size());

    auto commandBuffer = beginSingleTimeCommands();
    assert(transitionLayout(
        commandBuffer, TexturePtr.get(), EResourceLayout::TRANSFER_DST));

    if (imageBulkData.m_subresourceFootprints_.size() > 0) {
      for (int32_t i = 0;
           i < (int32_t)imageBulkData.m_subresourceFootprints_.size();
           ++i) {
        const ImageSubResourceData SubResourceData
            = imageBulkData.m_subresourceFootprints_[i];

        g_copyBufferToTexture(
            commandBuffer->getRef(),
            stagingBufferPtr->m_buffer_,
            stagingBufferPtr->m_offset_ + SubResourceData.m_offset_,
            TexturePtr->m_image_,
            SubResourceData.m_width_,
            SubResourceData.m_height_,
            SubResourceData.m_mipLevel_,
            SubResourceData.m_depth_);
      }
    } else {
      g_copyBufferToTexture(commandBuffer->getRef(),
                          stagingBufferPtr->m_buffer_,
                          stagingBufferPtr->m_offset_,
                          TexturePtr->m_image_,
                          witdh,
                          height);
    }

    assert(transitionLayout(commandBuffer, TexturePtr.get(), imageLayout));

    endSingleTimeCommands(commandBuffer);
  }

  return TexturePtr;
}

void RhiVk::drawArrays(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext,
    /*EPrimitiveType                               type, - deprecated (used in
       previous rendering api)*/
    int32_t                                    vertStartIndex,
    int32_t                                    vertCount) const {
  assert(renderFrameContext);
  assert(renderFrameContext->getActiveCommandBuffer());
  vkCmdDraw((VkCommandBuffer)renderFrameContext->getActiveCommandBuffer()
                ->getNativeHandle(),
            vertCount,
            1,
            vertStartIndex,
            0);
}

void RhiVk::drawArraysInstanced(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext,
    /*EPrimitiveType                               type, - deprecated (used in
       previous rendering api)*/
    int32_t                                    vertStartIndex,
    int32_t                                    vertCount,
    int32_t                                    instanceCount) const {
  assert(renderFrameContext);
  assert(renderFrameContext->getActiveCommandBuffer());
  vkCmdDraw((VkCommandBuffer)renderFrameContext->getActiveCommandBuffer()
                ->getNativeHandle(),
            vertCount,
            instanceCount,
            vertStartIndex,
            0);
}

void RhiVk::drawElements(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext,
    /*EPrimitiveType                               type, - deprecated (used in
       previous rendering api)*/
    int32_t                                    elementSize,
    int32_t                                    startIndex,
    int32_t                                    indexCount) const {
  assert(renderFrameContext);
  assert(renderFrameContext->getActiveCommandBuffer());
  vkCmdDrawIndexed((VkCommandBuffer)renderFrameContext->getActiveCommandBuffer()
                       ->getNativeHandle(),
                   indexCount,
                   1,
                   startIndex,
                   0,
                   0);
}

void RhiVk::drawElementsInstanced(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext,
    /*EPrimitiveType                               type, - deprecated (used in
       previous rendering api)*/
    int32_t                                    elementSize,
    int32_t                                    startIndex,
    int32_t                                    indexCount,
    int32_t                                    instanceCount) const {
  assert(renderFrameContext);
  assert(renderFrameContext->getActiveCommandBuffer());
  vkCmdDrawIndexed((VkCommandBuffer)renderFrameContext->getActiveCommandBuffer()
                       ->getNativeHandle(),
                   indexCount,
                   instanceCount,
                   startIndex,
                   0,
                   0);
}

void RhiVk::drawElementsBaseVertex(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext,
    /*EPrimitiveType                               type, - deprecated (used in
       previous rendering api)*/
    int32_t                                    elementSize,
    int32_t                                    startIndex,
    int32_t                                    indexCount,
    int32_t                                    baseVertexIndex) const {
  assert(renderFrameContext);
  assert(renderFrameContext->getActiveCommandBuffer());
  vkCmdDrawIndexed((VkCommandBuffer)renderFrameContext->getActiveCommandBuffer()
                       ->getNativeHandle(),
                   indexCount,
                   1,
                   startIndex,
                   baseVertexIndex,
                   0);
}

void RhiVk::drawElementsInstancedBaseVertex(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext,
    /*EPrimitiveType                               type, - deprecated (used in
       previous rendering api)*/
    int32_t                                    elementSize,
    int32_t                                    startIndex,
    int32_t                                    indexCount,
    int32_t                                    baseVertexIndex,
    int32_t                                    instanceCount) const {
  assert(renderFrameContext);
  assert(renderFrameContext->getActiveCommandBuffer());
  vkCmdDrawIndexed((VkCommandBuffer)renderFrameContext->getActiveCommandBuffer()
                       ->getNativeHandle(),
                   indexCount,
                   instanceCount,
                   startIndex,
                   baseVertexIndex,
                   0);
}

void RhiVk::drawIndirect(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext,
    /*EPrimitiveType                               type, - deprecated (used in
       previous rendering api)*/
    IBuffer*                                    buffer,
    int32_t                                    startIndex,
    int32_t                                    drawCount) const {
  assert(renderFrameContext);
  assert(renderFrameContext->getActiveCommandBuffer());
  vkCmdDrawIndirect(
      (VkCommandBuffer)renderFrameContext->getActiveCommandBuffer()
          ->getNativeHandle(),
      (VkBuffer)buffer->getHandle(),
      startIndex * sizeof(VkDrawIndirectCommand),
      drawCount,
      sizeof(VkDrawIndirectCommand));
}

void RhiVk::drawElementsIndirect(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext,
    /*EPrimitiveType                               type, - deprecated (used in
       previous rendering api)*/
    IBuffer*                                    buffer,
    int32_t                                    startIndex,
    int32_t                                    drawCount) const {
  assert(renderFrameContext);
  assert(renderFrameContext->getActiveCommandBuffer());
  vkCmdDrawIndexedIndirect(
      (VkCommandBuffer)renderFrameContext->getActiveCommandBuffer()
          ->getNativeHandle(),
      (VkBuffer)buffer->getHandle(),
      startIndex * sizeof(VkDrawIndexedIndirectCommand),
      drawCount,
      sizeof(VkDrawIndexedIndirectCommand));
}

void RhiVk::dispatchCompute(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext,
    uint32_t                                   numGroupsX,
    uint32_t                                   numGroupsY,
    uint32_t                                   numGroupsZ) const {
  assert(renderFrameContext);
  assert(renderFrameContext->getActiveCommandBuffer());
  vkCmdDispatch((VkCommandBuffer)renderFrameContext->getActiveCommandBuffer()
                    ->getNativeHandle(),
                numGroupsX,
                numGroupsY,
                numGroupsZ);
}

void RhiVk::flush() const {
  // TODO: Implement the Flush method to submit pending commands to the GPU.
  // Currently, it's identical to Finish method and waits for all queues to be
  // idle, which is a blocking operation. Ideally, Flush should only ensure
  // that commands have been submitted to the GPU for execution without
  // waiting for their completion. This might involve managing command buffers
  // and submissions more explicitly to track what has been submitted but not
  // yet completed.
  vkQueueWaitIdle(m_graphicsQueue_.m_queue_);
  vkQueueWaitIdle(m_computeQueue_.m_queue_);
  vkQueueWaitIdle(m_presentQueue_.m_queue_);
}

void RhiVk::finish() const {
  vkQueueWaitIdle(m_graphicsQueue_.m_queue_);
  vkQueueWaitIdle(m_computeQueue_.m_queue_);
  vkQueueWaitIdle(m_presentQueue_.m_queue_);
}

// TODO: check whether it's called
void RhiVk::recreateSwapChain() {
  flush();

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
  m_commandBufferManager_->createPool(m_graphicsQueue_.m_queueIndex_);

  FrameBufferPool::s_release();
  RenderTargetPool::s_release();
  s_pipelineStatePool.release();
  s_renderPassPool.release();

  // delete m_swapchain_;
  // m_swapchain_ = new m_swapchain_();
  m_swapchain_->create(m_window_);

  flush();
}

std::shared_ptr<RenderFrameContext> RhiVk::beginRenderFrame() {
  VkResult acquireNextImageResult = vkAcquireNextImageKHR(
      m_device_,
      (VkSwapchainKHR)m_swapchain_->getHandle(),
      UINT64_MAX,
      (VkSemaphore)m_swapchain_->getSwapchainImage(m_currentFrameIndex_)
          ->m_available_->getHandle(),
      VK_NULL_HANDLE,
      &m_currentFrameIndex_);

  VkFence lastCommandBufferFence
      = m_swapchain_->getSwapchainImage(m_currentFrameIndex_)
            ->m_commandBufferFence_;

  if (acquireNextImageResult != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error,
                      "Failed to acquire next image from swapchain.");
    return nullptr;
  }

  if (acquireNextImageResult == VK_ERROR_OUT_OF_DATE_KHR) {
    GlobalLogger::Log(LogLevel::Warning,
                      "Swapchain is out of date. Recreating swapchain.");
    recreateSwapChain();
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
            g_rhiVk->m_commandBufferManager_->getOrCreateCommandBuffer();

  if (lastCommandBufferFence != VK_NULL_HANDLE) {
    vkWaitForFences(m_device_, 1, &lastCommandBufferFence, VK_TRUE, UINT64_MAX);
  }

  getOneFrameUniformRingBuffer()->reset();
  getDescriptorPoolForSingleFrame()->reset();

  m_swapchain_->getSwapchainImage(m_currentFrameIndex_)->m_commandBufferFence_
      = (VkFence)commandBuffer->getFenceHandle();

  auto renderFrameContextPtr
      = std::make_shared<RenderFrameContextVk>(commandBuffer);
  // renderFrameContextPtr->UseForwardRenderer =
  // !gOptions.UseDeferredRenderer;
  renderFrameContextPtr->m_frameIndex_ = m_currentFrameIndex_;
  renderFrameContextPtr->m_sceneRenderTargetPtr_
      = std::make_shared<SceneRenderTarget>();
  renderFrameContextPtr->m_sceneRenderTargetPtr_->create(
      m_window_, m_swapchain_->getSwapchainImage(m_currentFrameIndex_));
  renderFrameContextPtr->m_currentWaitSemaphore_
      = m_swapchain_->getSwapchainImage(m_currentFrameIndex_)->m_available_;

  return renderFrameContextPtr;
}

void RhiVk::endRenderFrame(
    const std::shared_ptr<RenderFrameContext>& renderFrameContextPtr) {
  VkSemaphore signalSemaphore[]
      = {(VkSemaphore)m_swapchain_->getSwapchainImage(m_currentFrameIndex_)
             ->m_renderFinished_->getHandle()};
  queueSubmit(
      renderFrameContextPtr,
      m_swapchain_->getSwapchainImage(m_currentFrameIndex_)->m_renderFinished_);

  VkPresentInfoKHR presentInfo   = {};
  presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores    = signalSemaphore;

  VkSwapchainKHR swapChains[] = {(VkSwapchainKHR)m_swapchain_->getHandle()};
  presentInfo.swapchainCount  = 1;
  presentInfo.pSwapchains     = swapChains;
  presentInfo.pImageIndices   = &m_currentFrameIndex_;

  presentInfo.pResults = nullptr;  // Optional
  VkResult queuePresentResult
      = vkQueuePresentKHR(m_presentQueue_.m_queue_, &presentInfo);

  m_currentFrameIndex_
      = (m_currentFrameIndex_ + 1) % m_swapchain_->getNumOfSwapchainImages();
  renderFrameContextPtr->destroy();

  if ((queuePresentResult == VK_ERROR_OUT_OF_DATE_KHR)
      || (queuePresentResult == VK_SUBOPTIMAL_KHR) || m_isFramebufferResized_) {
    recreateSwapChain();
    m_isFramebufferResized_ = false;
    return;
  } else if (queuePresentResult != VK_SUCCESS) {
    assert(0);
    return;
  }
}

void RhiVk::queueSubmit(
    const std::shared_ptr<RenderFrameContext>& renderFrameContextPtr,
    ISemaphore*                                 signalSemaphore) {
  auto renderFrameContext = (RenderFrameContextVk*)renderFrameContextPtr.get();
  assert(renderFrameContext);

  assert(renderFrameContext->getActiveCommandBuffer());

  renderFrameContext->getActiveCommandBuffer()->end();

  VkCommandBuffer vkCommandBuffer
      = (VkCommandBuffer)renderFrameContext->getActiveCommandBuffer()
            ->getNativeHandle();
  VkFence vkFence
      = (VkFence)renderFrameContext->getActiveCommandBuffer()->getFenceHandle();

  // Submitting the command buffer
  VkSubmitInfo submitInfo = {};
  submitInfo.sType        = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitsemaphores[]
      = {(VkSemaphore)renderFrameContext->m_currentWaitSemaphore_->getHandle()};
  VkPipelineStageFlags waitStages[]
      = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores    = waitsemaphores;
  submitInfo.pWaitDstStageMask  = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &vkCommandBuffer;

  renderFrameContext->m_currentWaitSemaphore_ = signalSemaphore;

  VkSemaphore signalSemaphores[]
      = {(VkSemaphore)signalSemaphore->getHandle()};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores    = signalSemaphores;

  vkResetFences(m_device_, 1, &vkFence);

  auto queueSubmitResult
      = vkQueueSubmit(m_graphicsQueue_.m_queue_, 1, &submitInfo, vkFence);
  if ((queueSubmitResult == VK_ERROR_OUT_OF_DATE_KHR)
      || (queueSubmitResult == VK_SUBOPTIMAL_KHR)) {
    GlobalLogger::Log(LogLevel::Warning,
                      "Swapchain out of date or suboptimal, recreating...");
    recreateSwapChain();
  } else if (queueSubmitResult != VK_SUCCESS) {
    GlobalLogger::Log(LogLevel::Error, "Failed to submit queue");
  }
}

CommandBufferVk* RhiVk::beginSingleTimeCommands() const {
  return (CommandBufferVk*)m_commandBufferManager_->getOrCreateCommandBuffer();

  // VkCommandBufferAllocateInfo allocInfo = {};
  // allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  // allocInfo.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  // allocInfo.commandPool = m_commandBufferManager->getPool();
  // allocInfo.commandBufferCount = 1;

  // VkCommandBuffer commandBuffer;
  // vkAllocateCommandBuffers(m_device_, &allocInfo, &commandBuffer);

  // VkCommandBufferBeginInfo beginInfo = {};
  // beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  // beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  // vkBeginCommandBuffer(commandBuffer, &beginInfo);
  // return commandBuffer;
}

void RhiVk::endSingleTimeCommands(CommandBuffer* commandBuffer) const {
  // vkEndCommandBuffer(commandBuffer);

  // VkSubmitInfo submitInfo       = {};
  // submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  // submitInfo.commandBufferCount = 1;
  // submitInfo.pCommandBuffers    = &commandBuffer;

  // vkQueueSubmit(m_graphicsQueue_.Queue, 1, &submitInfo, VK_NULL_HANDLE);
  // vkQueueWaitIdle(m_graphicsQueue_.Queue);

  // vkFreeCommandBuffers(
  //     m_device_, m_commandBufferManager->getPool(), 1, &commandBuffer);

  auto commandBufferVk = (CommandBufferVk*)commandBuffer;

  commandBufferVk->end();

  VkSubmitInfo submitInfo       = {};
  submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &commandBufferVk->getRef();

  VkFence vkFence = (VkFence)commandBufferVk->getFenceHandle();

  vkResetFences(m_device_, 1, &vkFence);

  auto Result
      = vkQueueSubmit(m_graphicsQueue_.m_queue_, 1, &submitInfo, vkFence);
  assert(VK_SUCCESS == Result);

  Result = vkQueueWaitIdle(m_graphicsQueue_.m_queue_);
  assert(VK_SUCCESS == Result);

  m_commandBufferManager_->returnCommandBuffer(commandBufferVk);
}

void RhiVk::bindGraphicsShaderBindingInstances(
    const CommandBuffer*                 commandBuffer,
    const PipelineStateInfo*             piplineState,
    const ShaderBindingInstanceCombiner& shaderBindingInstanceCombiner,
    uint32_t                             firstSet) const {
  if (shaderBindingInstanceCombiner.m_descriptorSetHandles_.m_numOfData_) {
    assert(commandBuffer);
    vkCmdBindDescriptorSets(
        (VkCommandBuffer)commandBuffer->getNativeHandle(),
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        (VkPipelineLayout)piplineState->getPipelineLayoutHandle(),
        firstSet,
        shaderBindingInstanceCombiner.m_descriptorSetHandles_.m_numOfData_,
        (const VkDescriptorSet*)&shaderBindingInstanceCombiner
            .m_descriptorSetHandles_[0],
        shaderBindingInstanceCombiner.m_dynamicOffsets_.m_numOfData_,
        (shaderBindingInstanceCombiner.m_dynamicOffsets_.m_numOfData_
             ? &shaderBindingInstanceCombiner.m_dynamicOffsets_[0]
             : nullptr));
  }
}

bool RhiVk::transitionLayout(VkCommandBuffer commandBuffer,
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
      if (g_isDepthOnlyFormat(format)) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
      } else if (g_isDepthFormat(format)) {
        barrier.subresourceRange.aspectMask
            = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
      } else {
        assert(0);
      }
      break;
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
    case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
      if (g_isDepthOnlyFormat(format)) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
      } else {
        assert(0);
      }
      break;
    default:
      if (g_isDepthOnlyFormat(format)) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
      } else if (g_isDepthFormat(format)) {
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

  // m_target_ layouts (new)
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

bool RhiVk::transitionLayout(CommandBuffer*  commandBuffer,
                             Texture*        texture,
                             EResourceLayout newLayout) const {
  assert(commandBuffer);
  assert(texture);

  if (texture->getLayout() == newLayout) {
    return true;
  }

  auto texture_vk = (TextureVk*)texture;
  if (texture_vk->isDepthFormat()
      && (EResourceLayout::DEPTH_READ_ONLY == newLayout
          || EResourceLayout::SHADER_READ_ONLY == newLayout)) {
    newLayout = EResourceLayout::DEPTH_STENCIL_READ_ONLY;
  }

  if (transitionLayout((VkCommandBuffer)commandBuffer->getNativeHandle(),
                       texture_vk->m_image_,
                       g_getVulkanTextureFormat(texture_vk->m_format_),
                       texture_vk->m_mipLevels_,
                       texture_vk->m_layerCount_,
                       g_getVulkanImageLayout(texture_vk->m_imageLayout_),
                       g_getVulkanImageLayout(newLayout))) {
    ((TextureVk*)texture)->m_imageLayout_ = newLayout;
    return true;
  }
  return true;
}

void RhiVk::bindComputeShaderBindingInstances(
    const CommandBuffer*                 commandBuffer,
    const PipelineStateInfo*             piplineState,
    const ShaderBindingInstanceCombiner& shaderBindingInstanceCombiner,
    uint32_t                             firstSet) const {
  if (shaderBindingInstanceCombiner.m_descriptorSetHandles_.m_numOfData_) {
    assert(commandBuffer);
    vkCmdBindDescriptorSets(
        (VkCommandBuffer)commandBuffer->getNativeHandle(),
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        (VkPipelineLayout)piplineState->getPipelineLayoutHandle(),
        firstSet,
        shaderBindingInstanceCombiner.m_descriptorSetHandles_.m_numOfData_,
        (const VkDescriptorSet*)&shaderBindingInstanceCombiner
            .m_descriptorSetHandles_[0],
        shaderBindingInstanceCombiner.m_dynamicOffsets_.m_numOfData_,
        (shaderBindingInstanceCombiner.m_dynamicOffsets_.m_numOfData_
             ? &shaderBindingInstanceCombiner.m_dynamicOffsets_[0]
             : nullptr));
  }
}

void RhiVk::nextSubpass(const CommandBuffer* commandBuffer) const {
  assert(commandBuffer);
  vkCmdNextSubpass((VkCommandBuffer)commandBuffer->getNativeHandle(),
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
  createInfo.pfnUserCallback = g_debugCallback;
}

bool RhiVk::isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(device, &deviceProperties);

  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

  QueueFamilyIndices indices = g_findQueueFamilies(device, surface);

  bool extensionsSupported = checkDeviceExtensionSupport(device);

  bool swapChainAdequate = false;
  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport
        = g_querySwapChainSupport(device, surface);
    swapChainAdequate = !swapChainSupport.m_formats_.empty()
                     && !swapChainSupport.m_presentModes_.empty();
  }

  return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool RhiVk::checkDeviceExtensionSupport(VkPhysicalDevice device) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(
      device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(
      device, nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredExtensions(kDeviceExtensions.begin(),
                                           kDeviceExtensions.end());
  for (const auto& extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

}  // namespace game_engine
