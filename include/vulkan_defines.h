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
  VkSemaphore*     pImageAvailableSems;
  VkSemaphore*     pRenderCompleteSems;
} OpalVulkanWindow;

// Synchronization
// ============================================================

typedef struct OpalVulkanFence
{
  VkFence fence;
} OpalVulkanFence;

typedef struct OpalVulkanSemaphore
{
  VkSemaphore semaphore;
} OpalVulkanSemaphore;

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

  uint32_t queueIndexGraphicsCompute;
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
  VkQueue queueCompute;
  VkQueue queueTransfer;
  VkQueue queuePresent;

  VkCommandPool transientCommandPool;
  VkCommandPool graphicsCommandPool;

  struct
  {
    uint32_t cmdCount;
    VkCommandBuffer* pCmdBuffers;
    VkFence* pCmdAvailableFence;

    uint32_t curIndex;
    VkCommandBuffer curCmd;
    VkFence curFence;

    VkPipelineBindPoint bindPoint;

    VkPipelineLayout layout;
    uint32_t pushConstSize;
  } renderState;

  struct
  {
    uint32_t count;

    uint32_t graphicsHead;
    VkCommandBuffer* pGraphicsCmds;
    VkFence* pGraphicsFences;

    uint32_t transferHead;
    VkCommandBuffer* pTransferCmds;
    VkFence* pTransferFences;
  } singleUse;

  VkDescriptorPool descriptorPool;
} OpalVulkanState;

#endif // !OPAL_VULKAN_DEFINES_H
