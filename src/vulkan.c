
#include "src/common.h"

VkFormat OpalFormatToVkFormat_Ovk(OpalFormat _format)
{
  switch (_format)
  {
  case Opal_Format_R8 : return VK_FORMAT_R8_UNORM;
  case Opal_Format_RG8 : return VK_FORMAT_R8G8_UNORM;
  case Opal_Format_RGB8 : return VK_FORMAT_R8G8B8_UNORM;
  case Opal_Format_RGBA8 : return VK_FORMAT_R8G8B8A8_UNORM;
  case Opal_Format_R32 : return VK_FORMAT_R32_SFLOAT;
  case Opal_Format_RG32 : return VK_FORMAT_R32G32_SFLOAT;
  case Opal_Format_RGB32 : return VK_FORMAT_R32G32B32_SFLOAT;
  case Opal_Format_RGBA32 : return VK_FORMAT_R32G32B32A32_SFLOAT;
  case Opal_Format_R64 : return VK_FORMAT_R64_SFLOAT;
  case Opal_Format_RG64 : return VK_FORMAT_R64G64_SFLOAT;
  case Opal_Format_RGB64 : return VK_FORMAT_R64G64B64_SFLOAT;
  case Opal_Format_RGBA64 : return VK_FORMAT_R64G64B64A64_SFLOAT;

  case Opal_Format_R8I : return VK_FORMAT_R8_SINT;
  case Opal_Format_RG8I : return VK_FORMAT_R8G8_SINT;
  case Opal_Format_RGB8I : return VK_FORMAT_R8G8B8_SINT;
  case Opal_Format_RGBA8I : return VK_FORMAT_R8G8B8A8_SINT;
  case Opal_Format_R32I : return VK_FORMAT_R32_SINT;
  case Opal_Format_RG32I : return VK_FORMAT_R32G32_SINT;
  case Opal_Format_RGB32I : return VK_FORMAT_R32G32B32_SINT;
  case Opal_Format_RGBA32I : return VK_FORMAT_R32G32B32A32_SINT;
  case Opal_Format_R64I : return VK_FORMAT_R64_SINT;
  case Opal_Format_RG64I : return VK_FORMAT_R64G64_SINT;
  case Opal_Format_RGB64I : return VK_FORMAT_R64G64B64_SINT;
  case Opal_Format_RGBA64I : return VK_FORMAT_R64G64B64A64_SINT;

  case Opal_Format_R8U : return VK_FORMAT_R8_UINT;
  case Opal_Format_RG8U : return VK_FORMAT_R8G8_UINT;
  case Opal_Format_RGB8U : return VK_FORMAT_R8G8B8_UINT;
  case Opal_Format_RGBA8U : return VK_FORMAT_R8G8B8A8_UINT;
  case Opal_Format_R32U : return VK_FORMAT_R32_UINT;
  case Opal_Format_RG32U : return VK_FORMAT_R32G32_UINT;
  case Opal_Format_RGB32U : return VK_FORMAT_R32G32B32_UINT;
  case Opal_Format_RGBA32U : return VK_FORMAT_R32G32B32A32_UINT;
  case Opal_Format_R64U : return VK_FORMAT_R64_UINT;
  case Opal_Format_RG64U : return VK_FORMAT_R64G64_UINT;
  case Opal_Format_RGB64U : return VK_FORMAT_R64G64B64_UINT;
  case Opal_Format_RGBA64U : return VK_FORMAT_R64G64B64A64_UINT;

  // SRGB
  case Opal_Format_R8_Nonlinear : return VK_FORMAT_R8_SRGB;
  case Opal_Format_RG8_Nonlinear : return VK_FORMAT_R8G8_SRGB;
  case Opal_Format_RGB8_Nonlinear : return VK_FORMAT_R8G8B8_SRGB;
  case Opal_Format_RGBA8_Nonlinear : return VK_FORMAT_R8G8B8A8_SRGB;

  // B G R A variants
  case Opal_Format_BGR8 : return VK_FORMAT_B8G8R8A8_UNORM;
  case Opal_Format_BGRA8 : return VK_FORMAT_B8G8R8A8_UNORM;
  case Opal_Format_BGR32 : return VK_FORMAT_R32G32B32_SFLOAT;
  case Opal_Format_BGRA32 : return VK_FORMAT_R32G32B32A32_SFLOAT;
  case Opal_Format_BGR8_Nonlinear: return VK_FORMAT_B8G8R8A8_SRGB;
  case Opal_Format_BGRA8_Nonlinear: return VK_FORMAT_B8G8R8A8_SRGB;

  // Depth stencils
  case Opal_Format_D16_S8 : return VK_FORMAT_D16_UNORM_S8_UINT;
  case Opal_Format_D24_S8 : return VK_FORMAT_D24_UNORM_S8_UINT;
  case Opal_Format_D32 : return VK_FORMAT_D32_SFLOAT;

  default: OpalLog("Vulkan unkown or unsupported opal format %d\n", _format); return VK_FORMAT_UNDEFINED;
  }
}

OpalResult CreateInstance_Ovk(bool _debug)
{
  VkApplicationInfo appInfo = { 0 };
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.apiVersion = VK_API_VERSION_1_0;
  appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
  appInfo.pApplicationName = "Opal test application";
  appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
  appInfo.pEngineName = "Opal test engine";

  uint32_t extensionCount = 0;
  LapisWindowVulkanGetRequiredExtensions(&extensionCount, NULL);

  if (_debug)
    extensionCount++;
  const char** aExtensions = LapisMemAllocZeroArray(const char*, extensionCount);

  if (_debug)
    aExtensions[0] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

  LapisWindowVulkanGetRequiredExtensions(NULL, &aExtensions[_debug]);

  uint32_t layerCount = 0;
  const char** aLayers = NULL;

  if (_debug)
  {
    layerCount = 1;
    aLayers = LapisMemAllocZeroArray(const char*, layerCount);
    aLayers[0] = "VK_LAYER_KHRONOS_validation";
  }

  VkInstanceCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.enabledExtensionCount = extensionCount;
  createInfo.ppEnabledExtensionNames = aExtensions;
  createInfo.enabledLayerCount = layerCount;
  createInfo.ppEnabledLayerNames = aLayers;

  OVK_ATTEMPT(vkCreateInstance(&createInfo, oState.vk.allocator, &oState.vk.instance));

  LapisMemFree(aExtensions);

  if (_debug)
  {
    LapisMemFree(aLayers);
  }

  return Opal_Success;
}

uint32_t GetFamilyIndexForQueue_Ovk(const OpalVkGpu_T* const _gpu, VkQueueFlags _flags)
{
  uint32_t bestFit = ~0U;

  for (uint32_t i = 0; i < _gpu->queueFamilyPropertiesCount; i++)
  {
    uint32_t queueFlag = _gpu->queueFamilyProperties[i].queueFlags;

    if ((queueFlag & _flags) == _flags)
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
    OpalLog("Failed to find a queue family with the flag %u\n", _flags);
  }
  return bestFit;
}

uint32_t GetFamilyIndexForPresent_Ovk(const OpalVkGpu_T* const _gpu, VkSurfaceKHR _surface)
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
    OpalLog("Failed to find a queue family for presentation\n");
  }

  return bestFit;
}

OpalVkGpu_T CreateGpuInfo_Ovk(VkPhysicalDevice _device, VkSurfaceKHR _surface)
{
  OpalVkGpu_T gpu = { 0 };

  gpu.device = _device;

  vkGetPhysicalDeviceProperties(_device, &gpu.properties);
  vkGetPhysicalDeviceFeatures(_device, &gpu.features);
  vkGetPhysicalDeviceMemoryProperties(_device, &gpu.memoryProperties);

  vkGetPhysicalDeviceQueueFamilyProperties(_device, &gpu.queueFamilyPropertiesCount, NULL);
  gpu.queueFamilyProperties = LapisMemAllocArray(VkQueueFamilyProperties, gpu.queueFamilyPropertiesCount);
  vkGetPhysicalDeviceQueueFamilyProperties(_device, &gpu.queueFamilyPropertiesCount, gpu.queueFamilyProperties);

  gpu.queueIndexGraphics = GetFamilyIndexForQueue_Ovk(&gpu, VK_QUEUE_GRAPHICS_BIT);
  gpu.queueIndexTransfer = GetFamilyIndexForQueue_Ovk(&gpu, VK_QUEUE_TRANSFER_BIT);
  gpu.queueIndexPresent = GetFamilyIndexForPresent_Ovk(&gpu, _surface);

  return gpu;
}

void DestroyGpuInfo_Ovk(OpalVkGpu_T _gpu)
{
  LapisMemFree(_gpu.queueFamilyProperties);
}

bool DetermineDeviceSuitability_Ovk(VkPhysicalDevice _device, VkSurfaceKHR _surface)
{
  OpalVkGpu_T gpuInfo = CreateGpuInfo_Ovk(_device, _surface);

  bool isValid = true;

  isValid &= gpuInfo.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
  isValid &= gpuInfo.queueIndexPresent != ~0u;

  DestroyGpuInfo_Ovk(gpuInfo);

  return isValid;
}

OpalResult ChoosePhysicalDevice_Ovk(VkSurfaceKHR _surface)
{
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(oState.vk.instance, &deviceCount, NULL);
  VkPhysicalDevice* aDevices = LapisMemAllocArray(VkPhysicalDevice, deviceCount);
  vkEnumeratePhysicalDevices(oState.vk.instance, &deviceCount, aDevices);

  for (uint32_t i = 0; i < deviceCount; i++)
  {
    if (DetermineDeviceSuitability_Ovk(aDevices[i], _surface))
    {
      oState.vk.gpu = aDevices[i];
      oState.vk.gpuInfo = CreateGpuInfo_Ovk(aDevices[i], _surface);
      return Opal_Success;
    }
  }

  OpalLog("Failed to find a valid physical device\n");
  return Opal_Failure;
}

OpalResult CreateDevice_Ovk(bool _debug)
{
  VkPhysicalDeviceFeatures enabledFeatures = { 0 };

  // Queues =====
  uint32_t queueCount = 3;
  const float queuePriority = 1.0f;
  uint32_t* queueIndices = LapisMemAllocZeroArray(uint32_t, queueCount);
  queueIndices[0] = oState.vk.gpuInfo.queueIndexGraphics;
  queueIndices[1] = oState.vk.gpuInfo.queueIndexTransfer;
  queueIndices[2] = oState.vk.gpuInfo.queueIndexPresent;
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
  const char** aExtensions = LapisMemAllocZeroArray(const char*, extensionCount);
  aExtensions[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

  // Layers =====
  uint32_t layerCount = 0;
  const char** aLayers = NULL;

  if (_debug)
  {
    layerCount = 1;
    aLayers = LapisMemAllocZeroArray(const char*, layerCount);
    aLayers[0] = "VK_LAYER_KHRONOS_validation";
  }

  // Creation =====
  VkDeviceCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.pEnabledFeatures = &enabledFeatures;
  createInfo.queueCreateInfoCount = queueCount;
  createInfo.pQueueCreateInfos = queueCreateInfos;
  createInfo.enabledExtensionCount = extensionCount;
  createInfo.ppEnabledExtensionNames = aExtensions;
  createInfo.enabledLayerCount = layerCount;
  createInfo.ppEnabledLayerNames = aLayers;

  OVK_ATTEMPT(vkCreateDevice(oState.vk.gpu, &createInfo, NULL, &oState.vk.device));

  vkGetDeviceQueue(oState.vk.device, oState.vk.gpuInfo.queueIndexGraphics, 0, &oState.vk.queueGraphics);
  vkGetDeviceQueue(oState.vk.device, oState.vk.gpuInfo.queueIndexTransfer, 0, &oState.vk.queueTransfer);
  vkGetDeviceQueue(oState.vk.device, oState.vk.gpuInfo.queueIndexPresent, 0, &oState.vk.queuePresent);

  LapisMemFree(queueIndices);
  LapisMemFree(queueCreateInfos);
  LapisMemFree(aExtensions);

  if (_debug)
  {
    LapisMemFree(aLayers);
  }

  return Opal_Success;
}

OpalResult CreateCommandPool_Ovk(bool _isTransient)
{
  VkCommandPoolCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  createInfo.pNext = NULL;

  VkCommandPool* outPool = NULL;

  if (_isTransient)
  {
    createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    createInfo.queueFamilyIndex = oState.vk.gpuInfo.queueIndexTransfer;
    outPool = &oState.vk.transientCommandPool;
  }
  else
  {
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = oState.vk.gpuInfo.queueIndexGraphics;
    outPool = &oState.vk.graphicsCommandPool;
  }

  OVK_ATTEMPT(vkCreateCommandPool(oState.vk.device, &createInfo, NULL, outPool));

  return Opal_Success;
}

OpalResult CreateDescriptorPool_Ovk()
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

  OVK_ATTEMPT(vkCreateDescriptorPool(oState.vk.device, &createInfo, NULL, &oState.vk.descriptorPool));

  return Opal_Success;
}

OpalResult CreateVertexFormat_Ovk(uint32_t _count, OpalFormat* _pFormats)
{
  if (_count == 0 || _pFormats == NULL)
  {
    OpalLogError("No vertex format input\n");
    return Opal_Failure;
  }

  VkVertexInputAttributeDescription* pAttribs = LapisMemAllocZeroArray(VkVertexInputAttributeDescription, _count);
  VkVertexInputBindingDescription binding = { 0 };

  uint32_t offsetSum = 0;
  for (uint32_t i = 0; i < _count; i++)
  {
    pAttribs[i].binding = 0;
    pAttribs[i].location = i;
    pAttribs[i].offset = offsetSum;
    pAttribs[i].format = OpalFormatToVkFormat_Ovk(_pFormats[i]);
    offsetSum += OpalFormatToSize_Ovk(_pFormats[i]);
  }

  binding.binding = 0;
  binding.stride = offsetSum;
  binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  oState.vertexFormat.vk.bindingDescription = binding;
  oState.vertexFormat.vk.pAttribDescriptions = pAttribs;

  return Opal_Success;
}

OpalResult OvkInit(OpalInitInfo _initInfo)
{
  oState.vk.allocator = NULL;

  OPAL_ATTEMPT(CreateInstance_Ovk(_initInfo.debug));
  VkSurfaceKHR tmpInitSurface;
  if (OpalPlatformCreateSurface(_initInfo.windowPlatformInfo, oState.vk.instance, &tmpInitSurface))
  {
    OpalLog("Failed to create surface for lapis window\n");
    return Opal_Failure;
  }

  OPAL_ATTEMPT(ChoosePhysicalDevice_Ovk(tmpInitSurface));
  OPAL_ATTEMPT(CreateDevice_Ovk(_initInfo.debug));
  OPAL_ATTEMPT(CreateCommandPool_Ovk(false));
  OPAL_ATTEMPT(CreateCommandPool_Ovk(true));
  OPAL_ATTEMPT(CreateDescriptorPool_Ovk());

  OPAL_ATTEMPT(CreateVertexFormat_Ovk(_initInfo.vertexStruct.count, _initInfo.vertexStruct.pFormats));

  vkDestroySurfaceKHR(oState.vk.instance, tmpInitSurface, oState.vk.allocator);

  OpalLog("Vk init complete : %s\n", oState.vk.gpuInfo.properties.deviceName);
  return Opal_Success;
}

void OvkShutdown()
{
  vkDeviceWaitIdle(oState.vk.device);
  vkDestroyDescriptorPool(oState.vk.device, oState.vk.descriptorPool, oState.vk.allocator);
  vkDestroyCommandPool(oState.vk.device, oState.vk.graphicsCommandPool, oState.vk.allocator);
  vkDestroyCommandPool(oState.vk.device, oState.vk.transientCommandPool, oState.vk.allocator);

  LapisMemFree(oState.vertexFormat.vk.pAttribDescriptions);

  vkDestroyDevice(oState.vk.device, oState.vk.allocator);
  vkDestroyInstance(oState.vk.instance, oState.vk.allocator);

  OpalLog("Vk shutdown complete\n");
}
