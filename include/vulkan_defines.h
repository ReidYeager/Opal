#ifndef OPAL_VULKAN_DEFINES_H
#define OPAL_VULKAN_DEFINES_H 1

#include <vulkan/vulkan.h>

// Material
// ============================================================

typedef struct OpalVulkanShader
{
  VkShaderModule module;
} OpalVulkanShader;

typedef struct OpalVulkanShaderGroup
{
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;
} OpalVulkanShaderGroup;

typedef struct OpalVulkanShaderInput
{
  VkDescriptorSet set;
} OpalVulkanShaderInput;

// Objects
// ============================================================

typedef struct OpalVulkanBuffer
{
  VkBuffer buffer;
} OpalVulkanBuffer;

typedef struct OpalVulkanImage
{
  VkDeviceMemory memory;
  VkImage image;
  VkImageView view;
  VkSampler sampler;

  VkFormat format;
  VkImageLayout layout;
} OpalVulkanImage;

// Rendering
// ============================================================

typedef struct OpalVulkanFramebuffer
{
  VkFramebuffer framebuffer;
} OpalVulkanFramebuffer;

typedef struct OpalVulkanRenderpass
{
  VkRenderPass renderpass;
  VkClearValue* pClearValues;
} OpalVulkanRenderpass;

// Window
// ============================================================

typedef struct OpalVulkanWindow
{
  VkSurfaceKHR surface;

  VkFormat format;
  VkPresentModeKHR presentMode;
  VkSwapchainKHR swapchain;

  uint8_t imageCount;
  VkImage* pImages;
  VkImageView* pImageViews;
  VkSampler* pSamplers;

  VkFence* pFencesImageAvailable;
  VkSemaphore* pSemaphoresImageAvailable;
  VkSemaphore* pSemaphoresRenderComplete;
} OpalVulkanWindow;

// State
// ============================================================

typedef struct OpalVulkanGpuInfo
{
  VkPhysicalDevice device;
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceMemoryProperties memoryProperties;

  uint32_t queueFamilyPropertiesCount;
  VkQueueFamilyProperties* pQueueFamilyProperties;

  uint32_t queueIndexGraphics;
  uint32_t queueIndexTransfer;
  uint32_t queueIndexPresent;
} OpalVulkanGpuInfo;

typedef struct OpalVulkanState
{
  VkInstance instance;
  OpalVulkanGpuInfo gpu;
  VkDevice device;

  VkQueue queueGraphics;
  VkQueue queueTransfer;
  VkQueue queuePresent;

  VkCommandPool transientCommandPool;
  VkCommandPool graphicsCommandPool;

  VkDescriptorPool descriptorPool;
} OpalVulkanState;

#endif // !OPAL_VULKAN_DEFINES_H
