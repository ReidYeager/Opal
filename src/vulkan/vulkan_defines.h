
#ifndef GEM_OPAL_VULKAN_DEFINES_H
#define GEM_OPAL_VULKAN_DEFINES_H

#include <vulkan/vulkan.h>

#include <stdint.h>

// =====
// Buffer
// =====

typedef struct OvkBuffer_T
{
  VkBuffer buffer;
  VkDeviceMemory memory;
  // Usage?
} OvkBuffer_T;

// =====
// Image
// =====

typedef struct OvkImage_T
{
  VkFormat format;
  VkImageLayout layout;

  VkImage image;
  VkDeviceMemory memory;
  VkImageView view;
  VkSampler sampler;
} OvkImage_T;

// =====
// Material
// =====

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

// =====
// Mesh
// =====

typedef struct OvkMesh_T
{
  VkDescriptorSet descriptorSet;
} OvkMesh_T;

// =====
// Renderable
// =====

typedef struct OvkRenderable_T
{
  VkDescriptorSet descSet;
} OvkRenderable_T;

typedef enum OvkRenderpassAttachmentLoadOp {
  Ovk_Attachment_LoadOp_Clear,
  Ovk_Attachment_LoadOp_Load,
  Ovk_Attachment_LoadOp_Dont_Care
} OvkRenderpassAttachmentLoadOp;

typedef enum OvkRenderpassAttachmentUsage {
  Ovk_Attachment_Usage_Color,
  Ovk_Attachment_Usage_Depth,
  Ovk_Attachment_Usage_Presented
} OvkRenderpassAttachmentUsage;

typedef struct OvkRenderpassAttachment {
  VkFormat dataFormat;
  OvkRenderpassAttachmentUsage usage;

  OvkRenderpassAttachmentLoadOp loadOperation;
  uint8_t shouldStoreReneredData;
} OvkRenderpassAttachment;

typedef struct OvkCreateRenderpassInfo {
  uint32_t attachmentCount;
  OvkRenderpassAttachment* attachments;

  uint32_t subpassCount;
  OpalRenderpassSubpass* subpasses;
} OvkCreateRenderpassInfo;

typedef struct OvkRenderpass_T {
  VkRenderPass renderpass;

  // Will be one for each swapchain image if shoulRenderToFramebuffer, otherwise only one
  VkFramebuffer* framebuffers;
  uint32_t framebufferCount;
} OvkRenderpass_T;

// =====
// Core
// =====

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

    VkImage* depthImages;
    VkDeviceMemory* depthImageMemories;
    VkImageView* depthImageViews;
    VkFormat depthFormat;
  } swapchain;

  uint32_t currentFrameSlotIndex;
  uint32_t frameSlotCount;
  OvkFrame_T* frameSlots;

  VkDescriptorSetLayout objectSetLayout;
} OvkState_T;

#define maxFlightSlotCount 3

#endif // !GEM_OPAL_VULKAN_DEFINES_H
