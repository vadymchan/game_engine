#ifndef GAME_ENGINE_DEVICE_UTILS_VK_H
#define GAME_ENGINE_DEVICE_UTILS_VK_H

#include "utils/logger/global_logger.h"

#include <vulkan/vulkan.h>

#include <optional>
#include <string>
#include <vector>

namespace game_engine {
namespace gfx {
namespace rhi {

// queue families available on a physical device
struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;
  std::optional<uint32_t> computeFamily;
  std::optional<uint32_t> transferFamily;

  bool isComplete() const {
    return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value();
  }
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR        capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR>   presentModes;
};

// Debug callback for validation layer messages
VKAPI_ATTR VkBool32 VKAPI_CALL g_debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                             VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                             void*                                       pUserData);

// Sets up the debug messenger creation info structure
void g_populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

// Finds queue families for a physical device
QueueFamilyIndices g_findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

// Checks if a device supports the required extensions
bool g_checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions);

// Checks if a device is suitable for the application
bool g_isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);

// Queries the swap chain support details for a device
SwapChainSupportDetails g_querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_DEVICE_UTILS_VK_H