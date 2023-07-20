
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

OpalResult OvkWindowInit(OpalWindow_T* _window);
OpalResult OvkWindowReinit(OpalWindow_T* _window);
OpalResult OvkWindowShutdown(OpalWindow_T* _window);

OpalResult OvkRenderpassInit(OpalRenderpass_T* _renderpass);
void OvkRenderpassShutdown(OpalRenderpass_T* _renderpass);

OpalResult OvkFramebufferInit();
OpalResult OvkFramebufferReinit();
void OvkFramebufferShutdown();
OpalResult OvkImageInit(OpalImage_T* _image, OpalImageInitInfo const* _initInfo);
void OvkImageShutdown(OpalImage_T* _image);
OpalResult OvkImageResize(OpalImage_T* _image, OpalExtent _extents);



#endif // !GEM_OPAL_LOCAL_COMMON_H_
