#include "gfx/rhi/rhi_new/backends/vulkan/device_utils_vk.h"

#include <set>

namespace game_engine {
namespace gfx {
namespace rhi {

VKAPI_ATTR VkBool32 VKAPI_CALL g_debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                             VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                             void*                                       pUserData) {
  LogLevel logLevel;

  if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    logLevel = LogLevel::Error;
  } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    logLevel = LogLevel::Warning;
  } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
    logLevel = LogLevel::Info;
  } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
    logLevel = LogLevel::Trace;
  } else {
    logLevel = LogLevel::Debug;
  }

  std::string typeStr;
  if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
    typeStr += "GENERAL ";
  }
  if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
    typeStr += "VALIDATION ";
  }
  if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
    typeStr += "PERFORMANCE ";
  }
  if (typeStr.empty()) {
    typeStr = "UNKNOWN";
  }

  std::string message = "Vulkan [" + typeStr + "] ";

  // Add message ID if available
  if (pCallbackData->pMessageIdName) {
    message += "[" + std::string(pCallbackData->pMessageIdName) + "] ";
  }

  // Actual message
  message += pCallbackData->pMessage;

  // Add object information if available
  if (pCallbackData->objectCount > 0) {
    message += "\nObjects involved:";
    for (uint32_t i = 0; i < pCallbackData->objectCount; i++) {
      const auto& obj  = pCallbackData->pObjects[i];
      message         += "\n  - Type: " + std::to_string(obj.objectType);
      if (obj.pObjectName) {
        message += ", Name: " + std::string(obj.pObjectName);
      }
    }
  }

  GlobalLogger::Log(logLevel, message);

  return VK_FALSE;
}

void g_populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
  createInfo       = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity
      = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
#ifdef _DEBUG
  // Include verbose and info messages in debug builds
  createInfo.messageSeverity
      |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
#endif
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                         | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = g_debugCallback;
  createInfo.pUserData       = nullptr;
}

QueueFamilyIndices g_findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

  // Find queue families that support graphics, compute, and present
  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    // Graphics queue
    if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
    }

    // Compute queue (preferably separate from graphics)
    if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
      if (!indices.computeFamily.has_value()
          || (indices.graphicsFamily.has_value() && i != indices.graphicsFamily.value())) {
        indices.computeFamily = i;
      }
    }

    // Transfer queue (preferably separate from graphics and compute)
    if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
      if (!indices.transferFamily.has_value()
          || (indices.graphicsFamily.has_value() && i != indices.graphicsFamily.value()
              && indices.computeFamily.has_value() && i != indices.computeFamily.value())) {
        indices.transferFamily = i;
      }
    }

    // Present queue
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

    if (presentSupport) {
      indices.presentFamily = i;
    }

    // Exit early if we found all queue families
    if (indices.isComplete()) {
      break;
    }
  }

  // If we didn't find a dedicated compute queue, use the graphics queue
  if (!indices.computeFamily.has_value() && indices.graphicsFamily.has_value()) {
    indices.computeFamily = indices.graphicsFamily;
  }

  // If we didn't find a dedicated transfer queue, use the compute or graphics queue
  if (!indices.transferFamily.has_value()) {
    if (indices.computeFamily.has_value()) {
      indices.transferFamily = indices.computeFamily;
    } else if (indices.graphicsFamily.has_value()) {
      indices.transferFamily = indices.graphicsFamily;
    }
  }

  return indices;
}

bool g_checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

  for (const auto& extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

SwapChainSupportDetails g_querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
  SwapChainSupportDetails details;

  // Get surface capabilities
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

  // Get surface formats
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

  if (formatCount != 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
  }

  // Get presentation modes
  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
  }

  return details;
}

bool g_isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions) {
  // Check if device supports required queue families
  QueueFamilyIndices indices = g_findQueueFamilies(device, surface);

  // Check if device supports required extensions
  bool extensionsSupported = g_checkDeviceExtensionSupport(device, deviceExtensions);

  // Check if swap chain is adequate
  bool swapChainAdequate = false;
  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport = g_querySwapChainSupport(device, surface);
    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
  }

  // Get device features
  VkPhysicalDeviceFeatures supportedFeatures;
  vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

  // Check for required features (e.g., sampler anisotropy)
  bool featuresSupported = supportedFeatures.samplerAnisotropy;

  return indices.isComplete() && extensionsSupported && swapChainAdequate && featuresSupported;
}

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine