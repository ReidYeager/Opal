#ifndef OPAL_VULKAN_COMMON_H
#define OPAL_VULKAN_COMMON_H 1

#include "src/common.h"
#include "include/vulkan_defines.h"

#include <vulkan/vulkan.h>

// Preprocessor
// ============================================================

#define OPAL_ATTEMPT_VK(fn, ...)                                            \
{                                                                           \
  VkResult vResult = (fn);                                                  \
  if (vResult != VK_SUCCESS)                                                \
  {                                                                         \
    OpalLogError("Vulkan function \""#fn"\" failed. Result = %d", vResult); \
    OpalLogError("    %s:%d\n", __FILE__, __LINE__);                        \
    { __VA_ARGS__; }                                                        \
    return Opal_Failure_Api;                                                \
  }                                                                         \
}

#define OPAL_ATTEMPT_VK_FAIL_LOG(message, ...)     \
{                                                  \
  OpalLogError("Vulkan :: " message, __VA_ARGS__); \
  OpalLogError("    %s:%d\n", __FILE__, __LINE__); \
}

// Global variables
// ============================================================

extern OpalVulkanState* g_ovkState;

// Wrappers
// ============================================================

// Core ==========
OpalResult OpalVulkanInit                 (OpalInitInfo initInfo);
void       OpalVulkanShutdown             ();
void       OpalVulkanWaitIdle             ();
OpalResult TransferBufferInit_Ovk         ();
void       TransferBufferShutdown_Ovk     ();

// Window ==========
OpalResult OpalVulkanWindowInit           (OpalWindow* pWindow, OpalWindowInitInfo initInfo);
void       OpalVulkanWindowShutdown       (OpalWindow* pWindow);

// Buffer ==========
OpalResult OpalVulkanBufferInit           (OpalBuffer* pBuffer, OpalBufferInitInfo initInfo);
void       OpalVulkanBufferShutdown       (OpalBuffer* pBuffer);
OpalResult OpalVulkanBufferPushData       (OpalBuffer* pBuffer, void* data);
OpalResult OpalVulkanBufferPushDataSegment(OpalBuffer* pBuffer, void* data, uint64_t size, uint64_t offset);

// Image ==========
OpalResult OpalVulkanImageInit            (OpalImage* pImage, OpalImageInitInfo initInfo);
void       OpalVulkanImageShutdown        (OpalImage* pImage);
OpalResult ImageTransitionLayout_Ovk      (OpalImage* pImage, VkImageLayout newLayout);

// Renderpass ==========
OpalResult OpalVulkanRenderpassInit       (OpalRenderpass* pRenderpass, OpalRenderpassInitInfo initInfo);
void       OpalVulkanRenderpassShutdown   (OpalRenderpass* pRenderpass);

// Framebuffer ==========
OpalResult OpalVulkanFramebufferInit      (OpalFramebuffer* pFramebuffer, OpalFramebufferInitInfo initInfo);
void       OpalVulkanFramebufferShutdown  (OpalFramebuffer* pFramebuffer);

// Shader ==========
OpalResult OpalVulkanShaderInit           (OpalShader* pShader, OpalShaderInitInfo initInfo);
void       OpalVulkanShaderShutdown       (OpalShader* pShader);

// ShaderGroup ==========
OpalResult OpalVulkanShaderGroupInit      (OpalShaderGroup* pShaderGroup, OpalShaderGroupInitInfo initInfo);
void       OpalVulkanShaderGroupShutdown  (OpalShaderGroup* pShaderGroup);

// ShaderInput ==========
OpalResult OpalVulkanShaderInputInit      (OpalShaderInput* pShaderInput, OpalShaderInputInitInfo initInfo);
void       OpalVulkanShaderInputShutdown  (OpalShaderInput* pShaderInput);

// Rendering ==========
OpalResult OpalVulkanRenderBegin          ();
OpalResult OpalVulkanRenderEnd            ();
OpalResult OpalVulkanRenderToWindowBegin  (OpalWindow* pWindow);
OpalResult OpalVulkanRenderToWindowEnd    (OpalWindow* pWindow);
void       OpalVulkanRenderRenderpassBegin(const OpalRenderpass* pRenderpass, const OpalFramebuffer* pFramebuffer);
void       OpalVulkanRenderRenderpassEnd  (const OpalRenderpass* pRenderpass);

// Platform
// ============================================================

uint32_t   PlatformGetExtensions_Ovk(const char*** pOutExtensions);
OpalResult PlatformCreateSurface_Ovk(OpalPlatformWindowInfo windowInfo, VkSurfaceKHR* outSurface);

// Tools
// ============================================================

VkFormat           OpalFormatToVkFormat_Ovk       (OpalFormat _format);
OpalFormat         VkFormatToOpalFormat_Ovk       (VkFormat _format);
VkShaderStageFlags OpalStagesToVkStages_Ovk       (OpalStageFlags stages);
OpalResult         BeginSingleUseCommandBuffer_Ovk(VkCommandBuffer* pCmd, VkCommandPool pool);
OpalResult         EndSingleUseCommandBuffer_Ovk  (VkCommandBuffer cmd, VkCommandPool pool, VkQueue submissionQueue);


#endif // !OPAL_VULKAN_COMMON_H
