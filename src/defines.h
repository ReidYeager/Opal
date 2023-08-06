
#ifndef GEM_OPAL_LOCAL_DEFINES_H_
#define GEM_OPAL_LOCAL_DEFINES_H_ 1

#include "include/opal.h"

#include "src/define_buffer.h"
#include "src/define_framebuffer.h"
#include "src/define_image.h"
#include "src/define_material.h"
#include "src/define_renderpass.h"
#include "src/define_window.h"

#include <vulkan/vulkan.h>

#include <stdbool.h>

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

typedef struct OpalVkState_T
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
} OpalVkState_T;

typedef struct OpalState_T
{
  OpalWindow_T window;
  OpalVkState_T vk;

} OpalState_T;
extern OpalState_T oState;

#endif // !GEM_OPAL_LOCAL_DEFINES_H_
