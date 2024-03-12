#ifndef OPAL_VULKAN_DEFINES_H
#define OPAL_VULKAN_DEFINES_H 1

#include <stdbool.h>
#include <vulkan/vulkan.h>

// Shaders
// ============================================================

typedef struct OpalVulkanShader
{
  VkShaderModule module;
  VkShaderStageFlagBits stage;
} OpalVulkanShader;

typedef struct OpalVulkanShaderGroup
{
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;
} OpalVulkanShaderGroup;

typedef struct OpalVulkanShaderInputLayout
{
  VkDescriptorSetLayout layout;
} OpalVulkanShaderInputLayout;

typedef struct OpalVulkanShaderInput
{
  VkDescriptorSet set;
} OpalVulkanShaderInput;

// Objects
// ============================================================

typedef struct OpalVulkanBuffer
{
  VkBuffer buffer;
  VkDeviceMemory memory;
} OpalVulkanBuffer;

typedef struct OpalVulkanImage
{
  VkDeviceMemory memory;
  VkImage image;
  VkImageView view;
  VkSampler sampler;

  VkFormat format;
  VkImageLayout layout;

  VkImageAspectFlags aspectFlags;
} OpalVulkanImage;

// Rendering
// ============================================================

struct OpalImage;

typedef struct OpalVulkanFramebuffer
{
  VkFramebuffer framebuffer;
  struct OpalImage** pImagePointers;
} OpalVulkanFramebuffer;

typedef struct OpalVulkanRenderpass
{
  VkRenderPass renderpass;
  VkClearValue* pClearValues;
  VkImageLayout* pFinalLayouts;
  uint32_t* subpassColorOutputCount;
} OpalVulkanRenderpass;

// Window
// ============================================================

typedef struct OpalVulkanWindow
{
  VkSurfaceKHR surface;

  VkFormat format;
  VkPresentModeKHR presentMode;
  VkSwapchainKHR swapchain;

  uint8_t      imageCount;
  VkImage*     pImages;
  VkImageView* pImageViews;
  VkSampler*   pSamplers;

  // Synchronization
  uint32_t         imageIndex;
  uint32_t         syncIndex;
  VkFence*         pCmdAvailableFences;
  VkSemaphore*     pImageAvailableSems;
  VkSemaphore*     pRenderCompleteSems;
  VkCommandBuffer* pCommandBuffers;
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

  struct
  {
    uint32_t attribCount;
    VkVertexInputAttributeDescription* pAttribDescriptions;
    VkVertexInputBindingDescription bindingDescription;
  } vertex;

  VkQueue queueGraphics;
  VkQueue queueTransfer;
  VkQueue queuePresent;

  VkCommandPool transientCommandPool;
  VkCommandPool graphicsCommandPool;

  struct
  {
    VkCommandBuffer cmd;
    VkPipelineLayout layout;
    uint32_t pushConstSize;
  } renderState;

  VkCommandBuffer renderCmd;
  VkFence renderCompleteFence;

  VkDescriptorPool descriptorPool;
} OpalVulkanState;

#endif // !OPAL_VULKAN_DEFINES_H
