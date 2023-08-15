
#ifndef GEM_OPAL_LOCAL_COMMON_H_
#define GEM_OPAL_LOCAL_COMMON_H_ 1

#include "src/defines.h"
#include <vulkan/vulkan.h>
#include <stdio.h>

#define OpalLog(message, ...)              \
{                                          \
  printf("Opal :: " message, __VA_ARGS__); \
}

#define OpalLogErr(message, ...)                  \
{                                                 \
  printf("Opal :: Err :: " message, __VA_ARGS__); \
}

#define OPAL_ATTEMPT(fn, ...)                                        \
{                                                                    \
  OpalResult oResult = (fn);                                         \
  if (oResult)                                                       \
  {                                                                  \
    OpalLogErr("Function \""#fn"\" failed. Result = %d\n", oResult); \
    OpalLogErr("    %s:%d\n", __FILE__, __LINE__);                   \
    { __VA_ARGS__; }                                                 \
    return Opal_Failure;                                             \
  }                                                                  \
}

#define OVK_ATTEMPT(fn, ...)                                                \
{                                                                           \
  VkResult vResult = (fn);                                                  \
  if (vResult != VK_SUCCESS)                                                \
  {                                                                         \
    OpalLogErr("Vulkan function \""#fn"\" failed. Result = %d\n", vResult); \
    OpalLogErr("    %s:%d\n", __FILE__, __LINE__);                          \
    { __VA_ARGS__; }                                                        \
    return Opal_Failure;                                                    \
  }                                                                         \
}

VkFormat OpalFormatToVkFormat_Ovk(OpalFormat _format);
uint32_t OpalFormatToSize(OpalFormat _format);

OpalResult OvkInit(OpalInitInfo _initInfo);
void OvkShutdown();

OpalResult OvkBufferInit(OpalBuffer _buffer, OpalBufferInitInfo _initInfo);
void OvkBufferShutdown(OpalBuffer _buffer);
OpalResult OvkBufferPushData(OpalBuffer _buffer, void* _data);

OpalResult OvkBeginSingleUseCommand(VkCommandPool _pool, VkCommandBuffer* _cmd);
OpalResult OvkEndSingleUseCommand(VkCommandPool _pool, VkQueue _queue, VkCommandBuffer _cmd);

OpalResult OvkWindowInit(OpalWindow_T* _window);
OpalResult OvkWindowReinit(OpalWindow_T* _window);
OpalResult OvkWindowShutdown(OpalWindow_T* _window);

OpalResult OvkImageInit(OpalImage_T* _image, OpalImageInitInfo _initInfo);
void OvkImageShutdown(OpalImage_T* _image);
OpalResult OvkImageResize(OpalImage_T* _image, OpalExtent _extents);
OpalResult OvkImageFill(OpalImage_T* _image, void* _data);
OpalResult OvkTransitionImageLayout(VkImage _image, VkImageLayout _layout, VkImageLayout _newLayout);

OpalResult OvkRenderpassInit(OpalRenderpass_T* _renderpass, OpalRenderpassInitInfo _initInfo);
void OvkRenderpassShutdown(OpalRenderpass_T* _renderpass);

OpalResult OvkFramebufferInit(OpalFramebuffer_T* _framebuffer, OpalFramebufferInitInfo _initInfo);
OpalResult OvkFramebufferReinit(OpalFramebuffer_T* _framebuffer);
void OvkFramebufferShutdown(OpalFramebuffer_T* _framebuffer);

OpalResult OvkShaderInit(OpalShader_T* _shader, OpalShaderInitInfo _initInfo);
void OvkShaderShutdown(OpalShader_T* _shader);

OpalResult OvkInputSetInit(OpalInputSet_T* _set, OpalInputSetInitInfo _initInfo);
void OvkInputSetShutdown(OpalInputSet_T* _set);
OpalResult OvkInputSetUpdate(OpalInputSet _set, uint32_t _count, OpalInputInfo* _pInputs);

OpalResult OvkMaterialInit(OpalMaterial_T* _material, OpalMaterialInitInfo _initInfo);
void OvkMaterialShutdown(OpalMaterial_T* _material);
OpalResult OvkMaterialReinit(OpalMaterial_T* _material);

OpalResult OvkRenderBegin();
OpalResult OvkRenderEnd();
void OvkRenderBeginRenderpass(OpalRenderpass _renderpass, OpalFramebuffer _framebuffer);
void OvkRenderEndRenderpass(OpalRenderpass _renderpass);
void OvkRenderBindInputSet(OpalInputSet _set, uint32_t _setIndex);
void OvkRenderBindMaterial(OpalMaterial _material);
void OvkRenderMesh(OpalMesh _mesh);

#endif // !GEM_OPAL_LOCAL_COMMON_H_
