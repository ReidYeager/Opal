
#include "src/common.h"

OpalResult CreateSurface_Ovk(OpalWindow_T* _window);
OpalResult CreateSwapchain_Ovk(OpalWindow_T* _window);
OpalResult CreateSwapchainImages_Ovk(OpalWindow_T* _window);
OpalResult CreateSync_Ovk(OpalWindow_T* _window);

uint32_t GetMemoryTypeIndex(uint32_t _mask, VkMemoryPropertyFlags _flags)
{
  const VkPhysicalDeviceMemoryProperties* memProps = &oState.vk.gpuInfo.memoryProperties;
  for (uint32_t i = 0; i < memProps->memoryTypeCount; i++)
  {
    if (_mask & (1 << i) && (memProps->memoryTypes[i].propertyFlags & _flags) == _flags)
    {
      return i;
    }
  }

  OpalLog("Vulkan image failed to find a suitable memory type index\n");
  return ~0u;
}

OpalResult SuperTmpCreateBufferImage(OpalWindow_T* _window)
{
  OpalImage_T* image = &_window->renderBufferImage;
  image->format = Opal_Image_Format_R8G8B8A8;

  VkImageCreateInfo iCreateInfo = { 0 };
  iCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  iCreateInfo.extent.width = _window->extents.width;
  iCreateInfo.extent.height = _window->extents.height;
  iCreateInfo.extent.depth = 1;
  iCreateInfo.format = _window->vk.format;
  iCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  iCreateInfo.mipLevels = 1;
  iCreateInfo.arrayLayers = 1;
  iCreateInfo.imageType = VK_IMAGE_TYPE_2D;
  iCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  iCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  iCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;

  image->vk.format = _window->vk.format;
  image->width = _window->extents.width;
  image->height = _window->extents.height;

  OVK_ATTEMPT(vkCreateImage(oState.vk.device, &iCreateInfo, oState.vk.allocator, &image->vk.image));

  VkMemoryRequirements memReq = { 0 };
  vkGetImageMemoryRequirements(oState.vk.device, image->vk.image, &memReq);

  VkMemoryAllocateInfo memAllocInfo = { 0 };
  memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memAllocInfo.allocationSize = memReq.size;
  memAllocInfo.memoryTypeIndex = GetMemoryTypeIndex(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  OVK_ATTEMPT(vkAllocateMemory(oState.vk.device, &memAllocInfo, oState.vk.allocator, &image->vk.memory));
  OVK_ATTEMPT(vkBindImageMemory(oState.vk.device, image->vk.image, image->vk.memory, 0)); 

  VkImageViewCreateInfo vCreateInfo = { 0 };
  vCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  vCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  vCreateInfo.image = image->vk.image;
  vCreateInfo.format = image->vk.format;
  vCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  vCreateInfo.subresourceRange.levelCount = 1;
  vCreateInfo.subresourceRange.baseMipLevel = 0;
  vCreateInfo.subresourceRange.layerCount = 1;
  vCreateInfo.subresourceRange.baseArrayLayer = 0;

  OVK_ATTEMPT(vkCreateImageView(oState.vk.device, &vCreateInfo, oState.vk.allocator, &image->vk.view));

  return Opal_Success;
}

OpalResult OvkWindowInit(OpalWindow_T* _window)
{
  if (_window->vk.surface != VK_NULL_HANDLE)
  {
    vkDestroySurfaceKHR(oState.vk.instance, _window->vk.surface, oState.vk.allocator);
  }

  OPAL_ATTEMPT(CreateSurface_Ovk(_window));
  OPAL_ATTEMPT(CreateSwapchain_Ovk(_window));
  OPAL_ATTEMPT(CreateSwapchainImages_Ovk(_window));
  OPAL_ATTEMPT(CreateSync_Ovk(_window));

  OpalLog("Vk window init complete\n");

  return Opal_Success;
}

OpalResult OvkWindowReinit(OpalWindow_T* _window)
{
  vkDestroyImageView(oState.vk.device, _window->renderBufferImage.vk.view, oState.vk.allocator);
  vkDestroyImage(oState.vk.device, _window->renderBufferImage.vk.image, oState.vk.allocator);
  vkFreeMemory(oState.vk.device, _window->renderBufferImage.vk.memory, oState.vk.allocator);

  for (uint32_t i = 0; i < _window->imageCount; i++)
  {
    vkDestroyImageView(oState.vk.device, _window->vk.swapchain.pViews[i], oState.vk.allocator);
  }
  //LapisMemFree(_window->vk.swapchain.pViews);
  //LapisMemFree(_window->vk.swapchain.pImages);

  vkDestroySwapchainKHR(oState.vk.device, _window->vk.swapchain.swapchain, oState.vk.allocator);
  vkDestroySurfaceKHR(oState.vk.instance, _window->vk.surface, oState.vk.allocator);

  OPAL_ATTEMPT(CreateSurface_Ovk(_window));
  OPAL_ATTEMPT(CreateSwapchain_Ovk(_window));
  OPAL_ATTEMPT(CreateSwapchainImages_Ovk(_window));

  OpalLog("Vk window re-init complete\n");

  return Opal_Success;
}

OpalResult OvkWindowShutdown(OpalWindow_T* _window)
{
  vkDestroyImageView(oState.vk.device, _window->renderBufferImage.vk.view, oState.vk.allocator);
  vkDestroyImage(oState.vk.device, _window->renderBufferImage.vk.image, oState.vk.allocator);
  vkFreeMemory(oState.vk.device, _window->renderBufferImage.vk.memory, oState.vk.allocator);

  for (uint32_t i = 0; i < _window->imageCount; i++)
  {
    vkDestroyFence(oState.vk.device, _window->vk.pSync[i].fenceFrameAvailable, oState.vk.allocator);
    vkDestroySemaphore(oState.vk.device, _window->vk.pSync[i].semImageAvailable, oState.vk.allocator);
    vkDestroySemaphore(oState.vk.device, _window->vk.pSync[i].semRenderComplete, oState.vk.allocator);

    vkDestroyImageView(oState.vk.device, _window->vk.swapchain.pViews[i], oState.vk.allocator);
    // Image destruction handled by vkDestroySwapchain
  }
  LapisMemFree(_window->vk.pSync);
  LapisMemFree(_window->vk.swapchain.pViews);
  LapisMemFree(_window->vk.swapchain.pImages);

  vkDestroySwapchainKHR(oState.vk.device, _window->vk.swapchain.swapchain, oState.vk.allocator);

  vkDestroySurfaceKHR(oState.vk.instance, _window->vk.surface, oState.vk.allocator);

  OpalLog("Vk window shutdown complete\n");

  return Opal_Success;
}

OpalResult CreateSurface_Ovk(OpalWindow_T* _window)
{
  if (LapisWindowVulkanCreateSurface(*oState.window.lWindow, oState.vk.instance, &oState.window.vk.surface))
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
  else
  {
    _extents->width = LapisWindowGetWidth(*_window->lWindow);
    _extents->height = LapisWindowGetHeight(*_window->lWindow);
  }
}

void ChooseSwapchainFormat_Ovk(const OpalWindow_T* const _window, VkSurfaceFormatKHR* _format)
{
  uint32_t formatCount = 0;
  VkSurfaceFormatKHR* formats;
  vkGetPhysicalDeviceSurfaceFormatsKHR(oState.vk.gpu, _window->vk.surface, &formatCount, NULL);
  formats = LapisMemAllocArray(VkSurfaceFormatKHR, formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(oState.vk.gpu, _window->vk.surface, &formatCount, formats);

  *_format = formats[0];
  for (uint32_t i = 0; i < formatCount; i++)
  {
    if (formats[i].format == VK_FORMAT_R8G8B8A8_SRGB
      && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
    {
      *_format = formats[i];
      break;
    }
  }

  LapisMemFree(formats);
}

void ChooseSwapchainPresentMode_Ovk(const OpalWindow_T* const _window, VkPresentModeKHR* _mode)
{
  uint32_t presentModeCount = 0;
  VkPresentModeKHR* presentModes;
  vkGetPhysicalDeviceSurfacePresentModesKHR(oState.vk.gpu, _window->vk.surface, &presentModeCount, NULL);
  presentModes = LapisMemAllocArray(VkPresentModeKHR, presentModeCount);
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

  LapisMemFree(presentModes);
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
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
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
      LapisMemFree(_window->vk.swapchain.pImages);
    if (_window->vk.swapchain.pViews != NULL)
      LapisMemFree(_window->vk.swapchain.pViews);

    _window->imageCount = createdImageCount;
    _window->vk.swapchain.pImages = LapisMemAllocArray(VkImage, createdImageCount);
    _window->vk.swapchain.pViews = LapisMemAllocArray(VkImageView, createdImageCount);
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

  OPAL_ATTEMPT(SuperTmpCreateBufferImage(_window));

  return Opal_Success;
}

OpalResult CreateSync_Ovk(OpalWindow_T* _window)
{
  _window->vk.pSync = LapisMemAllocArray(OvkSync_T, _window->imageCount);

  VkSemaphoreCreateInfo semCreateInfo = { 0 };
  semCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceCreateInfo = { 0 };
  fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (uint32_t i = 0; i < _window->imageCount; i++)
  {
    OvkSync_T* sync = &_window->vk.pSync[i];
    OVK_ATTEMPT(vkCreateFence(oState.vk.device, &fenceCreateInfo, oState.vk.allocator, &sync->fenceFrameAvailable));
    OVK_ATTEMPT(vkCreateSemaphore(oState.vk.device, &semCreateInfo, oState.vk.allocator, &sync->semImageAvailable));
    OVK_ATTEMPT(vkCreateSemaphore(oState.vk.device, &semCreateInfo, oState.vk.allocator, &sync->semRenderComplete));
  }

  return Opal_Success;
}
