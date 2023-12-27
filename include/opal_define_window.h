
#ifndef GEM_OPAL_WINDOW_H_
#define GEM_OPAL_WINDOW_H_ 1

#include "include/opal_define_image.h"

#include <vulkan/vulkan.h>

typedef struct OpalWindow_T* OpalWindow;

typedef struct OvkSync_T
{
  VkFence fenceFrameAvailable;
  VkSemaphore semImageAvailable;
  VkSemaphore semRenderComplete;
  VkCommandBuffer cmdBuffer;
} OvkSync_T;

typedef struct OvkWindow_T
{
  VkSurfaceKHR surface;
  VkPresentModeKHR presentMode;
  VkFormat format;
  struct
  {
    VkSwapchainKHR swapchain;
    VkImage* pImages;
    VkImageView* pViews;
  }swapchain;

  OvkSync_T* pSync;
} OvkWindow_T;

#ifdef OPAL_PLATFORM_WIN32
#include <Windows.h>
#include <Windowsx.h>
typedef struct OpalWindowPlatformInfo_T
{
  HWND hwnd;
  HINSTANCE hinstance;
} OpalWindowPlatformInfo_T;
#else
typedef struct OpalWindowPlatformInfo_T
{
  int none;
} OpalWindowPlatformInfo_T;
#endif // OPAL_PLATFORM_

typedef struct OpalWindow_T
{
  OpalWindowPlatformInfo_T platform;

  uint32_t imageCount;
  OpalExtent extents;

  // Used for framebuffers, contents copied to appropriate swapchain image during SwapBuffers
  OpalImage_T* renderBufferImage;

  OvkWindow_T vk;
} OpalWindow_T;

typedef struct OpalWindowInitInfo
{
  OpalWindowPlatformInfo_T platformInfo;

  OpalVec2U extents;
  //OpalVec2I position;
} OpalWindowInitInfo;

#endif // !GEM_OPAL_WINDOW_H_
