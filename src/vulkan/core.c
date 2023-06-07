
#include "src/defines.h"
#include "src/vulkan/vulkan_common.h"

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
  const char** extensions = LapisMemAllocZeroArray(char*, extensionCount);
  extensions[0] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

  if (!_state->isHeadless)
  {
    uint32_t windowExtensionCount = 0;
    LapisWindowVulkanGetRequiredExtensions(&windowExtensionCount, NULL);
    extensions = (char**)LapisMemRealloc(extensions, sizeof(char*) * (extensionCount + windowExtensionCount));
    LapisWindowVulkanGetRequiredExtensions(NULL, &extensions[extensionCount]);
    extensionCount += windowExtensionCount;
  }

  // Layers =====
  uint32_t layerCount = 1;
  const char** layers = LapisMemAllocZeroArray(char*, layerCount);
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

  OVK_ATTEMPT(vkCreateInstance(&createInfo, NULL, &_state->instance),
  {
    LapisMemFree(layers);
    LapisMemFree(extensions);
    return Opal_Failure_Vk_Create;
  });

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
    OVK_LOG_ERROR("Failed to create Lapis surface : result = %d\n", result);
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
    OVK_LOG_ERROR("Failed to find a queue family with the flag %u\n", _flags);
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
    OVK_LOG(Lapis_Console_Warning, "Failed to find a queue family for presentation\n");
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
  gpu.queueFamilyProperties = LapisMemAllocArray(VkQueueFamilyProperties, gpu.queueFamilyPropertiesCount);
  vkGetPhysicalDeviceQueueFamilyProperties(_device, &gpu.queueFamilyPropertiesCount, gpu.queueFamilyProperties);

  gpu.queueIndexGraphics = GetFamilyIndexForQueue(&gpu, VK_QUEUE_GRAPHICS_BIT);
  gpu.queueIndexTransfer = GetFamilyIndexForQueue(&gpu, VK_QUEUE_TRANSFER_BIT);
  gpu.queueIndexPresent = GetFamilyIndexForPresent(&gpu, _state->surface);

  return gpu;
}

void DestroyGpuInfo(OvkGpu_T _gpu)
{
  LapisMemFree(_gpu.queueFamilyProperties);
}

int32_t RatePhysicalDeviceSuitability(OvkState_T* _state, VkPhysicalDevice _device)
{
  OvkGpu_T gpu = CreateGpuInfo(_state, _device);

  int32_t score = 0;

#define FatalFeatureAssert(x) { if (!(x)) {return 0x80000000;} }

  score += 100 * ( gpu.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU );
  FatalFeatureAssert(gpu.queueIndexPresent != ~0u);

  DestroyGpuInfo(gpu);
  return score;
#undef FatalFeatureAssert
}

OpalResult SelectPhysicalDevice(OvkState_T* _state)
{
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(_state->instance, &deviceCount, NULL);
  VkPhysicalDevice* devices = LapisMemAllocZeroArray(VkPhysicalDevice, deviceCount);
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
  OVK_LOG(Lapis_Console_Info, "Using \"%s\"\n", _state->gpu.properties.deviceName);

  LapisMemFree(devices);
  return Opal_Success;
}

OpalResult CreateDevice(OvkState_T* _state)
{
  VkPhysicalDeviceFeatures enabledFeatures = { 0 };

  // Queues =====
  uint32_t queueCount = 3;
  const float queuePriority = 1.0f;
  uint32_t* queueIndices = LapisMemAllocZeroArray(uint32_t, queueCount);
  queueIndices[0] = _state->gpu.queueIndexGraphics;
  queueIndices[1] = _state->gpu.queueIndexTransfer;
  queueIndices[2] = _state->gpu.queueIndexPresent;
  VkDeviceQueueCreateInfo* queueCreateInfos = LapisMemAllocZeroArray(VkDeviceQueueCreateInfo, queueCount);

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
  const char** extensions = LapisMemAllocZeroArray(char*, extensionCount);
  extensions[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

  // Layers =====
  uint32_t layerCount = 1;
  const char** layers = LapisMemAllocZeroArray(char*, layerCount);
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

  OVK_ATTEMPT(vkCreateDevice(_state->gpu.device, &createInfo, NULL, &_state->device),
  {
    LapisMemFree(queueIndices);
    LapisMemFree(queueCreateInfos);
    LapisMemFree(extensions);
    LapisMemFree(layers);
    return Opal_Failure_Vk_Create;
  });

  vkGetDeviceQueue(_state->device, _state->gpu.queueIndexGraphics, 0, &_state->queueGraphics);
  vkGetDeviceQueue(_state->device, _state->gpu.queueIndexTransfer, 0, &_state->queueTransfer);
  vkGetDeviceQueue(_state->device, _state->gpu.queueIndexPresent, 0, &_state->queuePresent);

  LapisMemFree(queueIndices);
  LapisMemFree(queueCreateInfos);
  LapisMemFree(extensions);
  LapisMemFree(layers);

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

  OVK_ATTEMPT(vkCreateCommandPool(_state->device, &createInfo, NULL, outPool),
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

  OVK_ATTEMPT(vkCreateDescriptorPool(_state->device, &createInfo, NULL, &_state->descriptorPool),
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
  formats = LapisMemAllocArray(VkSurfaceFormatKHR, formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(_state->gpu.device, _state->surface, &formatCount, formats);

  uint32_t presentModeCount = 0;
  VkPresentModeKHR* presentModes;
  vkGetPhysicalDeviceSurfacePresentModesKHR(_state->gpu.device, _state->surface, &presentModeCount, NULL);
  presentModes = LapisMemAllocArray(VkPresentModeKHR, presentModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(_state->gpu.device, _state->surface, &presentModeCount, presentModes);

  // Choose information =====

  // Image count
  uint32_t imageCount = surfCapabilities.minImageCount + 1;
  if (surfCapabilities.maxImageCount > 0 && imageCount > surfCapabilities.maxImageCount)
  {
    imageCount = surfCapabilities.maxImageCount;
  }

  // Format
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

  // Present mode
  VkPresentModeKHR presentMode = presentModes[0];
  for (uint32_t i = 0; i < presentModeCount; i++)
  {
    if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
    {
      presentMode = presentModes[i];
      break;
    }
  }

  // Extents
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

  OVK_ATTEMPT(vkCreateSwapchainKHR(_state->device, &createInfo, NULL, &_state->swapchain.swapchain),
  {
    LapisMemFree(formats);
    LapisMemFree(presentModes);
    return Opal_Failure_Vk_Create;
  });

  LapisMemFree(formats);
  LapisMemFree(presentModes);

  // Images =====
  vkGetSwapchainImagesKHR(_state->device, _state->swapchain.swapchain, &imageCount, NULL);
  _state->swapchain.images = (VkImage*)LapisMemAlloc(sizeof(VkImage) * imageCount);
  _state->swapchain.imageViews = (VkImageView*)LapisMemAlloc(sizeof(VkImageView) * imageCount);
  vkGetSwapchainImagesKHR(_state->device, _state->swapchain.swapchain, &imageCount, _state->swapchain.images);

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

    OVK_ATTEMPT(vkCreateImageView(_state->device, &ivCreateInfo, NULL, &_state->swapchain.imageViews[i]),
      return Opal_Failure_Vk_Create);
  }

  _state->swapchain.format = format;
  _state->swapchain.extents = extents;
  _state->swapchain.imageCount = imageCount;

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
      return Opal_Failure_Vk_Create);

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
  _state->frameSlots = LapisMemAllocArray(OvkFrame_T, maxFlightSlotCount);
  _state->frameSlotCount = maxFlightSlotCount;

  VkSemaphoreCreateInfo semaphoreCreateInfo = { 0 };
  semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceCreateInfo = { 0 };
  fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (uint32_t i = 0; i < maxFlightSlotCount; i++)
  {
    OvkFrame_T* slot = &_state->frameSlots[i];

    OVK_ATTEMPT(vkCreateFence(_state->device, &fenceCreateInfo, NULL, &slot->fenceFrameAvailable),
    {
      LapisMemFree(_state->frameSlots);
      OVK_LOG_ERROR("Failed to create flight slot fence %d\n", i);
      return Opal_Failure_Vk_Create;
    });

    OVK_ATTEMPT(vkCreateSemaphore(_state->device, &semaphoreCreateInfo, NULL, &slot->semRenderComplete),
    {
      LapisMemFree(_state->frameSlots);
      OVK_LOG_ERROR("Failed to create render complete semaphore %d\n", i);
      return Opal_Failure_Vk_Create;
    });

    OVK_ATTEMPT(vkCreateSemaphore(_state->device, &semaphoreCreateInfo, NULL, &slot->semImageAvailable),
    {
      LapisMemFree(_state->frameSlots);
      OVK_LOG_ERROR("Failed to create image available semaphore %d\n", i);
      return Opal_Failure_Vk_Create;
    });
  }

  return Opal_Success;
}

OpalResult CreateCommandBuffers(OvkState_T* _state)
{
  VkCommandBufferAllocateInfo allocInfo = { 0 };
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.pNext = NULL;
  allocInfo.commandBufferCount = 1;
  allocInfo.commandPool = _state->graphicsCommandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

  for (uint32_t i = 0; i < _state->frameSlotCount; i++)
  {
    OVK_ATTEMPT(vkAllocateCommandBuffers(_state->device, &allocInfo, &_state->frameSlots[i].cmd),
      return Opal_Failure_Vk_Create);
  }

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

  OPAL_ATTEMPT(CreateInstance(state),
  {
    LapisMemFree(state);
    OVK_LOG_ERROR("Failed to create vulkan instance\n");
    return Opal_Failure_Vk_Init;
  });

  // TODO : Move surface creation to window setup
  OPAL_ATTEMPT(CreateSurface(_createInfo.window, state),
  {
    LapisMemFree(state);
    OVK_LOG_ERROR("Failed to create vulkan surface\n");
    return Opal_Failure_Vk_Init;
  });

  OPAL_ATTEMPT(SelectPhysicalDevice(state),
  {
    LapisMemFree(state);
    OVK_LOG_ERROR("Failed to select a physical device\n");
    return Opal_Failure_Vk_Init;
  });

  OPAL_ATTEMPT(CreateDevice(state),
  {
    LapisMemFree(state);
    OVK_LOG_ERROR("Failed to create vkDevice\n");
    return Opal_Failure_Vk_Init;
  });

  OPAL_ATTEMPT(CreateCommandPool(state, 0),
  {
    LapisMemFree(state);
    OVK_LOG_ERROR("Failed to create graphics command pool\n");
    return Opal_Failure_Vk_Init;
  });

  OPAL_ATTEMPT(CreateCommandPool(state, 1),
  {
    LapisMemFree(state);
    OVK_LOG_ERROR("Failed to create transient command pool\n");
    return Opal_Failure_Vk_Init;
  });

  OPAL_ATTEMPT(CreateDescriptorPool(state),
  {
    LapisMemFree(state);
    OVK_LOG_ERROR("Failed to create descriptor pool\n");
    return Opal_Failure_Vk_Init;
  });

  uint32_t count = 0;
  OpalShaderArgTypes* args = NULL;
  if (_createInfo.pCustomObjectShaderArgumentLayout != NULL)
  {
    count = _createInfo.pCustomObjectShaderArgumentLayout->argumentCount;
    args = _createInfo.pCustomObjectShaderArgumentLayout->args;
  }
  OPAL_ATTEMPT(OpalVkCreateDescriptorSetLayout(state, count, args, &state->objectSetLayout),
  {
    LapisMemFree(state);
    OVK_LOG_ERROR("Failed to create object descriptor set layout\n");
    return Opal_Failure_Vk_Init;
  });

  // TODO : Move swapchain creation to window setup
  OPAL_ATTEMPT(CreateSwapchain(state, _createInfo.window),
  {
    LapisMemFree(state);
    OVK_LOG_ERROR("Failed to create swapchain\n");
    return Opal_Failure_Vk_Init;
  });

  // TODO : Modify frames objects to accommodate headless rendering
  OPAL_ATTEMPT(CreateSyncObjects(state),
  {
    LapisMemFree(state);
    OVK_LOG_ERROR("Failed to create sync objects\n");
    return Opal_Failure_Vk_Init;
  });

  OPAL_ATTEMPT(CreateCommandBuffers(state),
  {
    LapisMemFree(state);
    OVK_LOG_ERROR("Failed to create graphics command buffers\n");
    return Opal_Failure_Vk_Init;
  });

  OVK_LOG(Lapis_Console_Info, "Init complete\n");

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

  OVK_LOG(Lapis_Console_Info, "Shutdown complete\n");

  LapisMemFree(state);
  _oState->backend.state = NULL;
}

OpalResult OpalVkRenderFrame(OpalState _oState, const OpalFrameData* _oFrameData)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;
  uint32_t slotIndex = state->currentFrameSlotIndex;
  OvkFrame_T* frameSlot = &state->frameSlots[slotIndex];

  // Setup =====

  OVK_ATTEMPT(vkWaitForFences(state->device, 1, &frameSlot->fenceFrameAvailable, VK_TRUE, UINT64_MAX),
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

  OPAL_ATTEMPT(OvkRecordCommandBuffer(state, frameSlot, _oFrameData),
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

  OVK_ATTEMPT(vkResetFences(state->device, 1, &frameSlot->fenceFrameAvailable),
    return Opal_Failure_Vk_Render);

  OVK_ATTEMPT(vkQueueSubmit(state->queueGraphics, 1, &submitInfo, frameSlot->fenceFrameAvailable),
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

  OVK_ATTEMPT(vkQueuePresentKHR(state->queuePresent, &presentInfo),
    return Opal_Failure_Vk_Render);

  state->currentFrameSlotIndex = (state->currentFrameSlotIndex + 1) % maxFlightSlotCount;

  return Opal_Success;
}

OpalExtents2D OpalVkGetSwapchainExtents(OpalState _oState)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;
  OpalExtents2D out = {state->swapchain.extents.width, state->swapchain.extents.height};
  return out;
}
