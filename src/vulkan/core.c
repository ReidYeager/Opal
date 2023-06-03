
#include "src/defines.h"
#include "src/vulkan/vulkan.h"

#include <vulkan/vulkan.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

VkFormat OpalFormatToVkFormat(OpalFormat _inFormat)
{
  switch (_inFormat)
  {
  case Opal_Format_8_Bit_Int_1: return VK_FORMAT_R8_SINT;
  case Opal_Format_8_Bit_Int_2: return VK_FORMAT_R8G8_SINT;
  case Opal_Format_8_Bit_Int_3: return VK_FORMAT_R8G8B8_SINT;
  case Opal_Format_8_Bit_Int_4: return VK_FORMAT_R8G8B8A8_SINT;
  case Opal_Format_8_Bit_Uint_1: return VK_FORMAT_R8_UINT;
  case Opal_Format_8_Bit_Uint_2: return VK_FORMAT_R8G8_UINT;
  case Opal_Format_8_Bit_Uint_3: return VK_FORMAT_R8G8B8_UINT;
  case Opal_Format_8_Bit_Uint_4: return VK_FORMAT_R8G8B8A8_UINT;
  case Opal_Format_8_Bit_Unorm_1: return VK_FORMAT_R8_UNORM;
  case Opal_Format_8_Bit_Unorm_2: return VK_FORMAT_R8G8_UNORM;
  case Opal_Format_8_Bit_Unorm_3: return VK_FORMAT_R8G8B8_UNORM;
  case Opal_Format_8_Bit_Unorm_4: return VK_FORMAT_R8G8B8A8_UNORM;
  case Opal_Format_32_Bit_Int_1: return VK_FORMAT_R32_SINT;
  case Opal_Format_32_Bit_Int_2: return VK_FORMAT_R32G32_SINT;
  case Opal_Format_32_Bit_Int_3: return VK_FORMAT_R32G32B32_SINT;
  case Opal_Format_32_Bit_Int_4: return VK_FORMAT_R32G32B32A32_SINT;
  case Opal_Format_32_Bit_Uint_1: return VK_FORMAT_R32_UINT;
  case Opal_Format_32_Bit_Uint_2: return VK_FORMAT_R32G32_UINT;
  case Opal_Format_32_Bit_Uint_3: return VK_FORMAT_R32G32B32_UINT;
  case Opal_Format_32_Bit_Uint_4: return VK_FORMAT_R32G32B32A32_UINT;
  case Opal_Format_32_Bit_Float_1: return VK_FORMAT_R32_SFLOAT;
  case Opal_Format_32_Bit_Float_2: return VK_FORMAT_R32G32_SFLOAT;
  case Opal_Format_32_Bit_Float_3: return VK_FORMAT_R32G32B32_SFLOAT;
  case Opal_Format_32_Bit_Float_4: return VK_FORMAT_R32G32B32A32_SFLOAT;
  case Opal_Format_24_Bit_Depth_8_Bit_Stencil : return VK_FORMAT_D24_UNORM_S8_UINT;
  default: return VK_FORMAT_UNDEFINED;
  }
}

OpalResult CreateInstance(OvkState_T* _state)
{
  // App info =====
  VkApplicationInfo appInfo = { 0 };
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pNext = NULL;
  appInfo.apiVersion = VK_API_VERSION_1_3;
  appInfo.pEngineName = "Gem:Opal";
  appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
  appInfo.pApplicationName = "Opal application name";
  appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);

  // Extensions =====
  uint32_t extensionCount = 1;
  const char** extensions = (char**)LapisMemAllocZero(sizeof(char*) * extensionCount);
  extensions[0] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

  if (!_state->isHeadless)
  {
    uint32_t windowExtensionCount = 0;
    LapisWindowVulkanGetRequiredExtensions(&windowExtensionCount, NULL);
    extensions =
      (char**)LapisMemRealloc(extensions, sizeof(char*) * (extensionCount + windowExtensionCount));
    LapisWindowVulkanGetRequiredExtensions(NULL, &extensions[extensionCount]);
    extensionCount += windowExtensionCount;
  }

  // Layers =====
  uint32_t layerCount = 1;
  const char** layers = (char**)LapisMemAllocZero(sizeof(char*) * layerCount);
  layers[0] = "VK_LAYER_KHRONOS_validation";

  // Creation =====
  VkInstanceCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.enabledExtensionCount = extensionCount;
  createInfo.ppEnabledExtensionNames = extensions;
  createInfo.enabledLayerCount = layerCount;
  createInfo.ppEnabledLayerNames = layers;
  createInfo.pApplicationInfo = &appInfo;

  OVK_ATTEMPT(
    vkCreateInstance(&createInfo, NULL, &_state->instance),
    return Opal_Failure_Vk_Create);

  LapisMemFree(layers);
  LapisMemFree(extensions);
  return Opal_Success;
}

OpalResult CreateSurface(LapisWindow _window, OvkState_T* _state)
{
  if (_state->isHeadless)
  {
    return Opal_Success;
  }

  LapisResult result = LapisWindowVulkanCreateSurface(_window, _state->instance, &_state->surface);
  if (result != Lapis_Success)
  {
    return Opal_Failure_Vk_Create;
  }

  return Opal_Success;
}

uint32_t GetFamilyIndexForQueue(const OvkGpu_T* const _gpu, VkQueueFlags _flags)
{
  uint32_t bestFit = ~0U;

  for (uint32_t i = 0; i < _gpu->queueFamilyPropertiesCount; i++)
  {
    uint32_t queueFlag = _gpu->queueFamilyProperties[i].queueFlags;

    if (( queueFlag & _flags ) == _flags)
    {
      // Try to avoid choosing overlapping queue family indices
      if (_flags & VK_QUEUE_GRAPHICS_BIT || i != _gpu->queueIndexGraphics)
      {
        return i;
      }
      else
      {
        bestFit = i;
      }
    }
  }

  if (bestFit == ~0U)
  {
    OPAL_LOG_VK_ERROR("Failed to find a queue family with the flag %u\n", _flags);
  }
  return bestFit;
}

uint32_t GetFamilyIndexForPresent(const OvkGpu_T* const _gpu, VkSurfaceKHR _surface)
{
  VkBool32 canPresent = VK_FALSE;
  uint32_t bestFit = ~0u;

  for (uint32_t i = 0; i < _gpu->queueFamilyPropertiesCount; i++)
  {
    vkGetPhysicalDeviceSurfaceSupportKHR(_gpu->device, i, _surface, &canPresent);

    if (canPresent == VK_TRUE)
    {
      if (i != _gpu->queueIndexGraphics && i != _gpu->queueIndexTransfer)
      {
        return i;
      }
      else if (i != _gpu->queueIndexTransfer)
      {
        bestFit = i;
      }
    }
  }

  if (bestFit == ~0u)
  {
    OPAL_LOG_VK(Lapis_Console_Warning, "Failed to find a queue family for presentation\n");
  }

  return bestFit;
}


OvkGpu_T CreateGpuInfo(OvkState_T* _state, VkPhysicalDevice _device)
{
  OvkGpu_T gpu = { 0 };

  gpu.device = _device;

  vkGetPhysicalDeviceProperties(_device, &gpu.properties);
  vkGetPhysicalDeviceFeatures(_device, &gpu.features);
  vkGetPhysicalDeviceMemoryProperties(_device, &gpu.memoryProperties);

  vkGetPhysicalDeviceQueueFamilyProperties(_device, &gpu.queueFamilyPropertiesCount, NULL);
  gpu.queueFamilyProperties =
    (VkQueueFamilyProperties*)LapisMemAlloc(
      sizeof(VkQueueFamilyProperties) * gpu.queueFamilyPropertiesCount);
  vkGetPhysicalDeviceQueueFamilyProperties(
    _device,
    &gpu.queueFamilyPropertiesCount,
    gpu.queueFamilyProperties);

  gpu.queueIndexGraphics = GetFamilyIndexForQueue(&gpu, VK_QUEUE_GRAPHICS_BIT);
  gpu.queueIndexTransfer = GetFamilyIndexForQueue(&gpu, VK_QUEUE_TRANSFER_BIT);
  gpu.queueIndexPresent = GetFamilyIndexForPresent(&gpu, _state->surface);

  return gpu;
}

int32_t RatePhysicalDeviceSuitability(OvkState_T* _state, VkPhysicalDevice _device)
{
  OvkGpu_T gpu = CreateGpuInfo(_state, _device);

  int32_t score = 0;

#define FatalFeatureAssert(x) { if (!(x)) {return 0x80000000;} }

  score += 100 * ( gpu.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU );
  FatalFeatureAssert(gpu.queueIndexPresent != ~0u);

  LapisMemFree(gpu.queueFamilyProperties);
  return score;
#undef FatalFeatureAssert
}

OpalResult SelectPhysicalDevice(OvkState_T* _state)
{
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(_state->instance, &deviceCount, NULL);
  VkPhysicalDevice* devices = LapisMemAllocZero(sizeof(VkPhysicalDevice) * deviceCount);
  vkEnumeratePhysicalDevices(_state->instance, &deviceCount, devices);

  uint32_t winningIndex = -1;
  int32_t winningScore = INT32_MIN;

  for (uint32_t i = 0; i < deviceCount; i++)
  {
    int32_t score = RatePhysicalDeviceSuitability(_state, devices[i]);

    if (score > winningScore)
    {
      winningScore = score;
      winningIndex = i;
    }
  }

  _state->gpu = CreateGpuInfo(_state, devices[winningIndex]);
  OPAL_LOG_VK(Lapis_Console_Info, "Using \"%s\"\n", _state->gpu.properties.deviceName);

  LapisMemFree(devices);
  return Opal_Success;
}

OpalResult CreateDevice(OvkState_T* _state)
{
  VkPhysicalDeviceFeatures enabledFeatures = { 0 };

  // Queues =====
  uint32_t queueCount = 3;
  const float queuePriority = 1.0f;
  uint32_t* queueIndices = (uint32_t*)LapisMemAllocZero(4 * queueCount);
  queueIndices[0] = _state->gpu.queueIndexGraphics;
  queueIndices[1] = _state->gpu.queueIndexTransfer;
  queueIndices[2] = _state->gpu.queueIndexPresent;
  VkDeviceQueueCreateInfo* queueCreateInfos =
    (VkDeviceQueueCreateInfo*)LapisMemAllocZero(sizeof(VkDeviceQueueCreateInfo) * queueCount);

  for (uint32_t i = 0; i < queueCount; i++)
  {
    queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[i].pNext = NULL;
    queueCreateInfos[i].flags = 0;
    queueCreateInfos[i].queueCount = 1;
    queueCreateInfos[i].queueFamilyIndex = queueIndices[i];
    queueCreateInfos[i].pQueuePriorities = &queuePriority;
  }

  // Extensions =====
  uint32_t extensionCount = 1;
  const char** extensions = (const char**)LapisMemAllocZero(sizeof(char*) * extensionCount);
  extensions[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

  // Layers =====
  uint32_t layerCount = 1;
  const char** layers = (const char**)LapisMemAllocZero(sizeof(char*) * layerCount);
  layers[0] = "VK_LAYER_KHRONOS_validation";

  // Creation =====
  VkDeviceCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.pEnabledFeatures = &enabledFeatures;
  createInfo.queueCreateInfoCount = queueCount;
  createInfo.pQueueCreateInfos = queueCreateInfos;
  createInfo.enabledExtensionCount = extensionCount;
  createInfo.ppEnabledExtensionNames = extensions;
  createInfo.enabledLayerCount = layerCount;
  createInfo.ppEnabledLayerNames = layers;

  OVK_ATTEMPT(
    vkCreateDevice(_state->gpu.device, &createInfo, NULL, &_state->device),
    return Opal_Failure_Vk_Create);

  vkGetDeviceQueue(_state->device, _state->gpu.queueIndexGraphics, 0, &_state->queueGraphics);
  vkGetDeviceQueue(_state->device, _state->gpu.queueIndexTransfer, 0, &_state->queueTransfer);
  vkGetDeviceQueue(_state->device, _state->gpu.queueIndexPresent, 0, &_state->queuePresent);

  return Opal_Success;
}

OpalResult CreateCommandPool(OvkState_T* _state, uint32_t _isTransient)
{
  VkCommandPoolCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  createInfo.pNext = NULL;

  VkCommandPool* outPool = NULL;

  if (_isTransient)
  {
    createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    createInfo.queueFamilyIndex = _state->gpu.queueIndexTransfer;
    outPool = &_state->transientCommantPool;
  }
  else
  {
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = _state->gpu.queueIndexGraphics;
    outPool = &_state->graphicsCommandPool;
  }

  OVK_ATTEMPT(
    vkCreateCommandPool(_state->device, &createInfo, NULL, outPool),
    return Opal_Failure_Vk_Create);

  return Opal_Success;
}

OpalResult CreateDescriptorPool(OvkState_T* _state)
{
  VkDescriptorPoolSize sizes[2] = { 0 };
  sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  sizes[0].descriptorCount = 1024;
  sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  sizes[1].descriptorCount = 1024;

  VkDescriptorPoolCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  createInfo.maxSets = 1024;
  createInfo.poolSizeCount = 2;
  createInfo.pPoolSizes = sizes;

  OVK_ATTEMPT(
    vkCreateDescriptorPool(_state->device, &createInfo, NULL, &_state->descriptorPool),
    return Opal_Failure_Vk_Create);

  return Opal_Success;
}

OpalResult CreateSwapchain(OvkState_T* _state, LapisWindow _window)
{
  // Collect hardware information =====
  VkSurfaceCapabilitiesKHR surfCapabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_state->gpu.device, _state->surface, &surfCapabilities);
  uint32_t formatCount = 0;
  VkSurfaceFormatKHR* formats;
  vkGetPhysicalDeviceSurfaceFormatsKHR(_state->gpu.device, _state->surface, &formatCount, NULL);
  formats = (VkSurfaceFormatKHR*)LapisMemAlloc(sizeof(VkSurfaceFormatKHR) * formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(_state->gpu.device, _state->surface, &formatCount, formats);
  uint32_t presentModeCount = 0;
  VkPresentModeKHR* presentModes;
  vkGetPhysicalDeviceSurfacePresentModesKHR(
    _state->gpu.device,
    _state->surface,
    &presentModeCount,
    NULL);
  presentModes = (VkPresentModeKHR*)LapisMemAlloc(sizeof(VkPresentModeKHR) * presentModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(
    _state->gpu.device,
    _state->surface,
    &presentModeCount,
    presentModes);

  // Choose information =====
  uint32_t imageCount = surfCapabilities.minImageCount + 1;
  if (surfCapabilities.maxImageCount > 0 && imageCount > surfCapabilities.maxImageCount)
  {
    imageCount = surfCapabilities.maxImageCount;
  }

  VkSurfaceFormatKHR format = formats[0];
  for (uint32_t i = 0; i < formatCount; i++)
  {
    if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB
      && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
    {
      format = formats[i];
      break;
    }
  }

  VkPresentModeKHR presentMode = presentModes[0];
  for (uint32_t i = 0; i < presentModeCount; i++)
  {
    if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
    {
      presentMode = presentModes[i];
      break;
    }
  }

  VkExtent2D extents;
  if (surfCapabilities.currentExtent.width != -1)
  {
    extents = surfCapabilities.currentExtent;
  }
  else
  {
    extents.width = LapisWindowGetWidth(_window);
    extents.height = LapisWindowGetHeight(_window);
  }

  // Create =====
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
  createInfo.surface = _state->surface;

  uint32_t queueIndies[2] = { _state->gpu.queueIndexGraphics, _state->gpu.queueIndexTransfer };
  if (_state->gpu.queueIndexGraphics == _state->gpu.queueIndexTransfer)
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }
  else
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueIndies;
  }

  OVK_ATTEMPT(
    vkCreateSwapchainKHR(_state->device, &createInfo, NULL, &_state->swapchain.swapchain),
    return Opal_Failure_Vk_Create);

  // Images =====
  vkGetSwapchainImagesKHR(_state->device, _state->swapchain.swapchain, &imageCount, NULL);
  _state->swapchain.images = (VkImage*)LapisMemAlloc(sizeof(VkImage) * imageCount);
  _state->swapchain.imageViews = (VkImageView*)LapisMemAlloc(sizeof(VkImageView) * imageCount);
  vkGetSwapchainImagesKHR(
    _state->device,
    _state->swapchain.swapchain,
    &imageCount,
    _state->swapchain.images);

  // Image views =====
  VkImageViewCreateInfo ivCreateInfo = { 0 };
  ivCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  ivCreateInfo.pNext = NULL;
  ivCreateInfo.flags = 0;
  ivCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  ivCreateInfo.format = format.format;
  ivCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  ivCreateInfo.subresourceRange.levelCount = 1;
  ivCreateInfo.subresourceRange.baseMipLevel = 0;
  ivCreateInfo.subresourceRange.layerCount = 1;
  ivCreateInfo.subresourceRange.baseArrayLayer = 0;

  for (uint32_t i = 0; i < imageCount; i++)
  {
    ivCreateInfo.image = _state->swapchain.images[i];

    OVK_ATTEMPT(
      vkCreateImageView(_state->device, &ivCreateInfo, NULL, &_state->swapchain.imageViews[i]),
      return Opal_Failure_Vk_Create);
  }

  _state->swapchain.format = format;
  _state->swapchain.extents = extents;
  _state->swapchain.imageCount = imageCount;

  LapisMemFree(formats);
  LapisMemFree(presentModes);
  return Opal_Success;
}

OpalResult CreateDepthBuffers(OvkState_T* _state)
{
  uint32_t imageCount = _state->swapchain.imageCount;
  _state->swapchain.depthImages = (VkImage*)LapisMemAlloc(sizeof(VkImage) * imageCount);
  _state->swapchain.depthImageMemories = (VkDeviceMemory*)LapisMemAlloc(sizeof(VkDeviceMemory) * imageCount);
  _state->swapchain.depthImageViews = (VkImageView*)LapisMemAlloc(sizeof(VkImageView) * imageCount);
  _state->swapchain.depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;

  for (uint32_t i = 0; i < imageCount; i++)
  {
    OPAL_ATTEMPT(
      OvkCreateImage(
        _state,
        _state->swapchain.extents,
        _state->swapchain.depthFormat,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        &_state->swapchain.depthImages[i]),
      return Opal_Failure_Vk_Create);

    OPAL_ATTEMPT(
      OvkCreateImageMemory(
        _state,
        _state->swapchain.depthImages[i],
        &_state->swapchain.depthImageMemories[i]),
      return Opal_Failure_Vk_Create);

    OVK_ATTEMPT(
      vkBindImageMemory(
        _state->device,
        _state->swapchain.depthImages[i],
        _state->swapchain.depthImageMemories[i], 0),
      {
        OPAL_LOG_VK_ERROR("Failed to bind depth image and memory\n");
        return Opal_Failure_Vk_Create;
      });

    OPAL_ATTEMPT(
      OvkCreateImageView(
        _state,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        _state->swapchain.depthImages[i],
        _state->swapchain.depthFormat,
        &_state->swapchain.depthImageViews[i]),
      return Opal_Failure_Vk_Create);
  }

  return Opal_Success;
}

OpalResult CreateSyncObjects(OvkState_T* _state)
{
  _state->frameSlots = (OvkFrame_T*)LapisMemAlloc(sizeof(OvkFrame_T) * maxFlightSlotCount);
  _state->frameSlotCount = maxFlightSlotCount;

  VkSemaphoreCreateInfo semaphoreCreateInfo = { 0 };
  semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceCreateInfo = { 0 };
  fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (uint32_t i = 0; i < maxFlightSlotCount; i++)
  {
    OvkFrame_T* slot = &_state->frameSlots[i];

    OVK_ATTEMPT(
      vkCreateFence(_state->device, &fenceCreateInfo, NULL, &slot->fenceFrameAvailable),
      {
        OPAL_LOG_VK_ERROR("Failed to create flight slot fence %d\n", i);
        return Opal_Failure_Vk_Create;
      });

    OVK_ATTEMPT(
      vkCreateSemaphore(_state->device, &semaphoreCreateInfo, NULL, &slot->semRenderComplete),
      {
        OPAL_LOG_VK_ERROR("Failed to create render complete semaphore %d\n", i);
        return Opal_Failure_Vk_Create;
      });

    OVK_ATTEMPT(
      vkCreateSemaphore(_state->device, &semaphoreCreateInfo, NULL, &slot->semImageAvailable),
      {
        OPAL_LOG_VK_ERROR("Failed to create image available semaphore %d\n", i);
        return Opal_Failure_Vk_Create;
      });
  }

  return Opal_Success;
}

OpalResult CreateCommandBuffers(OvkState_T* _state)
{
  // Graphics command buffers =====

  (VkCommandBuffer*)LapisMemAlloc(sizeof(VkCommandBuffer) * _state->swapchain.imageCount);

  VkCommandBufferAllocateInfo allocInfo = { 0 };
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.pNext = NULL;
  allocInfo.commandBufferCount = 1;
  allocInfo.commandPool = _state->graphicsCommandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

  for (uint32_t i = 0; i < _state->frameSlotCount; i++)
  {
    OVK_ATTEMPT(
      vkAllocateCommandBuffers(_state->device, &allocInfo, &_state->frameSlots[i].cmd),
      return Opal_Failure_Vk_Create);
  }

  return Opal_Success;
}

// TODO : Allow renderpass to be headless
OpalResult OvkCreateRenderpass(
  OvkState_T* _state,
  OvkCreateRenderpassInfo _createInfo,
  VkRenderPass* _outRenderpass)
{
  VkAttachmentDescription* attachments =
    (VkAttachmentDescription*)LapisMemAllocZero(
      sizeof(VkAttachmentDescription) * _createInfo.attachmentCount);
  VkAttachmentReference* attachmentRefs =
    (VkAttachmentReference*)LapisMemAllocZero(
      sizeof(VkAttachmentReference) * _createInfo.attachmentCount);
  uint32_t depthAttachmentIndex = ~0u;

  for (int i = 0; i < _createInfo.attachmentCount; i++)
  {
    OvkRenderpassAttachment inAttachment = _createInfo.attachments[i];

    attachmentRefs[i].attachment = i;

    attachments[i].flags = 0;
    attachments[i].format = inAttachment.dataFormat;
    attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[i].finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    switch (inAttachment.usage)
    {
    case Ovk_Attachment_Usage_Color:
    {
      attachmentRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      attachments[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    } break;
    case Ovk_Attachment_Usage_Depth:
    {
      depthAttachmentIndex = i;
      attachmentRefs[i].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      attachments[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    } break;
    case Ovk_Attachment_Usage_Presented:
    {
      attachmentRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      attachments[i].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    } break;
    default: return Opal_Failure;
    }

    switch (inAttachment.loadOperation)
    {
    case Ovk_Attachment_LoadOp_Load:
    {
      attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      if (inAttachment.usage == Ovk_Attachment_Usage_Presented)
        attachments[i].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
      else
        attachments[i].initialLayout = attachmentRefs[i].layout;
    } break;
    case Ovk_Attachment_LoadOp_Clear:attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;  break;
    case Ovk_Attachment_LoadOp_Dont_Care: attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; break;
    default: return Opal_Failure;
    }

    attachments[i].storeOp =
      inAttachment.shouldStoreReneredData
        ? VK_ATTACHMENT_STORE_OP_STORE
        : VK_ATTACHMENT_STORE_OP_DONT_CARE;
  }

  // Subpasses =====

  VkSubpassDescription* subpasses = (VkSubpassDescription*)LapisMemAlloc(sizeof(VkSubpassDescription) * _createInfo.subpassCount);

  for (uint32_t subpassIndex = 0; subpassIndex < _createInfo.subpassCount; subpassIndex++)
  {
    OpalRenderpassSubpass inSub = _createInfo.subpasses[subpassIndex];

    VkAttachmentReference* colorAttachments = (VkAttachmentReference*)LapisMemAlloc( sizeof(VkAttachmentReference) * inSub.colorAttachmentCount);
    for (uint32_t i = 0; i < inSub.colorAttachmentCount; i++)
    {
      colorAttachments[i] = attachmentRefs[inSub.pColorAttachmentIndices[i]];
    }

    VkAttachmentReference* inputAttachments = (VkAttachmentReference*)LapisMemAlloc( sizeof(VkAttachmentReference) * inSub.inputAttachmentCount);
    for (uint32_t i = 0; i < inSub.inputAttachmentCount; i++)
    {
      inputAttachments[i] = attachmentRefs[inSub.pInputAttachmentIndices[i]];
    }

    VkSubpassDescription sub = { 0 };
    sub.flags = 0;
    sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    if (_createInfo.subpasses[subpassIndex].depthAttachmentIndex != ~0u)
    {
      sub.pDepthStencilAttachment = &attachmentRefs[_createInfo.subpasses[subpassIndex].depthAttachmentIndex];
    }
    sub.colorAttachmentCount = inSub.colorAttachmentCount;
    sub.pColorAttachments = colorAttachments;
    sub.inputAttachmentCount = inSub.inputAttachmentCount;
    sub.pInputAttachments = inputAttachments;
    sub.preserveAttachmentCount = inSub.preserveAttachmentCount;
    sub.pPreserveAttachments = inSub.pPreserveAttachmentIndices;
    sub.pResolveAttachments = NULL;

    subpasses[subpassIndex] = sub;
  }

  // Dependencies =====
  // None needed
  VkSubpassDependency dep = { 0 };
  dep.srcSubpass = 0;
  dep.dstSubpass = 1;
  dep.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dep.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;


  // Creation =====
  VkRenderPassCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.attachmentCount = _createInfo.attachmentCount;
  createInfo.pAttachments = attachments;
  createInfo.subpassCount = _createInfo.subpassCount;
  createInfo.pSubpasses = subpasses;
  createInfo.dependencyCount = 1;
  createInfo.pDependencies = &dep;

  OVK_ATTEMPT(
    vkCreateRenderPass(_state->device, &createInfo, NULL, _outRenderpass),
    return Opal_Failure_Vk_Create);

  return Opal_Success;
}

OpalResult OvkCreateFramebuffer(
  OvkState_T* _state,
  VkExtent2D _extents,
  VkRenderPass _renderpass,
  uint32_t _viewCount,
  VkImageView* _views,
  VkFramebuffer* _outFramebuffer)
{
  VkFramebufferCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.width = _extents.width;
  createInfo.height = _extents.height;
  createInfo.layers = 1;
  createInfo.renderPass = _renderpass;
  createInfo.attachmentCount = _viewCount;
  createInfo.pAttachments = _views;

  OVK_ATTEMPT(
    vkCreateFramebuffer(_state->device, &createInfo, NULL, _outFramebuffer),
    {
      OPAL_LOG_VK_ERROR("Failed to create framebuffer\n");
      return Opal_Failure_Vk_Create;
    });

  return Opal_Success;
}

OpalResult OpalVkCreateRenderpassAndFramebuffers(
  OpalState _oState,
  OpalCreateRenderpassInfo _createInfo,
  OpalRenderpass _outRenderpass)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;
  OvkRenderpass_T* outOvkRp = &_outRenderpass->backend.vulkan;

  uint32_t fbCount = _createInfo.rendersToSwapchain ? state->swapchain.imageCount : 1;
  uint32_t attachmentCount = _createInfo.imageCount + (_createInfo.rendersToSwapchain ? 1 : 0);
  VkImageView* views = (VkImageView*)LapisMemAllocZero(sizeof(VkImageView) * attachmentCount);

  // Renderpass =====

  OvkCreateRenderpassInfo rpInfo = { 0 };
  rpInfo.attachmentCount = attachmentCount;
  rpInfo.attachments = (OvkRenderpassAttachment*)LapisMemAllocZero(sizeof(OvkRenderpassAttachment) * attachmentCount);
  rpInfo.subpassCount = _createInfo.subpassCount;
  rpInfo.subpasses = _createInfo.subpasses;

  for (uint32_t i = 0; i < _createInfo.imageCount; i++)
  {
    rpInfo.attachments[i].dataFormat = _createInfo.images[i]->backend.vulkan.format;
    rpInfo.attachments[i].shouldStoreReneredData = _createInfo.imageAttachments[i].shouldStoreReneredData;
    rpInfo.attachments[i].loadOperation = (OvkRenderpassAttachmentLoadOp)_createInfo.imageAttachments[i].loadOperation;
    rpInfo.attachments[i].usage = (OvkRenderpassAttachmentUsage)_createInfo.imageAttachments[i].usage;

    views[i] = _createInfo.images[i]->backend.vulkan.view;
  }
  if (_createInfo.rendersToSwapchain)
  {
    rpInfo.attachments[attachmentCount - 1].dataFormat = state->swapchain.format.format;
    rpInfo.attachments[attachmentCount - 1].shouldStoreReneredData = 1;
    rpInfo.attachments[attachmentCount - 1].loadOperation = Ovk_Attachment_LoadOp_Clear;
    rpInfo.attachments[attachmentCount - 1].usage = Ovk_Attachment_Usage_Presented;

    OpalRenderpassSubpass* finalSub = &_createInfo.subpasses[_createInfo.subpassCount - 1];
    uint32_t colorCount = finalSub->colorAttachmentCount;
    uint32_t** colorIndices = &finalSub->pColorAttachmentIndices;
    *colorIndices = (uint32_t*)LapisMemRealloc(*colorIndices, 4 * colorCount + 1);
    *colorIndices[colorCount] = _createInfo.imageCount;
    finalSub->colorAttachmentCount++;
  }

  OvkCreateRenderpass(state, rpInfo, &outOvkRp->renderpass);

  // Framebuffers =====

  outOvkRp->framebufferCount = fbCount;
  outOvkRp->framebuffers = (VkFramebuffer*)LapisMemAlloc(sizeof(VkFramebuffer) * fbCount);

  for (uint32_t i = 0; i < fbCount; i++)
  {
    views[_createInfo.imageCount] = state->swapchain.imageViews[i];
    OvkCreateFramebuffer(
      state,
      (VkExtent2D){_createInfo.images[0]->extents.width, _createInfo.images[0]->extents.height},
      outOvkRp->renderpass,
      attachmentCount,
      views,
      &outOvkRp->framebuffers[i]);
  }

  return Opal_Success;
}

OpalResult OpalVkCreateDescriptorSetLayout(
  OvkState_T* _state,
  uint32_t _shaderArgCount,
  OpalShaderArgTypes* _pShaderArgs,
  VkDescriptorSetLayout* _outLayout)
{
  VkDescriptorSetLayoutBinding* bindings = (VkDescriptorSetLayoutBinding*)LapisMemAllocZero(
    sizeof(VkDescriptorSetLayoutBinding) * _shaderArgCount);
  VkDescriptorSetLayoutBinding newBinding = { 0 };
  newBinding.descriptorCount = 1;
  newBinding.pImmutableSamplers = NULL;
  newBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

  for (uint32_t i = 0; i < _shaderArgCount; i++)
  {
    newBinding.binding = i;
    switch (_pShaderArgs[i])
    {
    case Opal_Shader_Arg_Uniform_Buffer:
    {
      newBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    } break;
    case Opal_Shader_Arg_Samped_Image:
    {
      newBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    } break;
    default:
    {
      LapisMemFree(bindings);
      return Opal_Failure_Vk_Create;
    }
    }

    bindings[i] = newBinding;
  }

  VkDescriptorSetLayoutCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.bindingCount = _shaderArgCount;
  createInfo.pBindings = bindings;

  OVK_ATTEMPT(
    vkCreateDescriptorSetLayout(_state->device, &createInfo, NULL, _outLayout),
    return Opal_Failure_Vk_Create);

  return Opal_Success;
}

OpalResult OpalVkCreateDescriptorSet(
  OvkState_T* _state,
  VkDescriptorSetLayout _layout,
  VkDescriptorSet* _outSet)
{
  VkDescriptorSetAllocateInfo allocInfo = { 0 };
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.pNext = NULL;
  allocInfo.descriptorPool = _state->descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &_layout;

  OVK_ATTEMPT(
    vkAllocateDescriptorSets(_state->device, &allocInfo, _outSet),
    return Opal_Failure_Vk_Create);

  return Opal_Success;
}

OpalResult OpalVkCreateRenderable(
  OpalState _oState,
  OpalShaderArg* _objectArguments,
  OpalRenderable _renderable)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;

  OPAL_ATTEMPT(
    OpalVkCreateDescriptorSet(state, state->objectSetLayout, &_renderable->backend.vulkan.descSet),
    return Opal_Failure_Vk_Create);

  OPAL_ATTEMPT(
    OvkUpdateShaderArguments(
      state,
      _oState->objectShaderArgsInfo.argumentCount,
      _objectArguments,
      _renderable->backend.vulkan.descSet),
    return Opal_Failure_Vk_Create);

  return Opal_Success;
}

OpalResult OvkCreateMesh(
  OvkState_T* _state,
  uint32_t _objectArgCount,
  OpalShaderArg* _objectArguments,
  OpalMesh _mesh)
{
  OPAL_ATTEMPT(
    OpalVkCreateDescriptorSet(
      _state,
      _state->objectSetLayout,
      &_mesh->backend.vulkan.descriptorSet),
    return Opal_Failure_Vk_Create);

  OPAL_ATTEMPT(
    OvkUpdateShaderArguments(
      _state,
      _objectArgCount,
      _objectArguments,
      _mesh->backend.vulkan.descriptorSet),
    return Opal_Failure_Vk_Create);

  return Opal_Success;
}

OpalResult OpalVkInitState(OpalCreateStateInfo _createInfo, OpalState _oState)
{
  OvkState_T* state = (OvkState_T*)LapisMemAllocZero(sizeof(OvkState_T));
  _oState->backend.state = (void*)state;

  if (_createInfo.window == NULL)
  {
    state->isHeadless = 1;
  }

  OPAL_ATTEMPT(
    CreateInstance(state),
    {
      OPAL_LOG_VK_ERROR("Failed to create vulkan instance\n");
      return Opal_Failure_Vk_Init;
    });

  // TODO : Move surface creation to window setup
  OPAL_ATTEMPT(
    CreateSurface(_createInfo.window, state),
    {
      OPAL_LOG_VK_ERROR("Failed to create vulkan surface\n");
      return Opal_Failure_Vk_Init;
    });

  OPAL_ATTEMPT(
    SelectPhysicalDevice(state),
    {
      OPAL_LOG_VK_ERROR("Failed to select a physical device\n");
      return Opal_Failure_Vk_Init;
    });

  OPAL_ATTEMPT(
    CreateDevice(state),
    {
      OPAL_LOG_VK_ERROR("Failed to create vkDevice\n");
      return Opal_Failure_Vk_Init;
    });

  OPAL_ATTEMPT(
    CreateCommandPool(state, 0),
    {
      OPAL_LOG_VK_ERROR("Failed to create graphics command pool\n");
      return Opal_Failure_Vk_Init;
    });

  OPAL_ATTEMPT(
    CreateCommandPool(state, 1),
    {
      OPAL_LOG_VK_ERROR("Failed to create transient command pool\n");
      return Opal_Failure_Vk_Init;
    });

  OPAL_ATTEMPT(
    CreateDescriptorPool(state),
    {
      OPAL_LOG_VK_ERROR("Failed to create descriptor pool\n");
      return Opal_Failure_Vk_Init;
    });

  uint32_t count = 0;
  OpalShaderArgTypes* args = NULL;
  if (_createInfo.pCustomObjectShaderArgumentLayout != NULL)
  {
    count = _createInfo.pCustomObjectShaderArgumentLayout->argumentCount;
    args = _createInfo.pCustomObjectShaderArgumentLayout->args;
  }
  OPAL_ATTEMPT(
    OpalVkCreateDescriptorSetLayout(state, count, args, &state->objectSetLayout),
    {
      OPAL_LOG_VK_ERROR("Failed to create object descriptor set layout\n");
      return Opal_Failure_Vk_Init;
    });

  // TODO : Move swapchain creation to window setup
  OPAL_ATTEMPT(
    CreateSwapchain(state, _createInfo.window),
    {
      OPAL_LOG_VK_ERROR("Failed to create swapchain\n");
      return Opal_Failure_Vk_Init;
    });

  // TODO : Modify frames objects to accommodate headless rendering
  OPAL_ATTEMPT(
    CreateSyncObjects(state),
    {
      OPAL_LOG_VK_ERROR("Failed to create sync objects\n");
      return Opal_Failure_Vk_Init;
    });

  OPAL_ATTEMPT(
    CreateCommandBuffers(state),
    {
      OPAL_LOG_VK_ERROR("Failed to create graphics command buffers\n");
      return Opal_Failure_Vk_Init;
    });

  OPAL_LOG_VK(Lapis_Console_Info, "Init complete\n");

  return Opal_Success;
}

void OpalVkShutdownState(OpalState _oState)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;

  vkDeviceWaitIdle(state->device);

  //for (uint32_t i = 0; i < state->swapchain.imageCount; i++)
  //{
  //  vkDestroyFramebuffer(state->device, state->framebuffers[i], NULL);
  //}
  //LapisMemFree(state->framebuffers);

  //vkDestroyRenderPass(state->device, state->renderpass, NULL);

  for (uint32_t i = 0; i < state->frameSlotCount; i++)
  {
    OvkFrame_T* slot = &state->frameSlots[i];
    vkDestroyFence(state->device, slot->fenceFrameAvailable, NULL);
    vkDestroySemaphore(state->device, slot->semImageAvailable, NULL);
    vkDestroySemaphore(state->device, slot->semRenderComplete, NULL);
    vkFreeCommandBuffers(state->device, state->graphicsCommandPool, 1, &slot->cmd);
  }
  LapisMemFree(state->frameSlots);

  for (uint32_t i = 0; i < state->swapchain.imageCount; i++)
  {
    vkDestroyImageView(state->device, state->swapchain.imageViews[i], NULL);
    // Images destroyed with the swapchain itself
  }
  vkDestroySwapchainKHR(state->device, state->swapchain.swapchain, NULL);
  LapisMemFree(state->swapchain.images);
  LapisMemFree(state->swapchain.imageViews);

  vkDestroyDescriptorPool(state->device, state->descriptorPool, NULL);
  vkDestroyCommandPool(state->device, state->transientCommantPool, NULL);
  vkDestroyCommandPool(state->device, state->graphicsCommandPool, NULL);
  vkDestroyDevice(state->device, NULL);
  vkDestroySurfaceKHR(state->instance, state->surface, NULL);
  vkDestroyInstance(state->instance, NULL);

  LapisMemFree(state->gpu.queueFamilyProperties);

  OPAL_LOG_VK(Lapis_Console_Info, "Shutdown complete\n");

  LapisMemFree(state);
  _oState->backend.state = NULL;
}

OpalResult OpalVkRenderFrame(OpalState _oState, const OpalFrameData* _oFrameData)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;
  uint32_t slotIndex = state->currentFrameSlotIndex;
  OvkFrame_T* frameSlot = &state->frameSlots[slotIndex];

  // Setup =====

  OVK_ATTEMPT(
    vkWaitForFences(state->device, 1, &frameSlot->fenceFrameAvailable, VK_TRUE, UINT64_MAX),
    return Opal_Failure_Vk_Render);

  OVK_ATTEMPT(
    vkAcquireNextImageKHR(
      state->device,
      state->swapchain.swapchain,
      UINT64_MAX,
      frameSlot->semImageAvailable,
      VK_NULL_HANDLE,
      &frameSlot->swapchainImageIndex),
    return Opal_Failure_Vk_Render);

  // Record =====

  OPAL_ATTEMPT(
    OvkRecordCommandBuffer(state, frameSlot, _oFrameData),
    return Opal_Failure_Vk_Render);

  // Render =====

  VkPipelineStageFlags waitStages[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  VkSubmitInfo submitInfo = { 0 };
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext = NULL;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &frameSlot->semImageAvailable;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &frameSlot->cmd;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &frameSlot->semRenderComplete;

  OVK_ATTEMPT(
    vkResetFences(state->device, 1, &frameSlot->fenceFrameAvailable),
    return Opal_Failure_Vk_Render);

  OVK_ATTEMPT(
    vkQueueSubmit(state->queueGraphics, 1, &submitInfo, frameSlot->fenceFrameAvailable),
    return Opal_Failure_Vk_Render);

  // Present =====

  VkPresentInfoKHR presentInfo = { 0 };
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = NULL;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &frameSlot->semRenderComplete;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &state->swapchain.swapchain;
  presentInfo.pImageIndices = &frameSlot->swapchainImageIndex;

  OVK_ATTEMPT(vkQueuePresentKHR(state->queuePresent, &presentInfo), return Opal_Failure_Vk_Render);

  state->currentFrameSlotIndex = (state->currentFrameSlotIndex + 1) % maxFlightSlotCount;

  return Opal_Success;
}

OpalExtents2D OpalVkGetSwapchainExtents(OpalState _oState)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;
  OpalExtents2D out = {state->swapchain.extents.width, state->swapchain.extents.height};
  return out;
}
