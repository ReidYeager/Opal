
#include "vulkan/vulkan_common.h"

// Declarations
// ============================================================

// Initialization ==========
//OpalResult       OpalVulkanWindowInit       (OpalWindowInitInfo initInfo, OpalWindow* pWindow);
OpalResult         SwapchainInit_Ovk          (OpalWindow* pWindow, OpalWindowInitInfo initInfo);
VkExtent2D         GetSurfaceExtents_Ovk      (OpalVulkanWindow* pWindow);
uint32_t           GetSwapchainImageCount_Ovk (OpalVulkanWindow* pWindow);
VkSurfaceFormatKHR GetSwapchainFormat_Ovk     (OpalVulkanWindow* pWindow, OpalFormat desiredFormat);
VkPresentModeKHR   GetSwapchainPresentMode_Ovk(OpalVulkanWindow* pWindow);
OpalResult         FramesInit_Ovk             (OpalWindow* pWindow, OpalWindowInitInfo initInfo);

// Shutdown ==========
//void             OpalVulkanWindowShutdown   (OpalWindow* pWindow);

// Initialization
// ============================================================

OpalResult OpalVulkanWindowInit(OpalWindow* pWindow, OpalWindowInitInfo initInfo)
{
  OPAL_ATTEMPT(PlatformCreateSurface_Ovk(initInfo.platform, &pWindow->api.vk.surface));
  OPAL_ATTEMPT(SwapchainInit_Ovk(pWindow, initInfo));
  OPAL_ATTEMPT(FramesInit_Ovk(pWindow, initInfo));

  pWindow->imageCount = pWindow->api.vk.imageCount;
  pWindow->api.vk.imageIndex = pWindow->api.vk.imageCount - 1;
  pWindow->imageFormat = VkFormatToOpalFormat_Ovk(pWindow->api.vk.format);
  pWindow->isMinimized = false;

  return Opal_Success;
}

OpalResult SwapchainInit_Ovk(OpalWindow* pWindow, OpalWindowInitInfo initInfo)
{
  VkExtent2D extents = GetSurfaceExtents_Ovk(&pWindow->api.vk);
  uint32_t count = GetSwapchainImageCount_Ovk(&pWindow->api.vk);
  VkPresentModeKHR presentMode = GetSwapchainPresentMode_Ovk(&pWindow->api.vk);
  VkSurfaceFormatKHR format = GetSwapchainFormat_Ovk(&pWindow->api.vk, initInfo.desiredImageFormat);

  OpalFormat formatOpal = VkFormatToOpalFormat_Ovk(format.format);

  VkSwapchainCreateInfoKHR createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.oldSwapchain = VK_NULL_HANDLE;
  createInfo.surface = pWindow->api.vk.surface;
  createInfo.minImageCount = count;
  createInfo.imageExtent = extents;
  createInfo.imageFormat = format.format;
  createInfo.imageColorSpace = format.colorSpace;
  createInfo.presentMode = presentMode;
  createInfo.imageArrayLayers = 1;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  createInfo.clipped = VK_TRUE;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  createInfo.imageUsage |= OpalImageUsageToVkFlags_Ovk(initInfo.imageUse, formatOpal);

  uint32_t queueIndices[2] = { g_ovkState->gpu.queueIndexGraphicsCompute, g_ovkState->gpu.queueIndexTransfer };
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

  OPAL_ATTEMPT_VK(vkCreateSwapchainKHR(g_ovkState->device, &createInfo, NULL, &pWindow->api.vk.swapchain));

  pWindow->width = extents.width;
  pWindow->height = extents.height;
  pWindow->imageFormat = formatOpal;
  pWindow->api.vk.format = format.format;
  pWindow->api.vk.presentMode = presentMode;

  return Opal_Success;
}

VkExtent2D GetSurfaceExtents_Ovk(OpalVulkanWindow* pWindow)
{
  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_ovkState->gpu.device, pWindow->surface, &capabilities);

  if (capabilities.currentExtent.width != -1)
  {
    return capabilities.currentExtent;
  }

  return capabilities.minImageExtent;
}

uint32_t GetSwapchainImageCount_Ovk(OpalVulkanWindow* pWindow)
{
  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_ovkState->gpu.device, pWindow->surface, &capabilities);

  uint32_t count = capabilities.minImageCount + 1;

  if (capabilities.maxImageCount > 0 && count > capabilities.maxImageCount)
  {
    count = capabilities.maxImageCount;
  }

  return count;
}

VkSurfaceFormatKHR GetSwapchainFormat_Ovk(OpalVulkanWindow* pWindow, OpalFormat desiredFormat)
{
  uint32_t count;
  VkSurfaceFormatKHR* formats;

  vkGetPhysicalDeviceSurfaceFormatsKHR(g_ovkState->gpu.device, pWindow->surface, &count, NULL);
  formats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(g_ovkState->gpu.device, pWindow->surface, &count, formats);

  VkFormat desiredFormatVk = OpalFormatToVkFormat_Ovk(desiredFormat);
  bool foundDesired = false;

  VkSurfaceFormatKHR finalFormat = formats[0];
  bool matchFormat = false, matchSpace = false;
  for (int i = 0; i < count; i++)
  {
    matchFormat = formats[i].format == desiredFormatVk;
    matchSpace = formats[i].colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR;

    if (matchFormat && matchSpace)
    {
      foundDesired = true;
      finalFormat = formats[i];
      break;
    }
    else if (matchFormat)
    {
      finalFormat = formats[i];
    }
    else if (matchSpace)
    {
      finalFormat = formats[i];
    }
  }

  if (!foundDesired)
  {
    OpalLogError("Surface does not support desired format (%d) using %d",
      desiredFormat, VkFormatToOpalFormat_Ovk(finalFormat.format));
  }

  free(formats);
  return finalFormat;
}

VkPresentModeKHR GetSwapchainPresentMode_Ovk(OpalVulkanWindow* pWindow)
{
  uint32_t count;
  VkPresentModeKHR* modes;

  vkGetPhysicalDeviceSurfacePresentModesKHR(g_ovkState->gpu.device, pWindow->surface, &count, NULL);
  modes = (VkPresentModeKHR*)malloc(sizeof(VkPresentModeKHR) * count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(g_ovkState->gpu.device, pWindow->surface, &count, modes);

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

OpalResult FramesInit_Ovk(OpalWindow* pWindow, OpalWindowInitInfo initInfo)
{
  uint32_t createdCount = 0;
  OpalVulkanWindow* window = &pWindow->api.vk;

  // Swapchain images ==========

  OPAL_ATTEMPT_VK(vkGetSwapchainImagesKHR(g_ovkState->device, window->swapchain, &createdCount, NULL));

  window->imageCount = createdCount;
  window->pImages             = OpalMemAllocArray(VkImage,         createdCount);
  window->pImageViews         = OpalMemAllocArray(VkImageView,     createdCount);
  window->pSamplers           = OpalMemAllocArray(VkSampler,       createdCount);
  window->pImageAvailableSems = OpalMemAllocArray(VkSemaphore,     createdCount);
  window->pRenderCompleteSems = OpalMemAllocArray(VkSemaphore,     createdCount);

  OPAL_ATTEMPT_VK(vkGetSwapchainImagesKHR(g_ovkState->device, window->swapchain, &createdCount, window->pImages));

  // Swapchain image views ==========

  VkImageViewCreateInfo viewInfo = { 0 };
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
    viewInfo.image = window->pImages[i];
    OPAL_ATTEMPT_VK(vkCreateImageView(g_ovkState->device, &viewInfo, NULL, &window->pImageViews[i]));
  }

  // Image samplers ==========

  VkSamplerCreateInfo samplerInfo = { 0 };
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.pNext = NULL;
  samplerInfo.flags = 0;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.anisotropyEnable = VK_FALSE;
  samplerInfo.maxAnisotropy = 1.0f;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = 0.0f;

  for (int i = 0; i < createdCount; i++)
  {
    OPAL_ATTEMPT_VK(vkCreateSampler(g_ovkState->device, &samplerInfo, NULL, &window->pSamplers[i]));
  }

  // Synchronization ==========

  VkSemaphoreCreateInfo semaphoreInfo = { 0 };
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  semaphoreInfo.pNext = NULL;
  semaphoreInfo.flags = 0;

  for (int i = 0; i < createdCount; i++)
  {
    OPAL_ATTEMPT_VK(
      vkCreateSemaphore(g_ovkState->device, &semaphoreInfo, NULL, &window->pImageAvailableSems[i]));
    OPAL_ATTEMPT_VK(
      vkCreateSemaphore(g_ovkState->device, &semaphoreInfo, NULL, &window->pRenderCompleteSems[i]));
  }
  window->syncIndex = 0;

  // Images ==========

  pWindow->pImages = OpalMemAllocArrayZeroed(OpalImage, createdCount);
  for (int i = 0; i < createdCount; i++)
  {
    pWindow->pImages[i].width = pWindow->width;
    pWindow->pImages[i].height = pWindow->height;
    pWindow->pImages[i].mipCount = 1;
    pWindow->pImages[i].format = VkFormatToOpalFormat_Ovk(pWindow->api.vk.format);
    pWindow->pImages[i].usage = Opal_Image_Usage_Output | initInfo.imageUse;
    pWindow->pImages[i].api.vk.format = pWindow->api.vk.format;
    pWindow->pImages[i].api.vk.image = pWindow->api.vk.pImages[i];
    pWindow->pImages[i].api.vk.view = pWindow->api.vk.pImageViews[i];
    pWindow->pImages[i].api.vk.sampler = pWindow->api.vk.pSamplers[i];
    pWindow->pImages[i].api.vk.memory = VK_NULL_HANDLE;
    pWindow->pImages[i].api.vk.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    pWindow->pImages[i].api.vk.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

    OPAL_ATTEMPT(ImageTransitionLayout_Ovk(&pWindow->pImages[i], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR));
  }

  return Opal_Success;
}

// Shutdown
// ============================================================

void OpalVulkanWindowShutdown(OpalWindow* pWindow)
{
  for (int i = 0; i < pWindow->api.vk.imageCount; i++)
  {
    vkDestroySemaphore(g_ovkState->device, pWindow->api.vk.pImageAvailableSems[i], NULL);
    vkDestroySemaphore(g_ovkState->device, pWindow->api.vk.pRenderCompleteSems[i], NULL);

    vkDestroyImageView(g_ovkState->device, pWindow->api.vk.pImageViews[i], NULL);
    vkDestroySampler(g_ovkState->device, pWindow->api.vk.pSamplers[i], NULL);
  }

  OpalMemFree(pWindow->api.vk.pImageAvailableSems);
  OpalMemFree(pWindow->api.vk.pRenderCompleteSems);
  OpalMemFree(pWindow->api.vk.pImages);
  OpalMemFree(pWindow->api.vk.pImageViews);
  OpalMemFree(pWindow->api.vk.pSamplers);

  vkDestroySwapchainKHR(g_ovkState->device, pWindow->api.vk.swapchain, NULL);
  vkDestroySurfaceKHR(g_ovkState->instance, pWindow->api.vk.surface, NULL);
}
