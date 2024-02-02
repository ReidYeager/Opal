
#include "src/common.h"

#ifdef OPAL_PLATFORM_WIN32
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

void OpalPlatformGetRequiredExtensions(uint32_t* _extensionCount, const char** _extensionNames)
{
  if (_extensionCount != NULL)
  {
    *_extensionCount = 2;
  }

  if (_extensionNames != NULL)
  {
    _extensionNames[0] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
    _extensionNames[1] = VK_KHR_SURFACE_EXTENSION_NAME;
  }
}

OpalResult OpalPlatformCreateSurface(OpalWindowPlatformInfo_T _window, VkInstance _instance, VkSurfaceKHR* _surface)
{
  VkWin32SurfaceCreateInfoKHR createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.hinstance = _window.hinstance;
  createInfo.hwnd = _window.hwnd;

  VkResult result = vkCreateWin32SurfaceKHR(
    _instance,
    &createInfo,
    NULL,
    _surface);

  if (result != VK_SUCCESS)
  {
    OpalLogError("Failed to create vulkan window surface");
    return Opal_Failure;
  }

  return Opal_Success;
}
#else
void OpalPlatformGetRequiredExtensions(uint32_t* _extensionCount, const char** _extensionNames) {}
OpalResult OpalPlatformCreateSurface(OpalWindowPlatformInfo_T _window, VkInstance _instance, VkSurfaceKHR* _surface)
{
  return Opal_Success;
}
#endif // MSB_PLATFORM_*
