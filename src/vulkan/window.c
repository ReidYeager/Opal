
#include "src/vulkan/vulkan_common.h"

OpalResult CreateWindowSurface_OpalVk(const OvkState_T _state, const LapisWindow _lapisWindow, OvkWindow_T* _window)
{
  LapisResult result = LapisWindowVulkanCreateSurface(_lapisWindow, _state.instance, &_window->surface);
  if (result != Lapis_Success)
  {
    OVK_LOG_ERROR("Failed to create Lapis surface : result = %d\n", result);
    return Opal_Failure_Vk_Create;
  }

  return Opal_Success;
}

OpalResult GetHardwareInformation_OpalVk(const OvkState_T _state, const LapisWindow _lapisWindow, OvkWindow_T* _window)
{
  VkSurfaceCapabilitiesKHR surfCapabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_state.gpu.device, _window->surface, &surfCapabilities);

  _window->frameCount = surfCapabilities.minImageCount + 1;
  if (surfCapabilities.maxImageCount > 0 && _window->frameCount > surfCapabilities.maxImageCount)
  {
    _window->frameCount = surfCapabilities.maxImageCount;
  }

  if (surfCapabilities.currentExtent.width != -1)
  {
    _window->extents = surfCapabilities.currentExtent;
  }
  else
  {
    _window->extents.width = LapisWindowGetWidth(_lapisWindow);
    _window->extents.height = LapisWindowGetHeight(_lapisWindow);
  }

  uint32_t formatCount = 0;
  VkSurfaceFormatKHR* formats;
  vkGetPhysicalDeviceSurfaceFormatsKHR(_state.gpu.device, _window->surface, &formatCount, NULL);
  formats = LapisMemAllocArray(VkSurfaceFormatKHR, formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(_state.gpu.device, _window->surface, &formatCount, formats);

  _window->surfaceFormat = formats[0];
  for (uint32_t i = 0; i < formatCount; i++)
  {
    if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB
      && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
    {
      _window->surfaceFormat = formats[i];
      break;
    }
  }

  LapisMemFree(formats);

  uint32_t presentModeCount = 0;
  VkPresentModeKHR* presentModes;
  vkGetPhysicalDeviceSurfacePresentModesKHR(_state.gpu.device, _window->surface, &presentModeCount, NULL);
  presentModes = LapisMemAllocArray(VkPresentModeKHR, presentModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(_state.gpu.device, _window->surface, &presentModeCount, presentModes);

  _window->swapchain.presentMode = presentModes[0];
  for (uint32_t i = 0; i < presentModeCount; i++)
  {
    if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
    {
      _window->swapchain.presentMode = presentModes[i];
      break;
    }
  }

  LapisMemFree(presentModes);

  return Opal_Success;
}

OpalResult CreateWindowSwapchain_OpalVk(const OvkState_T _state, OvkWindow_T* _window)
{
  VkSwapchainCreateInfoKHR createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.imageArrayLayers = 1;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  createInfo.clipped = VK_TRUE;
  createInfo.imageColorSpace = _window->surfaceFormat.colorSpace;
  createInfo.imageFormat = _window->surfaceFormat.format;
  createInfo.imageExtent = _window->extents;
  createInfo.presentMode = _window->swapchain.presentMode;
  createInfo.minImageCount = _window->frameCount;
  createInfo.surface = _window->surface;

  uint32_t queueIndies[2] = { _state.gpu.queueIndexGraphics, _state.gpu.queueIndexTransfer };
  if (_state.gpu.queueIndexGraphics == _state.gpu.queueIndexTransfer)
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }
  else
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueIndies;
  }

  OVK_ATTEMPT(vkCreateSwapchainKHR(_state.device, &createInfo, NULL, &_window->swapchain.swapchain),
    return Opal_Failure_Vk_Create);

  return Opal_Success;
}

OpalResult CreateWindowSwapchainImagesAndViews_OpalVk(const OvkState_T _state, OvkWindow_T* _window)
{
  // Images =====
  uint32_t realImageCount;
  vkGetSwapchainImagesKHR(_state.device, _window->swapchain.swapchain, &realImageCount, NULL);
  _window->frameCount = realImageCount;
  _window->swapchain.pImages = LapisMemAllocArray(VkImage, realImageCount);
  _window->swapchain.pViews = LapisMemAllocArray(VkImageView, realImageCount);
  vkGetSwapchainImagesKHR(_state.device, _window->swapchain.swapchain, &realImageCount, _window->swapchain.pImages);

  // Views =====

  VkImageViewCreateInfo viewCreateInfo = { 0 };
  viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewCreateInfo.pNext = NULL;
  viewCreateInfo.flags = 0;
  viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewCreateInfo.format = _window->surfaceFormat.format;
  viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewCreateInfo.subresourceRange.levelCount = 1;
  viewCreateInfo.subresourceRange.baseMipLevel = 0;
  viewCreateInfo.subresourceRange.layerCount = 1;
  viewCreateInfo.subresourceRange.baseArrayLayer = 0;

  for (uint32_t i = 0; i < realImageCount; i++)
  {
    viewCreateInfo.image = _window->swapchain.pImages[i];

    OVK_ATTEMPT(vkCreateImageView(_state.device, &viewCreateInfo, NULL, &_window->swapchain.pViews[i]),
      return Opal_Failure_Vk_Create);
  }

  return Opal_Success;
}

OpalResult CreateWindowFrameInformation_OpalVk(const OvkState_T _state, OvkWindow_T* _window)
{
  _window->currentFrame = 0;
  _window->frameCount = maxFlightSlotCount;
  _window->pFrames = LapisMemAllocArray(OvkFrame_T, _window->frameCount);

  VkSemaphoreCreateInfo semaphoreCreateInfo = { 0 };
  semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceCreateInfo = { 0 };
  fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  VkCommandBufferAllocateInfo cmdBufferAllocInfo = { 0 };
  cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmdBufferAllocInfo.pNext = NULL;
  cmdBufferAllocInfo.commandBufferCount = 1;
  cmdBufferAllocInfo.commandPool = _state.graphicsCommandPool;
  cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

  for (uint32_t i = 0; i < _window->frameCount; i++)
  {
    OvkFrame_T* frame = &_window->pFrames[i];

    frame->swapchainImageIndex = i;

    OVK_ATTEMPT(vkCreateFence(_state.device, &fenceCreateInfo, NULL, &frame->fenceFrameAvailable),
    {
      LapisMemFree(_window->pFrames);
      OVK_LOG_ERROR("Failed to create frame %d's fence\n", i);
      return Opal_Failure_Vk_Create;
    });

    OVK_ATTEMPT(vkCreateSemaphore(_state.device, &semaphoreCreateInfo, NULL, &frame->semRenderComplete),
    {
      LapisMemFree(_window->pFrames);
      OVK_LOG_ERROR("Failed to create frame %d's render complete semaphore\n", i);
      return Opal_Failure_Vk_Create;
    });

    OVK_ATTEMPT(vkCreateSemaphore(_state.device, &semaphoreCreateInfo, NULL, &frame->semImageAvailable),
    {
      LapisMemFree(_window->pFrames);
      OVK_LOG_ERROR("Failed to create frame %d's image available semaphore\n", i);
      return Opal_Failure_Vk_Create;
    });

    OVK_ATTEMPT(vkAllocateCommandBuffers(_state.device, &cmdBufferAllocInfo, &frame->cmd),
    {
      LapisMemFree(_window->pFrames);
      OVK_LOG_ERROR("Failed to create frame %d's command buffer\n", i);
      return Opal_Failure_Vk_Create;
    });
  }

  return Opal_Success;
}

OpalResult OpalVkInitializeWindow(const OpalState _oState, const LapisWindow _lapisWindow, OpalWindow _oWindow)
{
  const OvkState_T state = *(OvkState_T*)_oState->backend.state;
  OvkWindow_T* window = &_oWindow->backend.vulkan;

  if (state.isHeadless)
  {
    return Opal_Failure;
  }

  OPAL_ATTEMPT(CreateWindowSurface_OpalVk(state, _lapisWindow, window),
    return Opal_Failure_Vk_Create);

  OPAL_ATTEMPT(GetHardwareInformation_OpalVk(state, _lapisWindow, window),
    return Opal_Failure_Vk_Create);

  OPAL_ATTEMPT(CreateWindowSwapchain_OpalVk(state, window),
    return Opal_Failure_Vk_Create);
  OPAL_ATTEMPT(CreateWindowSwapchainImagesAndViews_OpalVk(state, window),
    return Opal_Failure_Vk_Create);

  OPAL_ATTEMPT(CreateWindowFrameInformation_OpalVk(state, window),
    return Opal_Failure_Vk_Create);

  return Opal_Success;
}

OpalResult InitWindowPartialA_OpalVk(const OpalState _oState, const LapisWindow _lapisWindow, OvkWindow_T* _window)
{
  const OvkState_T state = *(OvkState_T*)_oState->backend.state;

  if (state.isHeadless)
  {
    return Opal_Failure;
  }

  OPAL_ATTEMPT(CreateWindowSurface_OpalVk(state, _lapisWindow, _window),
    return Opal_Failure_Vk_Create);

  return Opal_Success;
}
OpalResult InitWindowPartialB_OpalVk(const OpalState _oState, const LapisWindow _lapisWindow, OvkWindow_T* _window)
{
  const OvkState_T state = *(OvkState_T*)_oState->backend.state;

  if (state.isHeadless)
  {
    return Opal_Failure;
  }

  OPAL_ATTEMPT(GetHardwareInformation_OpalVk(state, _lapisWindow, _window),
    return Opal_Failure_Vk_Create);

  OPAL_ATTEMPT(CreateWindowSwapchain_OpalVk(state, _window),
    return Opal_Failure_Vk_Create);
  OPAL_ATTEMPT(CreateWindowSwapchainImagesAndViews_OpalVk(state, _window),
    return Opal_Failure_Vk_Create);

  OPAL_ATTEMPT(CreateWindowFrameInformation_OpalVk(state, _window),
    return Opal_Failure_Vk_Create);

  return Opal_Success;
}

void OpalVkShutdownWindow(const OpalState _oState, OpalWindow _oWindow)
{
  OvkState_T state = *(OvkState_T*)_oState->backend.state;
  OvkWindow_T* window = &_oWindow->backend.vulkan;

  vkDeviceWaitIdle(state.device);

  for (uint32_t i = 0; i < window->frameCount; i++)
  {
    // Images destroyed with the swapchain
    //vkDestroyImage(state.device, window->swapchain.pImages[i], NULL);
    vkDestroyImageView(state.device, window->swapchain.pViews[i], NULL);
    vkFreeCommandBuffers(state.device, state.graphicsCommandPool, 1, &window->pFrames[i].cmd);

    vkDestroyFence(state.device, window->pFrames[i].fenceFrameAvailable, NULL);
    vkDestroySemaphore(state.device, window->pFrames[i].semImageAvailable, NULL);
    vkDestroySemaphore(state.device, window->pFrames[i].semRenderComplete, NULL);
  }
  LapisMemFree(window->pFrames);
  LapisMemFree(window->swapchain.pImages);
  LapisMemFree(window->swapchain.pViews);

  vkDestroySwapchainKHR(state.device, window->swapchain.swapchain, NULL);
  vkDestroySurfaceKHR(state.instance, window->surface, NULL);
}

OpalResult OpalVkRecreateWindow(const OpalState _oState, OpalWindow _oWindow)
{
  OpalVkShutdownWindow(_oState, _oWindow);
  OpalVkInitializeWindow(_oState, _oWindow->lapisWindow, _oWindow);

  return Opal_Success;
}
