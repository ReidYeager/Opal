
#include "src/common.h"

OpalResult CreateSurface_Ovk(OpalWindow_T* _window)
{
  if (OpalPlatformCreateSurface(_window->platform, oState.vk.instance, &_window->vk.surface))
  {
    OpalLog("Failed to create Vk surface for Lapis window\n");
    return Opal_Failure;
  }

  return Opal_Success;
}

void ChooseSwapchainImageCount_Ovk(const OpalWindow_T* const _window, uint32_t* _count)
{
  VkSurfaceCapabilitiesKHR surfCapabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(oState.vk.gpu, _window->vk.surface, &surfCapabilities);

  *_count = surfCapabilities.minImageCount + 1;
  if (surfCapabilities.maxImageCount > 0 && *_count > surfCapabilities.maxImageCount)
  {
    *_count = surfCapabilities.maxImageCount;
  }
}

void ChooseSwapchainImageExtents_Ovk(const OpalWindow_T* const _window, VkExtent2D* _extents)
{
  VkSurfaceCapabilitiesKHR surfCapabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(oState.vk.gpu, _window->vk.surface, &surfCapabilities);

  if (surfCapabilities.currentExtent.width != -1)
  {
    *_extents = surfCapabilities.currentExtent;
  }
}

void ChooseSwapchainFormat_Ovk(const OpalWindow_T* const _window, VkSurfaceFormatKHR* _format)
{
  uint32_t formatCount = 0;
  VkSurfaceFormatKHR* formats;
  vkGetPhysicalDeviceSurfaceFormatsKHR(oState.vk.gpu, _window->vk.surface, &formatCount, NULL);
  formats = OpalMemAllocArray(VkSurfaceFormatKHR, formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(oState.vk.gpu, _window->vk.surface, &formatCount, formats);

  *_format = formats[0];
  for (uint32_t i = 0; i < formatCount; i++)
  {
    if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB
      && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
    {
      *_format = formats[i];
      break;
    }
  }

  OpalMemFree(formats);
}

void ChooseSwapchainPresentMode_Ovk(const OpalWindow_T* const _window, VkPresentModeKHR* _mode)
{
  uint32_t presentModeCount = 0;
  VkPresentModeKHR* presentModes;
  vkGetPhysicalDeviceSurfacePresentModesKHR(oState.vk.gpu, _window->vk.surface, &presentModeCount, NULL);
  presentModes = OpalMemAllocArray(VkPresentModeKHR, presentModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(oState.vk.gpu, _window->vk.surface, &presentModeCount, presentModes);

  *_mode = presentModes[0];
  for (uint32_t i = 0; i < presentModeCount; i++)
  {
    if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
    {
      *_mode = presentModes[i];
      break;
    }
  }

  OpalMemFree(presentModes);
}

OpalResult CreateSwapchain_Ovk(OpalWindow_T* _window)
{
  uint32_t imageCount = 0;
  VkExtent2D extents = { 0, 0 };
  VkPresentModeKHR presentMode;
  VkSurfaceFormatKHR format;

  ChooseSwapchainImageCount_Ovk(_window, &imageCount);
  ChooseSwapchainImageExtents_Ovk(_window, &extents);
  ChooseSwapchainPresentMode_Ovk(_window, &presentMode);
  ChooseSwapchainFormat_Ovk(_window, &format);

  VkSwapchainCreateInfoKHR createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.imageArrayLayers = 1;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  createInfo.clipped = VK_TRUE;
  createInfo.imageColorSpace = format.colorSpace;
  createInfo.imageFormat = format.format;
  createInfo.imageExtent = extents;
  createInfo.presentMode = presentMode;
  createInfo.minImageCount = imageCount;
  createInfo.surface = _window->vk.surface;

  uint32_t queueIndies[2] = { oState.vk.gpuInfo.queueIndexGraphics, oState.vk.gpuInfo.queueIndexTransfer };
  if (queueIndies[0] == queueIndies[1])
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }
  else
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueIndies;
  }

  OVK_ATTEMPT(vkCreateSwapchainKHR(oState.vk.device, &createInfo, NULL, &_window->vk.swapchain.swapchain));

  _window->extents.width = extents.width;
  _window->extents.height = extents.height;
  _window->extents.depth = 1;
  _window->vk.format = format.format;
  _window->vk.presentMode = presentMode;

  return Opal_Success;
}

OpalResult CreateSwapchainImages_Ovk(OpalWindow_T* _window)
{
  // Images =====
  uint32_t createdImageCount;
  vkGetSwapchainImagesKHR(oState.vk.device, _window->vk.swapchain.swapchain, &createdImageCount, NULL);
  if (_window->imageCount != createdImageCount)
  {
    if (_window->vk.swapchain.pImages != NULL)
      OpalMemFree(_window->vk.swapchain.pImages);
    if (_window->vk.swapchain.pViews != NULL)
      OpalMemFree(_window->vk.swapchain.pViews);

    _window->imageCount = createdImageCount;
    _window->vk.swapchain.pImages = OpalMemAllocArray(VkImage, createdImageCount);
    _window->vk.swapchain.pViews = OpalMemAllocArray(VkImageView, createdImageCount);
  }
  vkGetSwapchainImagesKHR(oState.vk.device, _window->vk.swapchain.swapchain, &createdImageCount, _window->vk.swapchain.pImages);

  // Views =====

  VkImageViewCreateInfo viewCreateInfo = { 0 };
  viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewCreateInfo.pNext = NULL;
  viewCreateInfo.flags = 0;
  viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewCreateInfo.format = _window->vk.format;
  viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewCreateInfo.subresourceRange.levelCount = 1;
  viewCreateInfo.subresourceRange.baseMipLevel = 0;
  viewCreateInfo.subresourceRange.layerCount = 1;
  viewCreateInfo.subresourceRange.baseArrayLayer = 0;

  for (uint32_t i = 0; i < createdImageCount; i++)
  {
    viewCreateInfo.image = _window->vk.swapchain.pImages[i];

    OVK_ATTEMPT(vkCreateImageView(oState.vk.device, &viewCreateInfo, NULL, &_window->vk.swapchain.pViews[i]));
  }

  return Opal_Success;
}

OpalResult CreateSync_Ovk(OpalWindow_T* _window)
{
  _window->vk.pSync = OpalMemAllocArray(OvkSync_T, _window->imageCount);

  VkSemaphoreCreateInfo semCreateInfo = { 0 };
  semCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceCreateInfo = { 0 };
  fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  VkCommandBufferAllocateInfo cmdInfo = { 0 };
  cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  cmdInfo.commandBufferCount = 1;
  cmdInfo.commandPool = oState.vk.graphicsCommandPool;

  for (uint32_t i = 0; i < _window->imageCount; i++)
  {
    OvkSync_T* sync = &_window->vk.pSync[i];
    OVK_ATTEMPT(vkCreateFence(oState.vk.device, &fenceCreateInfo, oState.vk.allocator, &sync->fenceFrameAvailable));
    OVK_ATTEMPT(vkCreateSemaphore(oState.vk.device, &semCreateInfo, oState.vk.allocator, &sync->semImageAvailable));
    OVK_ATTEMPT(vkCreateSemaphore(oState.vk.device, &semCreateInfo, oState.vk.allocator, &sync->semRenderComplete));
    OVK_ATTEMPT(vkAllocateCommandBuffers(oState.vk.device, &cmdInfo, &sync->cmdBuffer));
  }

  return Opal_Success;
}

OpalResult OvkWindowInit(OpalWindow_T* _window, OpalWindowInitInfo _initInfo)
{
  if (_window->vk.surface != VK_NULL_HANDLE)
  {
    vkDestroySurfaceKHR(oState.vk.instance, _window->vk.surface, oState.vk.allocator);
  }

  uint32_t newWidth = _initInfo.extents.width;
  uint32_t newHeight = _initInfo.extents.height;

  if (!newWidth || !newHeight)
    return Opal_Failure;

  _window->extents.width = newWidth;
  _window->extents.height = newHeight;
  _window->extents.depth = 1;

  OPAL_ATTEMPT(CreateSurface_Ovk(_window));
  OPAL_ATTEMPT(CreateSwapchain_Ovk(_window));
  OPAL_ATTEMPT(CreateSwapchainImages_Ovk(_window));
  OPAL_ATTEMPT(CreateSync_Ovk(_window));

  // TODO : Replace with better framebuffer solution
  OpalImageInitInfo iInitInfo = { 0 };
  iInitInfo.extent = _window->extents;
  iInitInfo.extent.depth = 1;
  iInitInfo.format = Opal_Format_BGRA8;
  iInitInfo.usage = Opal_Image_Usage_Color | Opal_Image_Usage_Copy_Src;
  OPAL_ATTEMPT(OpalImageInit(&_window->renderBufferImage, iInitInfo));

  OpalLog("Vk window init complete\n");

  return Opal_Success;
}

OpalResult OvkWindowReinit(OpalWindow_T* _window)
{
  uint32_t newWidth = _window->extents.width;
  uint32_t newHeight = _window->extents.height;

  if (!newWidth || !newHeight)
    return Opal_Success;

  _window->extents.width = newWidth;
  _window->extents.height = newHeight;
  _window->extents.depth = 1;

  for (uint32_t i = 0; i < _window->imageCount; i++)
  {
    vkDestroyImageView(oState.vk.device, _window->vk.swapchain.pViews[i], oState.vk.allocator);
  }
  //OpalMemFree(_window->vk.swapchain.pViews);
  //OpalMemFree(_window->vk.swapchain.pImages);

  vkDestroySwapchainKHR(oState.vk.device, _window->vk.swapchain.swapchain, oState.vk.allocator);
  vkDestroySurfaceKHR(oState.vk.instance, _window->vk.surface, oState.vk.allocator);

  OPAL_ATTEMPT(CreateSurface_Ovk(_window));
  OPAL_ATTEMPT(CreateSwapchain_Ovk(_window));
  OPAL_ATTEMPT(CreateSwapchainImages_Ovk(_window));

  // TODO : Replace with better framebuffer solution
  OPAL_ATTEMPT(OpalImageResize(_window->renderBufferImage, _window->extents));

  return Opal_Success;
}

OpalResult OvkWindowShutdown(OpalWindow_T* _window)
{
  vkDeviceWaitIdle(oState.vk.device);
  // TODO : Replace with better framebuffer solution
  OpalImageShutdown(&_window->renderBufferImage);

  for (uint32_t i = 0; i < _window->imageCount; i++)
  {
    vkDestroyFence(oState.vk.device, _window->vk.pSync[i].fenceFrameAvailable, oState.vk.allocator);
    vkDestroySemaphore(oState.vk.device, _window->vk.pSync[i].semImageAvailable, oState.vk.allocator);
    vkDestroySemaphore(oState.vk.device, _window->vk.pSync[i].semRenderComplete, oState.vk.allocator);

    vkDestroyImageView(oState.vk.device, _window->vk.swapchain.pViews[i], oState.vk.allocator);
    // Image destruction handled by vkDestroySwapchain
  }
  OpalMemFree(_window->vk.pSync);
  OpalMemFree(_window->vk.swapchain.pViews);
  OpalMemFree(_window->vk.swapchain.pImages);

  vkDestroySwapchainKHR(oState.vk.device, _window->vk.swapchain.swapchain, oState.vk.allocator);

  vkDestroySurfaceKHR(oState.vk.instance, _window->vk.surface, oState.vk.allocator);

  OpalLog("Vk window shutdown complete\n");

  return Opal_Success;
}
