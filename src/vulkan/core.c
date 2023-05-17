
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
#define tmpDefineQueueCount 3
  const float queuePriority = 1.0f;
  uint32_t queueIndices[tmpDefineQueueCount] = {
    _state->gpu.queueIndexGraphics,
    _state->gpu.queueIndexTransfer,
    _state->gpu.queueIndexPresent
  };
  VkDeviceQueueCreateInfo queueCreateInfos[tmpDefineQueueCount] = { 0 };
  for (uint32_t i = 0; i < tmpDefineQueueCount; i++)
  {
    queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[i].pNext = NULL;
    queueCreateInfos[i].flags = 0;
    queueCreateInfos[i].queueCount = 1;
    queueCreateInfos[i].queueFamilyIndex = queueIndices[i];
    queueCreateInfos[i].pQueuePriorities = &queuePriority;
  }

  // Extensions =====
#define tmpDefineExtensionCount 1
  const char* extensions[tmpDefineExtensionCount] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
  };

  // Layers =====
#define tmpDefineLayerCount 1
  const char* layers[tmpDefineLayerCount] = {
    "VK_LAYER_KHRONOS_validation"
  };

  // Creation =====
  VkDeviceCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.pEnabledFeatures = &enabledFeatures;
  createInfo.queueCreateInfoCount = tmpDefineQueueCount;
  createInfo.pQueueCreateInfos = queueCreateInfos;
  createInfo.enabledExtensionCount = tmpDefineExtensionCount;
  createInfo.ppEnabledExtensionNames = extensions;
  createInfo.enabledLayerCount = tmpDefineLayerCount;
  createInfo.ppEnabledLayerNames = layers;

  OVK_ATTEMPT(
    vkCreateDevice(_state->gpu.device, &createInfo, NULL, &_state->device),
    return Opal_Failure_Vk_Create);

  vkGetDeviceQueue(_state->device, _state->gpu.queueIndexGraphics, 0, &_state->queueGraphics);
  vkGetDeviceQueue(_state->device, _state->gpu.queueIndexTransfer, 0, &_state->queueTransfer);
  vkGetDeviceQueue(_state->device, _state->gpu.queueIndexPresent, 0, &_state->queuePresent);

  return Opal_Success;
#undef tmpDefineQueueCount
#undef tmpDefineExtensionCount
#undef tmpDefineLayerCount
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
OpalResult CreateRenderpass(OvkState_T* _state)
{
  // Attachments =====
#define tmpAttachmentCount 1
  VkAttachmentDescription attachments[tmpAttachmentCount] = { 0 };
  // Swapchain
  attachments[0].flags = 0;
  attachments[0].format = _state->swapchain.format.format;
  attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  // Attachment references =====
  VkAttachmentReference attachmentRefs[tmpAttachmentCount] = { 0 };
  // Swapchain
  attachmentRefs[0].attachment = 0;
  attachmentRefs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // Subpasses =====
  VkSubpassDescription subpass = { 0 };
  subpass.flags = 0;
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &attachmentRefs[0];
  subpass.pDepthStencilAttachment = NULL;
  subpass.inputAttachmentCount = 0;
  subpass.pInputAttachments = NULL;
  subpass.preserveAttachmentCount = 0;
  subpass.pPreserveAttachments = NULL;
  subpass.pResolveAttachments = NULL;

  // Dependencies =====
  // None needed

  // Creation =====
  VkRenderPassCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.attachmentCount = tmpAttachmentCount;
  createInfo.pAttachments = attachments;
  createInfo.dependencyCount = 0;
  createInfo.pDependencies = NULL;
  createInfo.subpassCount = 1;
  createInfo.pSubpasses = &subpass;

  OVK_ATTEMPT(
    vkCreateRenderPass(_state->device, &createInfo, NULL, &_state->renderpass),
    return Opal_Failure_Vk_Create);

#undef tmpAttachmentCount
  return Opal_Success;
}

OpalResult CreateFramebuffers(OvkState_T* _state)
{
  const uint32_t framebufferCount = _state->swapchain.imageCount;
  const uint32_t viewsPerFramebuffer = 1; // Just swapchain color

  _state->framebuffers = (VkFramebuffer*)LapisMemAlloc(sizeof(VkFramebuffer) * framebufferCount);

  VkImageView* viewAttachments =
    (VkImageView*)LapisMemAlloc(sizeof(VkImageView) * framebufferCount);

  for (uint32_t i = 0; i < framebufferCount; i++)
  {
    viewAttachments[i * viewsPerFramebuffer + 0] = _state->swapchain.imageViews[i];
    // Add depth, etc. here
  }

  VkFramebufferCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.width = _state->swapchain.extents.width;
  createInfo.height = _state->swapchain.extents.height;
  createInfo.layers = 1;
  createInfo.renderPass = _state->renderpass;
  createInfo.attachmentCount = viewsPerFramebuffer;

  for (uint32_t i = 0; i < framebufferCount; i++)
  {
    createInfo.pAttachments = &viewAttachments[i * viewsPerFramebuffer];
    OVK_ATTEMPT(
      vkCreateFramebuffer(_state->device, &createInfo, NULL, &_state->framebuffers[i]),
      {
        OPAL_LOG_VK_ERROR("Failed to create framebuffer %d\n", i);
        return Opal_Failure_Vk_Create;
      });
  }

  return Opal_Success;
}

OpalResult OvkCreateDescriptorSetLayout(
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

OpalResult OvkCreateDescriptorSet(
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

OpalResult OvkCreateRenderable(
  OpalState _oState,
  OpalShaderArg* _objectArguments,
  OpalRenderable _renderable)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;
  OvkCreateDescriptorSet(
    state,
    state->objectSetLayout,
    &_renderable->backend.vulkan.descSet);

  UpdateShaderArguments(
    state,
    _oState->objectShaderArgsInfo.argumentCount,
    _objectArguments,
    _renderable->backend.vulkan.descSet);

  return Opal_Success;
}

OpalResult OvkInitState(OpalCreateStateInfo _createInfo, OpalState _oState)
{
  OvkState_T* state = (OvkState_T*)LapisMemAllocZero(sizeof(OvkState_T));

  if (_createInfo.window == NULL)
  {
    state->isHeadless = 1;
  }

  OPAL_ATTEMPT(
    CreateInstance(state),
      OPAL_LOG_VK_ERROR("Failed to create vulkan instance\n");
      return Opal_Failure_Vk_Init;);

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
    OvkCreateDescriptorSetLayout(state, count, args, &state->objectSetLayout),
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

  OPAL_ATTEMPT(
    CreateRenderpass(state),
    {
      OPAL_LOG_VK_ERROR("Failed to create renderpass\n");
      return Opal_Failure_Vk_Init;
    });

  OPAL_ATTEMPT(
    CreateFramebuffers(state),
    {
      OPAL_LOG_VK_ERROR("Failed to create framebuffers\n");
      return Opal_Failure_Vk_Init;
    });

  OPAL_LOG_VK(Lapis_Console_Info, "Init complete\n");

  _oState->backend.state = (void*)state;
  return Opal_Success;
}

void OvkShutdownState(OpalState _oState)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;

  vkDeviceWaitIdle(state->device);

  for (uint32_t i = 0; i < state->swapchain.imageCount; i++)
  {
    vkDestroyFramebuffer(state->device, state->framebuffers[i], NULL);
  }
  LapisMemFree(state->framebuffers);

  vkDestroyRenderPass(state->device, state->renderpass, NULL);

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

OpalResult OvkRenderFrame(OpalState _oState, const OpalFrameData* _oFrameData)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;
  uint32_t slotIndex = state->currentFrameSlotIndex;
  OvkFrame_T* frameSlot = &state->frameSlots[slotIndex];

  // Setup =====

  OVK_ATTEMPT(
    // 16 ms timeout
    vkWaitForFences(state->device, 1, &frameSlot->fenceFrameAvailable, VK_TRUE, 16000000),
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
