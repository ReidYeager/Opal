
#ifndef GEM_OPAL_VULKAN_DEFINES_H
#define GEM_OPAL_VULKAN_DEFINES_H

#include <vulkan/vulkan.h>

#include <stdint.h>

typedef struct OvkGpu_T
{
  VkPhysicalDevice device;
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceMemoryProperties memoryProperties;

  uint32_t queueFamilyPropertiesCount;
  VkQueueFamilyProperties* queueFamilyProperties;

  uint32_t queueIndexGraphics;
  uint32_t queueIndexTransfer;
} OvkGpu_T;

typedef struct OvkState_T
{
  VkInstance instance;
  VkSurfaceKHR surface;
  OvkGpu_T gpu;
  VkDevice device;

  VkCommandPool graphicsCommandPool;
  VkCommandPool transientCommantPool;

  struct {
    VkSwapchainKHR swapchain;
    uint32_t imageCount;
    VkImage* images;
    VkImageView* imageViews;
    VkSurfaceFormatKHR format;
    VkExtent2D extents;
    VkPresentModeKHR presentMode;
  } swapchain;

} OvkState_T;

#endif // !GEM_OPAL_VULKAN_DEFINES_H
