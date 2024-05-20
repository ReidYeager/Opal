
#include "common.h"
#include "vulkan/vulkan_common.h"

// Declarations
// ============================================================

// Win32 ==========
// uint32_t   PlatformGetExtensions_Ovk(const char*** pOutExtensions)
// OpalResult PlatformCreateSurface_Ovk(OpalPlatformWindowInfo windowInfo, VkSurfaceKHR* outSurface)

// Default ==========
// uint32_t   PlatformGetExtensions_Ovk(const char*** pOutExtensions)
// OpalResult PlatformCreateSurface_Ovk(OpalPlatformWindowInfo windowInfo, VkSurfaceKHR* outSurface)

#ifdef OPAL_PLATFORM_WIN32

// Win32
// ============================================================

#include <Windows.h>
#include <vulkan/vulkan_win32.h>

uint32_t PlatformGetExtensions_Ovk(const char*** pOutExtensions)
{
  *pOutExtensions = OpalMemAllocArray(const char*, 2);
  (*pOutExtensions)[0] = VK_KHR_SURFACE_EXTENSION_NAME;
  (*pOutExtensions)[1] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;

  return 2;
}

OpalResult PlatformCreateSurface_Ovk(OpalPlatformWindowInfo windowInfo, VkSurfaceKHR* outSurface)
{
  VkWin32SurfaceCreateInfoKHR createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.hinstance = (HINSTANCE)windowInfo.hinstance;
  createInfo.hwnd = (HWND)windowInfo.hwnd;

  OPAL_ATTEMPT_VK(vkCreateWin32SurfaceKHR(g_ovkState->instance, &createInfo, NULL, outSurface));

  return Opal_Success;
}

#else

// Default
// ============================================================

uint32_t PlatformGetExtensions_Ovk(const char*** pOutExtensions)
{
  *pOutExtensions = NULL;
  return 0;
}

OpalResult PlatformCreateSurface_Ovk(OpalPlatformWindowInfo windowInfo, VkSurfaceKHR* outSurface)
{
  return Opal_Failure_Unknown;
}

#endif // OPAL_PLATFORM_*
