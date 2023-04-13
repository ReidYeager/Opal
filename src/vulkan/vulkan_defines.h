
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
  uint32_t queueIndexPresent;
} OvkGpu_T;

typedef struct OvkFrame_T
{
  uint32_t swapchainImageIndex;

  VkFence fenceFrameAvailable;
  VkSemaphore semImageAvailable;
  VkSemaphore semRenderComplete;

  VkCommandBuffer cmd;
} OvkFrame_T;

typedef struct OvkState_T
{
  uint32_t isHeadless;

  VkInstance instance;
  VkSurfaceKHR surface;
  OvkGpu_T gpu;
  VkDevice device;

  VkCommandPool graphicsCommandPool;
  VkCommandPool transientCommantPool;
  VkQueue queueGraphics;
  VkQueue queuePresent;
  VkQueue queueTransfer;

  VkDescriptorPool descriptorPool;

  struct {
    VkSwapchainKHR swapchain;
    uint32_t imageCount;
    VkImage* images;
    VkImageView* imageViews;
    VkSurfaceFormatKHR format;
    VkExtent2D extents;
    VkPresentModeKHR presentMode;
  } swapchain;

  uint32_t currentFrameSlotIndex;
  uint32_t frameSlotCount;
  OvkFrame_T* frameSlots;

  VkRenderPass renderpass;
  VkFramebuffer* framebuffers;
} OvkState_T;

typedef struct OvkShader_T
{
  VkShaderModule module;
} OvkShader_T;

typedef struct OvkMaterial_T
{
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorSet descriptorSet;
} OvkMaterial_T;

typedef struct OvkBuffer_T
{
  VkBuffer buffer;
  VkDeviceMemory memory;
  // Usage?
} OvkBuffer_T;

#define maxFlightSlotCount 3

#endif // !GEM_OPAL_VULKAN_DEFINES_H
