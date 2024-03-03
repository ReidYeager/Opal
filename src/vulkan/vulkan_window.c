
#include "src/vulkan/vulkan_common.h"

// Declarations
// ============================================================

// Initialization ==========
//OpalResult       OpalVulkanWindowInit       (OpalWindowInitInfo initInfo, OpalWindow* pWindow);
OpalResult         SwapchainInit_Ovk          (OpalWindow* pWindow);
VkExtent2D         GetSurfaceExtents_Ovk      (OpalVulkanWindow* pWindow);
uint32_t           GetSwapchainImageCount_Ovk (OpalVulkanWindow* pWindow);
VkSurfaceFormatKHR GetSwapchainFormat_Ovk     (OpalVulkanWindow* pWindow);
VkPresentModeKHR   GetSwapchainPresentMode_Ovk(OpalVulkanWindow* pWindow);
OpalResult         FramesInit_Ovk             (OpalWindow* pWindow);

// Shutdown ==========
//void             OpalVulkanWindowShutdown   (OpalWindow* pWindow);

// Initialization
// ============================================================

OpalResult OpalVulkanWindowInit(OpalWindow* pWindow, OpalWindowInitInfo initInfo)
{
  OPAL_ATTEMPT(PlatformCreateSurface_Ovk(initInfo.platform, &pWindow->api.vk.surface));
  OPAL_ATTEMPT(SwapchainInit_Ovk(pWindow));
  OPAL_ATTEMPT(FramesInit_Ovk(pWindow));

  pWindow->imageCount = pWindow->api.vk.imageCount;
  pWindow->api.vk.imageIndex = pWindow->api.vk.imageCount - 1;

  return Opal_Success;
}

OpalResult SwapchainInit_Ovk(OpalWindow* pWindow)
{
  VkExtent2D extents = GetSurfaceExtents_Ovk(&pWindow->api.vk);
  uint32_t count = GetSwapchainImageCount_Ovk(&pWindow->api.vk);
  VkPresentModeKHR presentMode = GetSwapchainPresentMode_Ovk(&pWindow->api.vk);
  VkSurfaceFormatKHR format = GetSwapchainFormat_Ovk(&pWindow->api.vk);

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
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  createInfo.clipped = VK_TRUE;

  uint32_t queueIndices[2] = { g_ovkState->gpu.queueIndexGraphics, g_ovkState->gpu.queueIndexTransfer };
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

VkSurfaceFormatKHR GetSwapchainFormat_Ovk(OpalVulkanWindow* pWindow)
{
  uint32_t count;
  VkSurfaceFormatKHR* formats;

  vkGetPhysicalDeviceSurfaceFormatsKHR(g_ovkState->gpu.device, pWindow->surface, &count, NULL);
  formats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(g_ovkState->gpu.device, pWindow->surface, &count, formats);

  VkSurfaceFormatKHR finalFormat = formats[0];
  for (int i = 0; i < count; i++)
  {
    // TODO : Allow client colorspace/format selection on winodw creation
    //   ex: windowInfo.displayMode = Opal_Display_Hdr10
    if (formats[i].format == VK_FORMAT_R8G8B8A8_UNORM
      && formats[i].colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
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

OpalResult FramesInit_Ovk(OpalWindow* pWindow)
{
  uint32_t createdCount = 0;
  OpalVulkanWindow* window = &pWindow->api.vk;

  // Swapchain images ==========

  OPAL_ATTEMPT_VK(vkGetSwapchainImagesKHR(g_ovkState->device, window->swapchain, &createdCount, NULL));

  window->imageCount = createdCount;
  window->pImages             = OpalMemAllocArray(VkImage,         createdCount);
  window->pImageViews         = OpalMemAllocArray(VkImageView,     createdCount);
  window->pSamplers           = OpalMemAllocArray(VkSampler,       createdCount);
  window->pCmdAvailableFences = OpalMemAllocArray(VkFence,         createdCount);
  window->pImageAvailableSems = OpalMemAllocArray(VkSemaphore,     createdCount);
  window->pRenderCompleteSems = OpalMemAllocArray(VkSemaphore,     createdCount);
  window->pCommandBuffers     = OpalMemAllocArray(VkCommandBuffer, createdCount);

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

  VkFenceCreateInfo fenceInfo = { 0 };
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.pNext = NULL;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (int i = 0; i < createdCount; i++)
  {
    OPAL_ATTEMPT_VK(
      vkCreateFence(g_ovkState->device, &fenceInfo, NULL, &window->pCmdAvailableFences[i]));
    OPAL_ATTEMPT_VK(
      vkCreateSemaphore(g_ovkState->device, &semaphoreInfo, NULL, &window->pImageAvailableSems[i]));
    OPAL_ATTEMPT_VK(
      vkCreateSemaphore(g_ovkState->device, &semaphoreInfo, NULL, &window->pRenderCompleteSems[i]));
  }
  window->syncIndex = 0;

  // Command buffers ==========

  VkCommandBufferAllocateInfo allocInfo = { 0 };
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.pNext = NULL;
  allocInfo.commandBufferCount = createdCount;
  allocInfo.commandPool = g_ovkState->graphicsCommandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

  OPAL_ATTEMPT_VK(vkAllocateCommandBuffers(g_ovkState->device, &allocInfo, window->pCommandBuffers));

  // Images ==========

  pWindow->pImages = OpalMemAllocArrayZeroed(OpalImage, createdCount);
  for (int i = 0; i < createdCount; i++)
  {
    pWindow->pImages[i].width = pWindow->width;
    pWindow->pImages[i].height = pWindow->height;
    pWindow->pImages[i].mipCount = 1;
    pWindow->pImages[i].format = VkFormatToOpalFormat_Ovk(pWindow->api.vk.format);
    pWindow->pImages[i].api.vk.format = pWindow->api.vk.format;
    pWindow->pImages[i].api.vk.image = pWindow->api.vk.pImages[i];
    pWindow->pImages[i].api.vk.view = pWindow->api.vk.pImageViews[i];
    pWindow->pImages[i].api.vk.sampler = pWindow->api.vk.pSamplers[i];
    pWindow->pImages[i].api.vk.memory = VK_NULL_HANDLE;
    pWindow->pImages[i].api.vk.layout = VK_IMAGE_LAYOUT_UNDEFINED;

    OPAL_ATTEMPT(ImageTransitionLayout_Ovk(&pWindow->pImages[i], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));
  }

  return Opal_Success;
}

// Shutdown
// ============================================================

void OpalVulkanWindowShutdown(OpalWindow* pWindow)
{
  vkWaitForFences(g_ovkState->device, pWindow->api.vk.imageCount, pWindow->api.vk.pCmdAvailableFences, VK_TRUE, UINT64_MAX);
  vkFreeCommandBuffers(g_ovkState->device, g_ovkState->graphicsCommandPool, pWindow->api.vk.imageCount, pWindow->api.vk.pCommandBuffers);

  for (int i = 0; i < pWindow->api.vk.imageCount; i++)
  {
    vkDestroyFence(g_ovkState->device, pWindow->api.vk.pCmdAvailableFences[i], NULL);
    vkDestroySemaphore(g_ovkState->device, pWindow->api.vk.pImageAvailableSems[i], NULL);
    vkDestroySemaphore(g_ovkState->device, pWindow->api.vk.pRenderCompleteSems[i], NULL);

    vkDestroyImageView(g_ovkState->device, pWindow->api.vk.pImageViews[i], NULL);
    vkDestroySampler(g_ovkState->device, pWindow->api.vk.pSamplers[i], NULL);
  }

  OpalMemFree(pWindow->api.vk.pCmdAvailableFences);
  OpalMemFree(pWindow->api.vk.pImageAvailableSems);
  OpalMemFree(pWindow->api.vk.pRenderCompleteSems);
  OpalMemFree(pWindow->api.vk.pImages);
  OpalMemFree(pWindow->api.vk.pImageViews);
  OpalMemFree(pWindow->api.vk.pSamplers);

  vkDestroySwapchainKHR(g_ovkState->device, pWindow->api.vk.swapchain, NULL);
  vkDestroySurfaceKHR(g_ovkState->instance, pWindow->api.vk.surface, NULL);
}
