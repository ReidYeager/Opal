
#ifndef GEM_OPAL_LOCAL_COMMON_H_
#define GEM_OPAL_LOCAL_COMMON_H_ 1

#include "include/opal.h"
#include <vulkan/vulkan.h>
#include <stdio.h>

#ifdef GEM_LAPIS
#define OpalLog(message, ...) LapisConsolePrintMessage(Lapis_Console_Info, "Opal :: " message, __VA_ARGS__)
#else
#define OpalLog(message, ...) printf("Opal :: " message, __VA_ARGS__)
#endif // GEM_LAPIS

#ifdef GEM_LAPIS
#define OpalLogError(message, ...) LapisConsolePrintMessage(Lapis_Console_Error, "Opal :: Err :: " message, __VA_ARGS__)
#else
#define OpalLogError(message, ...) printf("Opal :: Err :: " message, __VA_ARGS__)
#endif // GEM_LAPIS

#define OPAL_ATTEMPT(fn, ...)                                          \
{                                                                      \
  OpalResult oResult = (fn);                                           \
  if (oResult)                                                         \
  {                                                                    \
    OpalLogError("Function \""#fn"\" failed. Result = %d\n", oResult); \
    OpalLogError("    %s:%d\n", __FILE__, __LINE__);                   \
    { __VA_ARGS__; }                                                   \
    return Opal_Failure;                                               \
  }                                                                    \
  else {}                                                              \
}

#define OVK_ATTEMPT(fn, ...)                                                  \
{                                                                             \
  VkResult vResult = (fn);                                                    \
  if (vResult != VK_SUCCESS)                                                  \
  {                                                                           \
    OpalLogError("Vulkan function \""#fn"\" failed. Result = %d\n", vResult); \
    OpalLogError("    %s:%d\n", __FILE__, __LINE__);                          \
    { __VA_ARGS__; }                                                          \
    return Opal_Failure;                                                      \
  }                                                                           \
  else {}                                                                     \
}

VkFormat OpalFormatToVkFormat_Ovk(OpalFormat _format);

OpalResult OvkInit(OpalInitInfo _initInfo);
void OvkShutdown();

OpalResult OvkWaitIdle();

OpalResult OvkBufferInit(OpalBuffer _buffer, OpalBufferInitInfo _initInfo);
void OvkBufferShutdown(OpalBuffer _buffer);
OpalResult OvkBufferPushData(OpalBuffer _buffer, void* _data);
OpalResult OvkBufferPushDataSegment(OpalBuffer _buffer, void* _data, uint32_t size, uint32_t offset);
uint32_t OvkBufferDumpData(OpalBuffer buffer, void** data);

OpalResult OvkWindowInit(OpalWindow_T* _window, OpalWindowInitInfo _initInfo);
OpalResult OvkWindowReinit(OpalWindow_T* _window);
OpalResult OvkWindowShutdown(OpalWindow_T* _window);

void OpalPlatformGetRequiredExtensions(uint32_t* _extensionCount, const char** _extensionNames);
OpalResult OpalPlatformCreateSurface(OpalWindowPlatformInfo_T _window, VkInstance _instance, VkSurfaceKHR* _surface);

OpalResult OvkImageInit(OpalImage_T* _image, OpalImageInitInfo _initInfo);
void OvkImageShutdown(OpalImage_T* _image);
OpalResult OvkImageResize(OpalImage_T* _image, OpalExtent _extents);
OpalResult OvkImageFill(OpalImage_T* _image, void* _data);
uint32_t OvkImageDumpData(OpalImage image, void** data);
OpalResult OvkTransitionImageLayout(VkImage _image, VkImageLayout _layout, VkImageLayout _newLayout);

OpalResult OvkRenderpassInit(OpalRenderpass_T* _renderpass, OpalRenderpassInitInfo _initInfo);
void OvkRenderpassShutdown(OpalRenderpass_T* _renderpass);

OpalResult OvkFramebufferInit(OpalFramebuffer_T* _framebuffer, OpalFramebufferInitInfo _initInfo);
OpalResult OvkFramebufferReinit(OpalFramebuffer_T* _framebuffer);
void OvkFramebufferShutdown(OpalFramebuffer_T* _framebuffer);

OpalResult OvkShaderInit(OpalShader_T* _shader, OpalShaderInitInfo _initInfo);
void OvkShaderShutdown(OpalShader_T* _shader);

OpalResult OvkInputLayoutInit(OpalInputLayout_T* _layout, OpalInputLayoutInitInfo _initInfo);
void OvkInputLayoutShutdown(OpalInputLayout_T* _layout);
OpalResult OvkInputSetInit(OpalInputSet_T* _set, OpalInputSetInitInfo _initInfo);
void OvkInputSetShutdown(OpalInputSet_T* _set);
OpalResult OvkInputSetUpdate(OpalInputSet _set, uint32_t _count, OpalInputInfo* _pInputs);

OpalResult OvkMaterialInit(OpalMaterial_T* _material, OpalMaterialInitInfo _initInfo);
void OvkMaterialShutdown(OpalMaterial_T* _material);
OpalResult OvkMaterialReinit(OpalMaterial_T* _material);

OpalResult OvkRenderBegin(OpalWindow _window);
OpalResult OvkRenderEnd();
VkCommandBuffer OvkRenderGetCommandBuffer();
void OvkRenderBeginRenderpass(OpalRenderpass _renderpass, OpalFramebuffer _framebuffer);
void OvkRenderEndRenderpass(OpalRenderpass _renderpass);
void OvkRenderBindInputSet(OpalInputSet _set, uint32_t _setIndex);
void OvkRenderBindMaterial(OpalMaterial _material);
void OvkRenderMesh(OpalMesh _mesh);
void OvkRenderSetPushConstant(void* _data);

#endif // !GEM_OPAL_LOCAL_COMMON_H_
