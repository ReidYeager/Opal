
#include "src/vulkan/vulkan_common.h"

// Declarations
// ============================================================

// Tools ==========
// VkFormat OpalFormatToVkFormat_Ovk(OpalFormat _format);
// VkShaderStageFlags OpalStagesToVkStages_Ovk(OpalStageFlags stages);

// Initialization ==========
// OpalResult     OpalVulkanInit              (OpalInitInfo initInfo)
OpalResult        InitInstance_Ovk            (bool useDebug);
OpalResult        ChoosePhysicalDevice_Ovk    (VkSurfaceKHR surface);
bool              IsDeviceSuitable_Ovk        (VkPhysicalDevice gpu, VkSurfaceKHR surface);
OpalVulkanGpuInfo InitGpuInfo_Ovk             (VkPhysicalDevice gpu, VkSurfaceKHR surface);
void              ShutdownGpuInfo_Ovk         (OpalVulkanGpuInfo* info);
uint32_t          GetFamilyIndexForQueue_Ovk  (const OpalVulkanGpuInfo* const gpu, VkQueueFlags flags);
uint32_t          GetFamilyIndexForPresent_Ovk(const OpalVulkanGpuInfo* const gpu, VkSurfaceKHR surface);
OpalResult        InitDevice_Ovk              (bool useDebug);
OpalResult        InitCommandPool_Ovk         (bool isTransient);
OpalResult        InitDescriptorPool          ();

// Tools
// ============================================================

VkFormat OpalFormatToVkFormat_Ovk(OpalFormat _format)
{
  switch (_format)
  {
  case Opal_Format_R8:       return VK_FORMAT_R8_UNORM;
  case Opal_Format_RG8:      return VK_FORMAT_R8G8_UNORM;
  case Opal_Format_RGB8:     return VK_FORMAT_R8G8B8_UNORM;
  case Opal_Format_RGBA8:    return VK_FORMAT_R8G8B8A8_UNORM;
  case Opal_Format_R16:      return VK_FORMAT_R16_UNORM;
  case Opal_Format_RG16:     return VK_FORMAT_R16G16_UNORM;
  case Opal_Format_RGB16:    return VK_FORMAT_R16G16B16_UNORM;
  case Opal_Format_RGBA16:   return VK_FORMAT_R16G16B16A16_UNORM;
  case Opal_Format_R32:      return VK_FORMAT_R32_SFLOAT;
  case Opal_Format_RG32:     return VK_FORMAT_R32G32_SFLOAT;
  case Opal_Format_RGB32:    return VK_FORMAT_R32G32B32_SFLOAT;
  case Opal_Format_RGBA32:   return VK_FORMAT_R32G32B32A32_SFLOAT;
  case Opal_Format_R64:      return VK_FORMAT_R64_SFLOAT;
  case Opal_Format_RG64:     return VK_FORMAT_R64G64_SFLOAT;
  case Opal_Format_RGB64:    return VK_FORMAT_R64G64B64_SFLOAT;
  case Opal_Format_RGBA64:   return VK_FORMAT_R64G64B64A64_SFLOAT;

  case Opal_Format_R8_I:     return VK_FORMAT_R8_SINT;
  case Opal_Format_RG8_I:    return VK_FORMAT_R8G8_SINT;
  case Opal_Format_RGB8_I:   return VK_FORMAT_R8G8B8_SINT;
  case Opal_Format_RGBA8_I:  return VK_FORMAT_R8G8B8A8_SINT;
  case Opal_Format_R16_I:    return VK_FORMAT_R16_SINT;
  case Opal_Format_RG16_I:   return VK_FORMAT_R16G16_SINT;
  case Opal_Format_RGB16_I:  return VK_FORMAT_R16G16B16_SINT;
  case Opal_Format_RGBA16_I: return VK_FORMAT_R16G16B16A16_SINT;
  case Opal_Format_R32_I:    return VK_FORMAT_R32_SINT;
  case Opal_Format_RG32_I:   return VK_FORMAT_R32G32_SINT;
  case Opal_Format_RGB32_I:  return VK_FORMAT_R32G32B32_SINT;
  case Opal_Format_RGBA32_I: return VK_FORMAT_R32G32B32A32_SINT;
  case Opal_Format_R64_I:    return VK_FORMAT_R64_SINT;
  case Opal_Format_RG64_I:   return VK_FORMAT_R64G64_SINT;
  case Opal_Format_RGB64_I:  return VK_FORMAT_R64G64B64_SINT;
  case Opal_Format_RGBA64_I: return VK_FORMAT_R64G64B64A64_SINT;

  case Opal_Format_R8_U:     return VK_FORMAT_R8_UINT;
  case Opal_Format_RG8_U:    return VK_FORMAT_R8G8_UINT;
  case Opal_Format_RGB8_U:   return VK_FORMAT_R8G8B8_UINT;
  case Opal_Format_RGBA8_U:  return VK_FORMAT_R8G8B8A8_UINT;
  case Opal_Format_R16_U:    return VK_FORMAT_R16_UINT;
  case Opal_Format_RG16_U:   return VK_FORMAT_R16G16_UINT;
  case Opal_Format_RGB16_U:  return VK_FORMAT_R16G16B16_UINT;
  case Opal_Format_RGBA16_U: return VK_FORMAT_R16G16B16A16_UINT;
  case Opal_Format_R32_U:    return VK_FORMAT_R32_UINT;
  case Opal_Format_RG32_U:   return VK_FORMAT_R32G32_UINT;
  case Opal_Format_RGB32_U:  return VK_FORMAT_R32G32B32_UINT;
  case Opal_Format_RGBA32_U: return VK_FORMAT_R32G32B32A32_UINT;
  case Opal_Format_R64_U:    return VK_FORMAT_R64_UINT;
  case Opal_Format_RG64_U:   return VK_FORMAT_R64G64_UINT;
  case Opal_Format_RGB64_U:  return VK_FORMAT_R64G64B64_UINT;
  case Opal_Format_RGBA64_U: return VK_FORMAT_R64G64B64A64_UINT;

    // Depth stencils
  case Opal_Format_D24_S8:   return VK_FORMAT_D24_UNORM_S8_UINT;
  case Opal_Format_D32:      return VK_FORMAT_D32_SFLOAT;

  default: OpalLog("Vulkan unkown or unsupported opal format %d", _format); return VK_FORMAT_UNDEFINED;
  }
}

VkShaderStageFlags OpalStagesToVkStages_Ovk(OpalStageFlags stages)
{
  VkShaderStageFlags flags = 0;

  flags |= VK_SHADER_STAGE_VERTEX_BIT                  * ((stages & Opal_Stage_Vertex)       == Opal_Stage_Vertex);
  flags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT    * ((stages & Opal_Stage_Tesselation)  == Opal_Stage_Tesselation);
  flags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT * ((stages & Opal_Stage_Tesselation)  == Opal_Stage_Tesselation);
  flags |= VK_SHADER_STAGE_GEOMETRY_BIT                * ((stages & Opal_Stage_Geometry)     == Opal_Stage_Geometry);
  flags |= VK_SHADER_STAGE_FRAGMENT_BIT                * ((stages & Opal_Stage_Fragment)     == Opal_Stage_Fragment);
  flags |= VK_SHADER_STAGE_COMPUTE_BIT                 * ((stages & Opal_Stage_Compute)      == Opal_Stage_Compute);
  flags |= VK_SHADER_STAGE_ALL_GRAPHICS                * ((stages & Opal_Stage_All_Graphics) == Opal_Stage_All_Graphics);
  flags |= VK_SHADER_STAGE_ALL                         * ((stages & Opal_Stage_All)          == Opal_Stage_All);

  return flags;
}

// Initialization
// ============================================================

OpalResult OpalVulkanInit(OpalInitInfo initInfo)
{
  OPAL_ATTEMPT(InitInstance_Ovk(initInfo.useDebug));
  OpalLog("Init Vulkan : Instance created");

  VkSurfaceKHR tmpInitSurface;
  OPAL_ATTEMPT(PlatformCreateSurface_Ovk(initInfo.window, &tmpInitSurface));
  OpalLog("Init Vulkan : Surface created");

  OPAL_ATTEMPT(ChoosePhysicalDevice_Ovk(tmpInitSurface));
  OpalLog("Init Vulkan : Chose physical device (%s)", g_OpalState.api.vk.gpu.properties.deviceName);

  OPAL_ATTEMPT(InitDevice_Ovk(initInfo.useDebug));
  OpalLog("Init Vulkan : Device created");

  OPAL_ATTEMPT(InitCommandPool_Ovk(false));
  OPAL_ATTEMPT(InitCommandPool_Ovk(true));
  OpalLog("Init Vulkan : Command pools created");

  OPAL_ATTEMPT(InitDescriptorPool());
  OpalLog("Init Vulkan : Descriptor pool created");

  vkDestroySurfaceKHR(g_OpalState.api.vk.instance, tmpInitSurface, NULL);

  OpalLog("Init Vulkan : complete");
  return Opal_Success;
}

OpalResult InitInstance_Ovk(bool useDebug)
{
  // Extensions ==============================

  const char** pPlatformExtensions = NULL;
  uint32_t platformExtensionCount = PlatformGetExtensions_Ovk(&pPlatformExtensions);

  uint32_t extensionCount = platformExtensionCount + useDebug;
  const char** extensions = NULL;

  if (extensionCount)
  {
    extensions = OpalMemAllocArray(const char*, extensionCount);

    uint32_t index = 0;
    for (; index < platformExtensionCount; index++)
    {
      extensions[index] = pPlatformExtensions[index];
    }

    if (useDebug)
    {
      extensions[index] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
      index++;
    }
  }

  // Layers ==============================

  uint32_t layerCount = 0 + useDebug;
  const char** layers = NULL;

  if (layerCount)
  {
    layers = OpalMemAllocArray(const char*, layerCount);

    uint32_t layerIndex = 0;

    if (useDebug)
    {
      layers[layerIndex] = "VK_LAYER_KHRONOS_validation";
      layerIndex++;
    }
  }

  // Creation ==============================

  VkInstanceCreateInfo createInfo;
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.flags = 0;
  createInfo.pNext = NULL;
  createInfo.pApplicationInfo = NULL;
  createInfo.enabledExtensionCount = extensionCount;
  createInfo.ppEnabledExtensionNames = extensions;
  createInfo.enabledLayerCount = layerCount;
  createInfo.ppEnabledLayerNames = layers;

  OPAL_ATTEMPT_VK(vkCreateInstance(&createInfo, NULL, &g_OpalState.api.vk.instance));

  // Shutdown ==============================

  if (pPlatformExtensions)
  {
    OpalMemFree(pPlatformExtensions);
  }

  if (extensions)
  {
    OpalMemFree(extensions);
  }
  return Opal_Success;
}

OpalResult ChoosePhysicalDevice_Ovk(VkSurfaceKHR surface)
{
  uint32_t deviceCount = 0;
  OPAL_ATTEMPT_VK(vkEnumeratePhysicalDevices(g_OpalState.api.vk.instance, &deviceCount, NULL));
  VkPhysicalDevice* devices = OpalMemAllocArray(VkPhysicalDevice, deviceCount);
  OPAL_ATTEMPT_VK(vkEnumeratePhysicalDevices(g_OpalState.api.vk.instance, &deviceCount, devices));

  for (uint32_t i = 0; i < deviceCount; i++)
  {
    if (IsDeviceSuitable_Ovk(devices[i], surface))
    {
      g_OpalState.api.vk.gpu = InitGpuInfo_Ovk(devices[i], surface);

      OpalMemFree(devices);
      return Opal_Success;
    }
  }

  if (deviceCount)
  {
    OpalMemFree(devices);
  }

  return Opal_Failure_Unknown;
}

bool IsDeviceSuitable_Ovk(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
  bool isSuitable = true;

  OpalVulkanGpuInfo info = InitGpuInfo_Ovk(gpu, surface);

  // TODO : Allow the client to specify desired gpu capabilities in settings
  isSuitable &= info.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
  isSuitable &= info.queueIndexPresent != ~0u;

  ShutdownGpuInfo_Ovk(&info);
  return isSuitable;
}

OpalVulkanGpuInfo InitGpuInfo_Ovk(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
  OpalVulkanGpuInfo info;

  info.device = gpu;

  vkGetPhysicalDeviceProperties(gpu, &info.properties);
  vkGetPhysicalDeviceFeatures(gpu, &info.features);
  vkGetPhysicalDeviceMemoryProperties(gpu, &info.memoryProperties);

  vkGetPhysicalDeviceQueueFamilyProperties(gpu, &info.queueFamilyPropertiesCount, NULL);
  info.queueFamilyProperties = OpalMemAllocArray(VkQueueFamilyProperties, info.queueFamilyPropertiesCount);
  vkGetPhysicalDeviceQueueFamilyProperties(gpu, &info.queueFamilyPropertiesCount, info.queueFamilyProperties);

  info.queueIndexGraphics = GetFamilyIndexForQueue_Ovk(&info, VK_QUEUE_GRAPHICS_BIT);
  info.queueIndexTransfer = GetFamilyIndexForQueue_Ovk(&info, VK_QUEUE_TRANSFER_BIT);
  info.queueIndexPresent = GetFamilyIndexForPresent_Ovk(&info, surface);

  return info;
}

void ShutdownGpuInfo_Ovk(OpalVulkanGpuInfo* info)
{
  OpalMemFree(info->queueFamilyProperties);
}

uint32_t GetFamilyIndexForQueue_Ovk(const OpalVulkanGpuInfo* const gpu, VkQueueFlags flags)
{
  uint32_t bestFit = ~0u;

  for (uint32_t i = 0; i < gpu->queueFamilyPropertiesCount; i++)
  {
    uint32_t queueFlag = gpu->queueFamilyProperties[i].queueFlags;

    if ((queueFlag & flags) == flags)
    {
      // Try to avoid choosing overlapping queue family indices
      if (flags & VK_QUEUE_GRAPHICS_BIT || i != gpu->queueIndexGraphics)
      {
        return i;
      }
      else
      {
        bestFit = i;
      }
    }
  }

  if (bestFit == ~0u)
  {
    OpalLog("Failed to find a queue family with the flag %u", flags);
  }
  return bestFit;
}

uint32_t GetFamilyIndexForPresent_Ovk(const OpalVulkanGpuInfo* const gpu, VkSurfaceKHR surface)
{
  VkBool32 canPresent = VK_FALSE;
  uint32_t bestFit = ~0u;

  for (uint32_t i = 0; i < gpu->queueFamilyPropertiesCount; i++)
  {
    vkGetPhysicalDeviceSurfaceSupportKHR(gpu->device, i, surface, &canPresent);

    if (canPresent == VK_TRUE)
    {
      if (i != gpu->queueIndexGraphics && i != gpu->queueIndexTransfer)
      {
        return i;
      }
      else if (i != gpu->queueIndexTransfer)
      {
        bestFit = i;
      }
    }
  }

  if (bestFit == ~0u)
  {
    OpalLog("Failed to find a queue family for presentation");
  }

  return bestFit;
}

OpalResult InitDevice_Ovk(bool useDebug)
{
  // Queues ==============================

  uint32_t queueCount = 3;
  const float queuePriority = 1.0f;
  uint32_t* queueIndices = OpalMemAllocArray(uint32_t, queueCount);
  queueIndices[0] = g_OpalState.api.vk.gpu.queueIndexGraphics;
  queueIndices[1] = g_OpalState.api.vk.gpu.queueIndexTransfer;
  queueIndices[2] = g_OpalState.api.vk.gpu.queueIndexPresent;
  VkDeviceQueueCreateInfo* queueCreateInfos = OpalMemAllocArray(VkDeviceQueueCreateInfo, queueCount);

  for (uint32_t i = 0; i < queueCount; i++)
  {
    queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[i].pNext = NULL;
    queueCreateInfos[i].flags = 0;
    queueCreateInfos[i].queueCount = 1;
    queueCreateInfos[i].queueFamilyIndex = queueIndices[i];
    queueCreateInfos[i].pQueuePriorities = &queuePriority;
  }

  // Extensions ==============================

  // TODO : Adjust when adding headless mode
  const uint32_t extensionCount = 1;
  const char** extensions = OpalMemAllocArray(const char*, extensionCount);
  extensions[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

  // Layers ==============================

  uint32_t layerCount = useDebug;
  const char** layers = NULL;

  if (layerCount)
  {
    layers = OpalMemAllocArray(const char*, layerCount);
    layers[0] = "VK_LAYER_KHRONOS_validation";
  }

  // Creation ==============================

  VkPhysicalDeviceFeatures enabledFeatures = {};

  VkDeviceCreateInfo createInfo;
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.flags = 0;
  createInfo.pNext = NULL;
  createInfo.pEnabledFeatures = &enabledFeatures;
  createInfo.queueCreateInfoCount = queueCount;
  createInfo.pQueueCreateInfos = queueCreateInfos;
  createInfo.enabledExtensionCount = extensionCount;
  createInfo.ppEnabledExtensionNames = extensions;
  createInfo.enabledLayerCount = layerCount;
  createInfo.ppEnabledLayerNames = layers;

  OPAL_ATTEMPT_VK(vkCreateDevice(g_OpalState.api.vk.gpu.device, &createInfo, NULL, &g_OpalState.api.vk.device));

  vkGetDeviceQueue(g_OpalState.api.vk.device, queueIndices[0], 0, &g_OpalState.api.vk.queueGraphics);
  vkGetDeviceQueue(g_OpalState.api.vk.device, queueIndices[1], 0, &g_OpalState.api.vk.queueTransfer);
  vkGetDeviceQueue(g_OpalState.api.vk.device, queueIndices[2], 0, &g_OpalState.api.vk.queuePresent);

  // Shutdown ==============================

  OpalMemFree(queueIndices);
  OpalMemFree(queueCreateInfos);

  if (extensionCount)
  {
    OpalMemFree(extensions);
  }

  if (layerCount)
  {
    OpalMemFree(layers);
  }

  return Opal_Success;
}

OpalResult InitCommandPool_Ovk(bool isTransient)
{
  VkCommandPoolCreateInfo createInfo;
  createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  createInfo.pNext = NULL;

  VkCommandPool* outPool = NULL;

  if (isTransient)
  {
    createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    createInfo.queueFamilyIndex = g_OpalState.api.vk.gpu.queueIndexTransfer;
    outPool = &g_OpalState.api.vk.transientCommandPool;
  }
  else
  {
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = g_OpalState.api.vk.gpu.queueIndexGraphics;
    outPool = &g_OpalState.api.vk.graphicsCommandPool;
  }

  OPAL_ATTEMPT_VK(vkCreateCommandPool(g_OpalState.api.vk.device, &createInfo, NULL, outPool));

  return Opal_Success;
}

OpalResult InitDescriptorPool()
{
  // TODO : Find a good way to determine required resource counts

  VkDescriptorPoolSize sizes[2];
  sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  sizes[0].descriptorCount = 1024;
  sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  sizes[1].descriptorCount = 1024;

  VkDescriptorPoolCreateInfo createInfo;
  createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  createInfo.pNext = NULL;
  createInfo.maxSets = 1024;
  createInfo.poolSizeCount = 2;
  createInfo.pPoolSizes = sizes;

  OPAL_ATTEMPT_VK(
    vkCreateDescriptorPool(g_OpalState.api.vk.device, &createInfo, NULL, &g_OpalState.api.vk.descriptorPool));

  return Opal_Success;
}
