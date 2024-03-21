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
OpalResult        OpalVulkanInit                       (OpalInitInfo initInfo);
void              OpalVulkanShutdown                   ();
void              OpalVulkanWaitIdle                   ();
OpalResult        TransferBufferInit_Ovk               ();
void              TransferBufferShutdown_Ovk           ();

// Window ==========
OpalResult        OpalVulkanWindowInit                 (OpalWindow* pWindow, OpalWindowInitInfo initInfo);
void              OpalVulkanWindowShutdown             (OpalWindow* pWindow);

// Buffer ==========
OpalResult        OpalVulkanBufferInit                 (OpalBuffer* pBuffer, OpalBufferInitInfo initInfo);
void              OpalVulkanBufferShutdown             (OpalBuffer* pBuffer);
OpalResult        OpalVulkanBufferPushData             (OpalBuffer* pBuffer, const void* data);
OpalResult        OpalVulkanBufferPushDataSegment      (OpalBuffer* pBuffer, const void* data, uint64_t size, uint64_t offset);
OpalResult        OpalVulkanBufferDumpData             (OpalBuffer* pBuffer, void* outData);
OpalResult        OpalVulkanBufferDumpDataSegment      (OpalBuffer* pBuffer, void* outData, uint64_t size, uint64_t offset);

// Image ==========
OpalResult        OpalVulkanImageInit                  (OpalImage* pImage, OpalImageInitInfo initInfo);
void              OpalVulkanImageShutdown              (OpalImage* pImage);
OpalResult        ImageTransitionLayout_Ovk            (OpalImage* pImage, VkImageLayout newLayout);
VkImageUsageFlags OpalImageUsageToVkFlags_Ovk          (OpalImageUsageFlags opalFlags, OpalFormat format);
OpalResult        OpalVulkanImagePushData              (OpalImage* pImage, const void* data);
OpalResult        OpalVulkanImageCopyImage             (OpalImage* pImage, OpalImage* pSourceImage, OpalImageFilterType filter);
OpalResult        OpalVulkanImageGetMipAsImage         (OpalImage* pImage, OpalImage* pMipImage, uint32_t mipLevel);

// Renderpass ==========
OpalResult        OpalVulkanRenderpassInit             (OpalRenderpass* pRenderpass, OpalRenderpassInitInfo initInfo);
void              OpalVulkanRenderpassShutdown         (OpalRenderpass* pRenderpass);

// Framebuffer ==========
OpalResult        OpalVulkanFramebufferInit            (OpalFramebuffer* pFramebuffer, OpalFramebufferInitInfo initInfo);
void              OpalVulkanFramebufferShutdown        (OpalFramebuffer* pFramebuffer);

// Shader ==========
OpalResult        OpalVulkanShaderInit                 (OpalShader* pShader, OpalShaderInitInfo initInfo);
void              OpalVulkanShaderShutdown             (OpalShader* pShader);

// ShaderGroup ==========
OpalResult        OpalVulkanShaderGroupInit            (OpalShaderGroup* pShaderGroup, OpalShaderGroupInitInfo initInfo);
void              OpalVulkanShaderGroupShutdown        (OpalShaderGroup* pShaderGroup);

// ShaderInput* ==========
OpalResult        OpalVulkanShaderInputLayoutInit      (OpalShaderInputLayout* pLayout, OpalShaderInputLayoutInitInfo initInfo);
void              OpalVulkanShaderInputLayoutShutdown  (OpalShaderInputLayout* pLayout);
OpalResult        OpalVulkanShaderInputInit            (OpalShaderInput* pShaderInput, OpalShaderInputInitInfo initInfo);
void              OpalVulkanShaderInputShutdown        (OpalShaderInput* pShaderInput);

// Synchronization ==========
OpalResult         OpalVulkanFenceInit                 (OpalFence* pFence, bool startSignaled);
void               OpalVulkanFenceShutdown             (OpalFence* pFence);
OpalResult         OpalVulkanSemaphoreInit             (OpalSemaphore* pSemaphore);
void               OpalVulkanSemaphoreShutdown         (OpalSemaphore* pSemaphore);

// Rendering ==========
// Rendering - Begin/End
OpalResult        OpalVulkanRenderBegin                ();
OpalResult        OpalVulkanRenderEnd                  (OpalSyncPack syncInfo);
OpalResult        OpalVulkanRenderToWindowBegin        (OpalWindow* pWindow);
OpalResult        OpalVulkanRenderToWindowEnd          (OpalWindow* pWindow);
// Rendering - Objects
void              OpalVulkanRenderRenderpassBegin      (const OpalRenderpass* pRenderpass, const OpalFramebuffer* pFramebuffer);
void              OpalVulkanRenderRenderpassEnd        (const OpalRenderpass* pRenderpass);
void              OpalVulkanRenderRenderpassNext       (const OpalRenderpass* pRenderpass);
void              OpalVulkanRenderBindShaderGroup      (const OpalShaderGroup* pGroup);
void              OpalVulkanRenderSetViewportDimensions(uint32_t width, uint32_t height);
void              OpalVulkanRenderSetPushConstant      (const void* data);
void              OpalVulkanRenderBindShaderInput      (const OpalShaderInput* pInput, uint32_t setIndex);
void              OpalVulkanRenderMesh                 (const OpalMesh* pMesh);

void              OpalVulkanComputeDispatch            (uint32_t x, uint32_t y, uint32_t z);

// Platform
// ============================================================

uint32_t          PlatformGetExtensions_Ovk            (const char*** pOutExtensions);
OpalResult        PlatformCreateSurface_Ovk            (OpalPlatformWindowInfo windowInfo, VkSurfaceKHR* outSurface);

// Tools
// ============================================================

VkFormat           OpalFormatToVkFormat_Ovk            (OpalFormat _format);
OpalFormat         VkFormatToOpalFormat_Ovk            (VkFormat _format);
VkShaderStageFlags OpalStagesToVkStages_Ovk            (OpalStageFlags stages);
OpalResult         SingleUseCmdBeginGraphics_Ovk       (VkCommandBuffer* pCmd);
OpalResult         SingleUseCmdEndGraphics_Ovk         (VkCommandBuffer  pCmd);
OpalResult         SingleUseCmdBeginTransfer_Ovk       (VkCommandBuffer* pCmd);
OpalResult         SingleUseCmdEndTransfer_Ovk         (VkCommandBuffer  pCmd);


#endif // !OPAL_VULKAN_COMMON_H
