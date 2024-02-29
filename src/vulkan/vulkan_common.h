#ifndef OPAL_VULKAN_COMMON_H
#define OPAL_VULKAN_COMMON_H 1

#include "src/common.h"
#include "include/vulkan_defines.h"

#include <vulkan/vulkan.h>

// ============================================================
// ============================================================
//
// Preprocessor ==========
// ATTEMPT_VK
// ATTEMPT_VK_FAIL_LOG
//
// Wrappers ==========
// OpalVulkanInit()
//
// Core ==========
// PlatformGetExtensions_Ovk()
// PlatformCreateSurface_Ovk()
//
// ============================================================
// ============================================================


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
  else {}                                                                   \
}

#define OPAL_ATTEMPT_VK_FAIL_LOG(message, ...)     \
{                                                  \
  OpalLogError("Vulkan :: " message, __VA_ARGS__); \
  OpalLogError("    %s:%d\n", __FILE__, __LINE__); \
}

// Wrappers
// ============================================================

OpalResult OpalVulkanInit(OpalInitInfo initInfo);
// Window ==========
OpalResult OpalVulkanWindowInit       (OpalWindowInitInfo initInfo, OpalWindow* pWindow);
void       OpalVulkanWindowShutdown   (OpalWindow* pWindow);
OpalResult OpalVulkanWindowSwapBuffers(const OpalWindow* pWindow);
//OpalResult         OpalVulkanWindowGetImage    (const OpalWindow* pWindow, OpalImage* pImage);

// Core
// ============================================================

uint32_t PlatformGetExtensions_Ovk(const char*** pOutExtensions);
OpalResult PlatformCreateSurface_Ovk(OpalPlatformWindowInfo windowInfo, VkSurfaceKHR* outSurface);


#endif // !OPAL_VULKAN_COMMON_H
