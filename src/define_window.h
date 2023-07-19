
#ifndef GEM_OPAL_WINDOW_H_
#define GEM_OPAL_WINDOW_H_ 1

#include "include/opal_defines.h"
#include <vulkan/vulkan.h>

typedef struct OvkSync_T
{
  VkFence fenceFrameAvailable;
  VkSemaphore semImageAvailable;
  VkSemaphore semRenderComplete;
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

typedef struct OpalWindow_T
{
  LapisWindow* lWindow;

  uint32_t imageCount;
  struct
  {
    uint32_t width;
    uint32_t height;
  } extents;

  OvkWindow_T vk;
} OpalWindow_T;

#endif // !GEM_OPAL_WINDOW_H_
