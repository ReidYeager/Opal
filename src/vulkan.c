
#include "src/common.h"

OpalResult CreateInstance_Ovk();
OpalResult ChoosePhysicalDevice_Ovk();
uint32_t GetFamilyIndexForQueue_Ovk(const OpalVkGpu_T* const _gpu, VkQueueFlags _flags);
uint32_t GetFamilyIndexForPresent_Ovk(const OpalVkGpu_T* const _gpu, VkSurfaceKHR _surface);
OpalVkGpu_T CreateGpuInfo_Ovk(VkPhysicalDevice _device);
void DestroyGpuInfo_Ovk(OpalVkGpu_T _gpu);
bool DetermineDeviceSuitability_Ovk(VkPhysicalDevice _device);
OpalResult CreateDevice_Ovk();
OpalResult CreateCommandPool_Ovk(bool _isTransient);
OpalResult CreateDescriptorPool_Ovk();

OpalResult OvkInit()
{
  OPAL_ATTEMPT(CreateInstance_Ovk());
  if (LapisWindowVulkanCreateSurface(*oState.window.lWindow, oState.vk.instance, &oState.window.vk.surface) != Lapis_Success)
  {
    OpalLog("Failed to create surface for lapis window\n");
    return Opal_Failure;
  }

  OPAL_ATTEMPT(ChoosePhysicalDevice_Ovk());
  OPAL_ATTEMPT(CreateDevice_Ovk());
  OPAL_ATTEMPT(CreateCommandPool_Ovk(false));
  OPAL_ATTEMPT(CreateCommandPool_Ovk(true));
  OPAL_ATTEMPT(CreateDescriptorPool_Ovk());

  OpalLog("Vk init complete : %s\n", oState.vk.gpuInfo.properties.deviceName);
  return Opal_Success;
}

void OvkShutdown()
{
  vkDestroyDescriptorPool(oState.vk.device, oState.vk.descriptorPool, oState.vk.allocator);
  vkDestroyCommandPool(oState.vk.device, oState.vk.graphicsCommandPool, oState.vk.allocator);
  vkDestroyCommandPool(oState.vk.device, oState.vk.transientCommandPool, oState.vk.allocator);

  vkDestroyDevice(oState.vk.device, oState.vk.allocator);
  vkDestroySurfaceKHR(oState.vk.instance, oState.window.vk.surface, oState.vk.allocator);
  vkDestroyInstance(oState.vk.instance, oState.vk.allocator);

  OpalLog("Vk shutdown complete\n");
}

OpalResult CreateInstance_Ovk()
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
  extensionCount++;
  const char** aExtensions = LapisMemAllocZeroArray(const char*, extensionCount);
  aExtensions[0] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
  LapisWindowVulkanGetRequiredExtensions(NULL, &aExtensions[1]);

  uint32_t layerCount = 1;
  const char** aLayers = LapisMemAllocZeroArray(const char*, layerCount);
  aLayers[0] = "VK_LAYER_KHRONOS_validation";

  VkInstanceCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.enabledExtensionCount = extensionCount;
  createInfo.ppEnabledExtensionNames = aExtensions;
  createInfo.enabledLayerCount = layerCount;
  createInfo.ppEnabledLayerNames = aLayers;

  OVK_ATTEMPT(vkCreateInstance(&createInfo, oState.vk.allocator, &oState.vk.instance));

  LapisMemFree(aExtensions);
  LapisMemFree(aLayers);

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

OpalVkGpu_T CreateGpuInfo_Ovk(VkPhysicalDevice _device)
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
  gpu.queueIndexPresent = GetFamilyIndexForPresent_Ovk(&gpu, oState.window.vk.surface);

  return gpu;
}

void DestroyGpuInfo_Ovk(OpalVkGpu_T _gpu)
{
  LapisMemFree(_gpu.queueFamilyProperties);
}

bool DetermineDeviceSuitability_Ovk(VkPhysicalDevice _device)
{
  OpalVkGpu_T gpuInfo = CreateGpuInfo_Ovk(_device);

  bool isValid = true;

  isValid &= gpuInfo.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
  isValid &= gpuInfo.queueIndexPresent != ~0u;

  DestroyGpuInfo_Ovk(gpuInfo);

  return isValid;
}

OpalResult ChoosePhysicalDevice_Ovk()
{
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(oState.vk.instance, &deviceCount, NULL);
  VkPhysicalDevice* aDevices = LapisMemAllocArray(VkPhysicalDevice, deviceCount);
  vkEnumeratePhysicalDevices(oState.vk.instance, &deviceCount, aDevices);

  for (uint32_t i = 0; i < deviceCount; i++)
  {
    if (DetermineDeviceSuitability_Ovk(aDevices[i]))
    {
      oState.vk.gpu = aDevices[i];
      oState.vk.gpuInfo = CreateGpuInfo_Ovk(aDevices[i]);
      return Opal_Success;
    }
  }

  OpalLog("Failed to find a valid physical device\n");
  return Opal_Failure;
}

OpalResult CreateDevice_Ovk()
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
  uint32_t layerCount = 1;
  const char** aLayers = LapisMemAllocZeroArray(const char*, layerCount);
  aLayers[0] = "VK_LAYER_KHRONOS_validation";

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
  LapisMemFree(aLayers);

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
