
#include "src/defines.h"
#include "src/vulkan/vulkan.h"

#include <vulkan/vulkan.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  uint32_t extensionCount = 0;
  LapisWindowVulkanGetRequiredExtensions(&extensionCount, NULL);
  extensionCount += 2;

  const char** extensions = (char**)LapisMemAllocZero(sizeof(char*) * extensionCount);
  LapisWindowVulkanGetRequiredExtensions(NULL, extensions);
  extensions[1] = VK_KHR_SURFACE_EXTENSION_NAME;
  extensions[2] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

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

  OVK_ATTEMPT(vkCreateInstance(&createInfo, NULL, &_state->instance), return Opal_Failure_Vk_Init);

  LapisMemFree(layers);
  LapisMemFree(extensions);
  return Opal_Success;
}

OpalResult CreateSurface(LapisWindow _window, OvkState_T* _state)
{
  LapisResult result = LapisWindowVulkanCreateSurface(_window, _state->instance, &_state->surface);
  if (result != Lapis_Success)
  {
    return Opal_Failure_Vk_Init;
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
    OPAL_LOG_VK_ERROR("Failed to find a queue family with the flag %u", _flags);
  }
  return bestFit;
}

OvkGpu_T CreateGpuInfo(VkPhysicalDevice _device)
{
  OvkGpu_T gpu = { 0 };

  gpu.device = _device;

  vkGetPhysicalDeviceProperties(_device, &gpu.properties);
  vkGetPhysicalDeviceFeatures(_device, &gpu.features);
  vkGetPhysicalDeviceMemoryProperties(_device, &gpu.memoryProperties);

  vkGetPhysicalDeviceQueueFamilyProperties(_device, &gpu.queueFamilyPropertiesCount, NULL);
  gpu.queueFamilyProperties = (VkQueueFamilyProperties*)LapisMemAlloc(
    sizeof(VkQueueFamilyProperties) * gpu.queueFamilyPropertiesCount);
  vkGetPhysicalDeviceQueueFamilyProperties(
    _device,
    &gpu.queueFamilyPropertiesCount,
    gpu.queueFamilyProperties);

  gpu.queueIndexGraphics = GetFamilyIndexForQueue(&gpu, VK_QUEUE_GRAPHICS_BIT);
  gpu.queueIndexTransfer = GetFamilyIndexForQueue(&gpu, VK_QUEUE_TRANSFER_BIT);

  return gpu;
}

int32_t RatePhysicalDeviceSuitability(VkPhysicalDevice _device)
{
  OvkGpu_T gpu = CreateGpuInfo(_device);

  int score = 0;

  score += 100 * ( gpu.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU );

  OPAL_LOG_VK(Lapis_Console_Debug, "\"%s\" final score = %d\n", gpu.properties.deviceName, score);

  LapisMemFree(gpu.queueFamilyProperties);
  return score;
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
    int32_t score = RatePhysicalDeviceSuitability(devices[i]);

    if (score > winningScore)
    {
      winningScore = score;
      winningIndex = i;
    }
  }

  _state->gpu = CreateGpuInfo(devices[winningIndex]);
  OPAL_LOG_VK(Lapis_Console_Info, "Using \"%s\"\n", _state->gpu.properties.deviceName);

  LapisMemFree(devices);
  return Opal_Success;
}

OpalResult CreateDevice(OvkState_T* _state)
{
  VkPhysicalDeviceFeatures enabledFeatures = { 0 };

  // Queues =====
#define tmpQueueCount 2
  const float queuePriority = 1.0f;
  const uint32_t queueCount = tmpQueueCount;
  uint32_t queueIndices[tmpQueueCount] = {
    _state->gpu.queueIndexGraphics,
    _state->gpu.queueIndexTransfer
  };
  VkDeviceQueueCreateInfo queueCreateInfos[tmpQueueCount] = { 0 };
  for (uint32_t i = 0; i < tmpQueueCount; i++)
  {
    queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[i].pNext = NULL;
    queueCreateInfos[i].flags = 0;
    queueCreateInfos[i].queueCount = 1;
    queueCreateInfos[i].queueFamilyIndex = queueIndices[i];
    queueCreateInfos[i].pQueuePriorities = &queuePriority;
  }
#undef tmpQueueCount

  // Extensions =====
#define tmpExtensionCount 1
  const uint32_t extensionCount = tmpExtensionCount;
  const char* extensions[tmpExtensionCount] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
  };
#undef tmpExtensionCount

  // Layers =====
#define tmpLayerCount 1
  const uint32_t layerCount = tmpLayerCount;
  const char* layers[tmpLayerCount] = {
    "VK_LAYER_KHRONOS_validation"
  };
#undef tmpLayerCount

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
    return Opal_Failure_Vk_Init);

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
    return Opal_Failure_Vk_Init);

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
    return Opal_Failure_Vk_Init);

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
      return Opal_Failure_Vk_Init);
  }

  _state->swapchain.format = format;
  _state->swapchain.extents = extents;
  _state->swapchain.imageCount = imageCount;

  LapisMemFree(formats);
  LapisMemFree(presentModes);
  return Opal_Success;
}

OpalResult OvkInitState(OpalCreateStateInfo _createInfo, OpalState _state)
{
  OvkState_T* vkState = (OvkState_T*)LapisMemAllocZero(sizeof(OvkState_T));

  OPAL_ATTEMPT(
    CreateInstance(vkState),
    {
      OPAL_LOG_VK_ERROR("Failed to create vulkan instance\n");
      return Opal_Failure_Vk_Init;
    });

  // TODO : Move surface creation to window setup
  OPAL_ATTEMPT(
    CreateSurface(_createInfo.window, vkState),
    {
      OPAL_LOG_VK_ERROR("Failed to create vulkan surface\n");
      return Opal_Failure_Vk_Init;
    });

  OPAL_ATTEMPT(
    SelectPhysicalDevice(vkState),
    {
      OPAL_LOG_VK_ERROR("Failed to select a physical device\n");
      return Opal_Failure_Vk_Init;
    });

  OPAL_ATTEMPT(
    CreateDevice(vkState),
    {
      OPAL_LOG_VK_ERROR("Failed to create vkDevice\n");
      return Opal_Failure_Vk_Init;
    });

  OPAL_ATTEMPT(
    CreateCommandPool(vkState, 0),
    {
      OPAL_LOG_VK_ERROR("Failed to create graphics command pool\n");
      return Opal_Failure_Vk_Init;
    });

  OPAL_ATTEMPT(
    CreateCommandPool(vkState, 1),
    {
      OPAL_LOG_VK_ERROR("Failed to create transient command pool\n");
      return Opal_Failure_Vk_Init;
    });

  // TODO : Move swapchain creation to window setup
  OPAL_ATTEMPT(
    CreateSwapchain(vkState, _createInfo.window),
    {
      OPAL_LOG_VK_ERROR("Failed to create swapchain\n");
      return Opal_Failure_Vk_Init;
    });

  // Fences/semaphores
  // Command buffers
  // Renderpass
  // Framebuffers

  OPAL_LOG_VK(Lapis_Console_Info, "Init complete\n");

  _state->backend.state = (void*)vkState;
  return Opal_Success;
}

void OvkShutdownState(OpalState _state)
{
  OpalState_T* state = _state;
  OvkState_T* vkState = (OvkState_T*)state->backend.state;

  for (uint32_t i = 0; i < vkState->swapchain.imageCount; i++)
  {
    vkDestroyImageView(vkState->device, vkState->swapchain.imageViews[i], NULL);
    // Images destroyed with the swapchain itself
  }
  vkDestroySwapchainKHR(vkState->device, vkState->swapchain.swapchain, NULL);
  LapisMemFree(vkState->swapchain.images);
  LapisMemFree(vkState->swapchain.imageViews);

  vkDestroyCommandPool(vkState->device, vkState->transientCommantPool, NULL);
  vkDestroyCommandPool(vkState->device, vkState->graphicsCommandPool, NULL);
  vkDestroyDevice(vkState->device, NULL);
  vkDestroySurfaceKHR(vkState->instance, vkState->surface, NULL);
  vkDestroyInstance(vkState->instance, NULL);

  LapisMemFree(vkState->gpu.queueFamilyProperties);

  OPAL_LOG_VK(Lapis_Console_Info, "Shutdown complete\n");

  LapisMemFree(state->backend.state);
  state->backend.state = NULL;
}
