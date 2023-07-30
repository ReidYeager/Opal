
#ifndef GEM_OPAL_LOCAL_COMMON_H_
#define GEM_OPAL_LOCAL_COMMON_H_ 1

#include "src/defines.h"
#include <vulkan/vulkan.h>
#include <stdio.h>

#define OpalLog(message, ...)              \
{                                          \
  printf("Opal :: " message, __VA_ARGS__); \
}

#define OPAL_ATTEMPT(fn, ...)                                     \
{                                                                 \
  OpalResult oResult = (fn);                                      \
  if (oResult)                                                    \
  {                                                               \
    OpalLog("Function \""#fn"\" failed. Result = %d\n", oResult); \
    OpalLog("    %s:%d\n", __FILE__, __LINE__);                   \
    { __VA_ARGS__; }                                              \
    return Opal_Failure;                                          \
  }                                                               \
}

#define OVK_ATTEMPT(fn, ...)                                             \
{                                                                        \
  VkResult vResult = (fn);                                               \
  if (vResult != VK_SUCCESS)                                             \
  {                                                                      \
    OpalLog("Vulkan function \""#fn"\" failed. Result = %d\n", vResult); \
    OpalLog("    %s:%d\n", __FILE__, __LINE__);                          \
    { __VA_ARGS__; }                                                     \
    return Opal_Failure;                                                 \
  }                                                                      \
}

OpalResult OvkInit();
void OvkShutdown();

OpalResult OvkBeginSingleUseCommand(VkCommandPool _pool, VkCommandBuffer* _cmd);
OpalResult OvkEndSingleUseCommand(VkCommandPool _pool, VkQueue _queue, VkCommandBuffer _cmd);

OpalResult OvkWindowInit(OpalWindow_T* _window);
OpalResult OvkWindowReinit(OpalWindow_T* _window);
OpalResult OvkWindowShutdown(OpalWindow_T* _window);

OpalResult OvkImageInit(OpalImage_T* _image, OpalImageInitInfo const* _initInfo);
void OvkImageShutdown(OpalImage_T* _image);
OpalResult OvkImageResize(OpalImage_T* _image, OpalExtent _extents);
OpalResult OvkTransitionImageLayout(VkImage _image, VkImageLayout _layout, bool _toWritable);

OpalResult OvkRenderpassInit(OpalRenderpass_T* _renderpass, OpalRenderpassInitInfo _initInfo);
void OvkRenderpassShutdown(OpalRenderpass_T* _renderpass);

OpalResult OvkFramebufferInit(OpalFramebuffer_T* _framebuffer, OpalFramebufferInitInfo const* _initInfo);
OpalResult OvkFramebufferReinit(OpalFramebuffer_T _framebuffer);
void OvkFramebufferShutdown(OpalFramebuffer_T* _framebuffer);

OpalResult OvkShaderInit(OpalShader_T* _shader, OpalShaderInitInfo const* _initInfo);
void OvkShaderShutdown(OpalShader_T* _shader);

OpalResult OvkMaterialInit(OpalMaterial_T* _material, OpalMaterialInitInfo const* _initInfo);
void OvkMaterialShutdown(OpalMaterial_T* _material);

OpalResult OvkRenderBegin();
OpalResult OvkRenderEnd();
void OvkRenderBeginRenderpass(OpalRenderpass _renderpass, OpalFramebuffer _framebuffer);
void OvkRenderEndRenderpass(OpalRenderpass _renderpass);
void OvkRenderBindMaterial(OpalMaterial _material);
void OvkRenderVertices(uint32_t _count); // Tmp. Replace with mesh render



#endif // !GEM_OPAL_LOCAL_COMMON_H_
