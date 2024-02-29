
#include "src/vulkan/vulkan_common.h"

// Declarations
// ============================================================s

// Initialization ==========
//OpalResult       OpalVulkanWindowInit       (OpalWindowInitInfo initInfo, OpalWindow* pWindow);
OpalResult         SwapchainInit_Ovk          (OpalWindow* pWindow);
VkExtent2D         GetSurfaceExtents_Ovk      (OpalVulkanWindow* pWindow);
uint32_t           GetSwapchainImageCount_Ovk (OpalVulkanWindow* pWindow);
VkSurfaceFormatKHR GetSwapchainFormat_Ovk     (OpalVulkanWindow* pWindow);
VkPresentModeKHR   GetSwapchainPresentMode_Ovk(OpalVulkanWindow* pWindow);
OpalResult         FramesInit_Ovk             (OpalWindow* pWindow);

// Shutdown ==========
//void OpalVulkanWindowShutdown(OpalWindow* pWindow);

// Use ==========
//OpalResult OpalVulkanWindowSwapBuffers(const OpalWindow* pWindow);
//OpalResult OpalVulkanWindowGetImage   (const OpalWindow* pWindow, OpalImage* pImage);

// Initialization
// ============================================================

OpalResult OpalVulkanWindowInit(OpalWindowInitInfo initInfo, OpalWindow* pWindow)
{
  OPAL_ATTEMPT(PlatformCreateSurface_Ovk(initInfo.platform, &pWindow->api.vk.surface));
  OPAL_ATTEMPT(SwapchainInit_Ovk(pWindow));
  OPAL_ATTEMPT(FramesInit_Ovk(pWindow));

  return Opal_Success;
}

OpalResult SwapchainInit_Ovk(OpalWindow* pWindow)
{
  VkExtent2D extents = GetSurfaceExtents_Ovk(&pWindow->api.vk);
  uint32_t count = GetSwapchainImageCount_Ovk(&pWindow->api.vk);
  VkPresentModeKHR presentMode = GetSwapchainPresentMode_Ovk(&pWindow->api.vk);
  VkSurfaceFormatKHR format = GetSwapchainFormat_Ovk(&pWindow->api.vk);

  VkSwapchainCreateInfoKHR createInfo;
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.surface = pWindow->api.vk.surface;
  createInfo.minImageCount = count;
  createInfo.imageExtent = extents;
  createInfo.imageFormat = format.format;
  createInfo.imageColorSpace = format.colorSpace;
  createInfo.presentMode = presentMode;
  createInfo.imageArrayLayers = 1;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  createInfo.clipped = VK_TRUE;

  uint32_t queueIndices[2] = { g_OpalState.api.vk.gpu.queueIndexGraphics, g_OpalState.api.vk.gpu.queueIndexTransfer };
  if (queueIndices[0] == queueIndices[1])
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }
  else
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueIndices;
  }

  OPAL_ATTEMPT_VK(vkCreateSwapchainKHR(g_OpalState.api.vk.device, &createInfo, NULL, &pWindow->api.vk.swapchain));

  pWindow->width = extents.width;
  pWindow->height = extents.height;
  pWindow->api.vk.format = format.format;
  pWindow->api.vk.presentMode = presentMode;

  return Opal_Success;
}

VkExtent2D GetSurfaceExtents_Ovk(OpalVulkanWindow* pWindow)
{
  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_OpalState.api.vk.gpu.device, pWindow->surface, &capabilities);

  if (capabilities.currentExtent.width != -1)
  {
    return capabilities.currentExtent;
  }

  return capabilities.minImageExtent;
}

uint32_t GetSwapchainImageCount_Ovk(OpalVulkanWindow* pWindow)
{
  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_OpalState.api.vk.gpu.device, pWindow->surface, &capabilities);

  uint32_t count = capabilities.minImageCount + 1;

  if (capabilities.maxImageCount > 0 && count > capabilities.maxImageCount)
  {
    count = capabilities.maxImageCount;
  }

  return count;
}

VkSurfaceFormatKHR GetSwapchainFormat_Ovk(OpalVulkanWindow* pWindow)
{
  uint32_t count;
  VkSurfaceFormatKHR* formats;

  vkGetPhysicalDeviceSurfaceFormatsKHR(g_OpalState.api.vk.gpu.device, pWindow->surface, &count, NULL);
  formats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(g_OpalState.api.vk.gpu.device, pWindow->surface, &count, formats);

  VkSurfaceFormatKHR finalFormat = formats[0];
  for (int i = 0; i < count; i++)
  {
    if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB
      && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
    {
      finalFormat = formats[i];
      break;
    }
  }

  free(formats);
  return finalFormat;
}

VkPresentModeKHR GetSwapchainPresentMode_Ovk(OpalVulkanWindow* pWindow)
{
  uint32_t count;
  VkPresentModeKHR* modes;

  vkGetPhysicalDeviceSurfacePresentModesKHR(g_OpalState.api.vk.gpu.device, pWindow->surface, &count, NULL);
  modes = (VkPresentModeKHR*)malloc(sizeof(VkPresentModeKHR) * count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(g_OpalState.api.vk.gpu.device, pWindow->surface, &count, modes);

  VkPresentModeKHR finalMode = modes[0];
  for (int i = 0; i < count; i++)
  {
    if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
    {
      finalMode = modes[i];
      break;
    }
  }

  free(modes);
  return finalMode;
}

OpalResult FramesInit_Ovk(OpalWindow* pWindow)
{
  uint32_t createdCount;
  OpalVulkanWindow* window = &pWindow->api.vk;

  // Swapchain images ==========

  OPAL_ATTEMPT_VK(vkGetSwapchainImagesKHR(g_OpalState.api.vk.device, window->swapchain, &createdCount, NULL));

  window->imageCount = createdCount;
  window->images                   = (VkImage*)    malloc(sizeof(VkImage)     * createdCount);
  window->imageViews               = (VkImageView*)malloc(sizeof(VkImageView) * createdCount);
  window->fencesImageAvailable     = (VkFence*)    malloc(sizeof(VkFence)     * createdCount);
  window->semaphoresImageAvailable = (VkSemaphore*)malloc(sizeof(VkSemaphore) * createdCount);
  window->semaphoresRenderComplete = (VkSemaphore*)malloc(sizeof(VkSemaphore) * createdCount);

  OPAL_ATTEMPT_VK(vkGetSwapchainImagesKHR(g_OpalState.api.vk.device, window->swapchain, &createdCount, window->images));

  // Swapchain image views ==========

  VkImageViewCreateInfo viewInfo;
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.pNext = NULL;
  viewInfo.flags = 0;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = window->format;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.layerCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  for (int i = 0; i < createdCount; i++)
  {
    viewInfo.image = window->images[i];
    OPAL_ATTEMPT_VK(vkCreateImageView(g_OpalState.api.vk.device, &viewInfo, NULL, &window->imageViews[i]));
  }

  // Synchronization ==========

  VkSemaphoreCreateInfo semaphoreInfo;
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  semaphoreInfo.flags = 0;
  semaphoreInfo.pNext = NULL;

  VkFenceCreateInfo fenceInfo;
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  fenceInfo.pNext = NULL;

  for (int i = 0; i < createdCount; i++)
  {
    OPAL_ATTEMPT_VK(
      vkCreateFence(g_OpalState.api.vk.device, &fenceInfo, NULL, &window->fencesImageAvailable[i]));
    OPAL_ATTEMPT_VK(
      vkCreateSemaphore(g_OpalState.api.vk.device, &semaphoreInfo, NULL, &window->semaphoresImageAvailable[i]));
    OPAL_ATTEMPT_VK(
      vkCreateSemaphore(g_OpalState.api.vk.device, &semaphoreInfo, NULL, &window->semaphoresRenderComplete[i]));
  }

  return Opal_Success;
}

// Shutdown
// ============================================================

void OpalVulkanWindowShutdown(OpalWindow* pWindow)
{
  for (int i = 0; i < pWindow->api.vk.imageCount; i++)
  {
    vkDestroyFence(g_OpalState.api.vk.device, pWindow->api.vk.fencesImageAvailable[i], NULL);
    vkDestroySemaphore(g_OpalState.api.vk.device, pWindow->api.vk.semaphoresImageAvailable[i], NULL);
    vkDestroySemaphore(g_OpalState.api.vk.device, pWindow->api.vk.semaphoresRenderComplete[i], NULL);

    vkDestroyImage(g_OpalState.api.vk.device, pWindow->api.vk.images[i], NULL);
    vkDestroyImageView(g_OpalState.api.vk.device, pWindow->api.vk.imageViews[i], NULL);
  }
  free(pWindow->api.vk.fencesImageAvailable);
  free(pWindow->api.vk.semaphoresImageAvailable);
  free(pWindow->api.vk.semaphoresRenderComplete);
  free(pWindow->api.vk.images);
  free(pWindow->api.vk.imageViews);

  vkDestroySwapchainKHR(g_OpalState.api.vk.device, pWindow->api.vk.swapchain, NULL);
  vkDestroySurfaceKHR(g_OpalState.api.vk.instance, pWindow->api.vk.surface, NULL);
}

// Use
// ============================================================

OpalResult OpalVulkanWindowSwapBuffers(const OpalWindow* pWindow)
{
  
  return Opal_Success;
}

//OpalResult OpalVulkanWindowGetImage(const OpalWindow* pWindow, OpalImage* pImage)
//{
//  
//  return Opal_Success;
//}
