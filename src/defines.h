
#ifndef GEM_OPAL_LOCAL_DEFINES_H_
#define GEM_OPAL_LOCAL_DEFINES_H_ 1

#include "include/opal.h"

#include <vulkan/vulkan.h>

#include <stdbool.h>
#include <stdio.h>

#define OpalLog(message, ...)              \
{                                          \
  printf("Opal :: " message, __VA_ARGS__); \
}

#define OPAL_ATTEMPT(fn)                \
{                                       \
  OpalResult oResult = (fn);            \
  if (oResult)                          \
  {                                     \
    OpalLog("Failure : %d\n", oResult); \
    return Opal_Failure;                \
  }                                     \
}

#define OVK_ATTEMPT(fn)                       \
{                                             \
  VkResult vResult = (fn);                    \
  if (vResult != VK_SUCCESS)                  \
  {                                           \
    OpalLog("Vulkan failed : %d\n", vResult); \
    return Opal_Failure;                      \
  }                                           \
}

typedef struct OvkSync_T
{
  VkFence fenceFrameAvailable;
  VkSemaphore semImageAvailable;
  VkSemaphore semRenderComplete;
} OvkSync_T;

typedef struct OpalWindow_T
{
  LapisWindow* lWindow;

  uint32_t imageCount;
  struct
  {
    uint32_t width;
    uint32_t height;
  } extents;

  struct
  {
    VkSurfaceKHR surface;
    VkPresentModeKHR presentMode;
    VkFormat format;
    struct
    {
      VkSwapchainKHR swapchain;
      VkImage* pImages;
      VkImageView* pViews;
    }swapchain;

    OvkSync_T* pSync;
  }vk;
} OpalWindow_T;

typedef struct OpalVkGpu_T
{
  VkPhysicalDevice device;
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceMemoryProperties memoryProperties;

  uint32_t queueFamilyPropertiesCount;
  VkQueueFamilyProperties* queueFamilyProperties;

  uint32_t queueIndexGraphics;
  uint32_t queueIndexTransfer;
  uint32_t queueIndexPresent;
} OpalVkGpu_T;

typedef struct OpalState_T
{
  OpalWindow_T window;

  struct
  {
    const VkAllocationCallbacks* allocator;
    VkInstance instance;
    VkPhysicalDevice gpu;
    OpalVkGpu_T gpuInfo;
    VkDevice device;

    VkQueue queueGraphics;
    VkQueue queueTransfer;
    VkQueue queuePresent;

    VkCommandPool transientCommandPool;
    VkCommandPool graphicsCommandPool;

    VkDescriptorPool descriptorPool;
  } vk;
} OpalState_T;
extern OpalState_T oState;

#endif // !GEM_OPAL_LOCAL_DEFINES_H_
