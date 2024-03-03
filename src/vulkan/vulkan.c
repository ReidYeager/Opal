
#include "src/vulkan/vulkan_common.h"

// Global variables
// ============================================================

OpalVulkanState* g_ovkState = NULL;

// Declarations
// ============================================================

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
OpalResult        InitDescriptorPool_Ovk      ();
OpalResult        InitGlobalResources_Ovk     (OpalInitInfo initInfo);

// Shutdown ==========
//void OpalVulkanShutdown();

// Tools ==========
//void OpalVulkanWaitIdle();

// Initialization
// ============================================================

OpalResult OpalVulkanInit(OpalInitInfo initInfo)
{
  if (g_ovkState != NULL)
  {
    OpalLogError("Vulkan state already initialized");
    return Opal_Failure_Unknown;
  }

  g_ovkState = &g_OpalState.api.vk;

  OPAL_ATTEMPT(InitInstance_Ovk(initInfo.useDebug));
  OpalLog("Init Vulkan : Instance created");

  VkSurfaceKHR tmpInitSurface;
  OPAL_ATTEMPT(PlatformCreateSurface_Ovk(initInfo.window, &tmpInitSurface));
  OpalLog("Init Vulkan : Surface created");

  OPAL_ATTEMPT(ChoosePhysicalDevice_Ovk(tmpInitSurface));
  OpalLog("Init Vulkan : Chose physical device (%s)", g_ovkState->gpu.properties.deviceName);

  OPAL_ATTEMPT(InitDevice_Ovk(initInfo.useDebug));
  OpalLog("Init Vulkan : Device created");

  OPAL_ATTEMPT(InitCommandPool_Ovk(false));
  OPAL_ATTEMPT(InitCommandPool_Ovk(true));
  OpalLog("Init Vulkan : Command pools created");

  OPAL_ATTEMPT(InitDescriptorPool_Ovk());
  OpalLog("Init Vulkan : Descriptor pool created");

  OPAL_ATTEMPT(InitGlobalResources_Ovk(initInfo));
  OpalLog("Init Vulkan : Base render resources created");

  vkDestroySurfaceKHR(g_ovkState->instance, tmpInitSurface, NULL);

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

  VkInstanceCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.pApplicationInfo = NULL;
  createInfo.enabledExtensionCount = extensionCount;
  createInfo.ppEnabledExtensionNames = extensions;
  createInfo.enabledLayerCount = layerCount;
  createInfo.ppEnabledLayerNames = layers;

  OPAL_ATTEMPT_VK(vkCreateInstance(&createInfo, NULL, &g_ovkState->instance));

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
  OPAL_ATTEMPT_VK(vkEnumeratePhysicalDevices(g_ovkState->instance, &deviceCount, NULL));
  VkPhysicalDevice* devices = OpalMemAllocArray(VkPhysicalDevice, deviceCount);
  OPAL_ATTEMPT_VK(vkEnumeratePhysicalDevices(g_ovkState->instance, &deviceCount, devices));

  for (uint32_t i = 0; i < deviceCount; i++)
  {
    if (IsDeviceSuitable_Ovk(devices[i], surface))
    {
      g_ovkState->gpu = InitGpuInfo_Ovk(devices[i], surface);

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
  info.pQueueFamilyProperties = OpalMemAllocArray(VkQueueFamilyProperties, info.queueFamilyPropertiesCount);
  vkGetPhysicalDeviceQueueFamilyProperties(gpu, &info.queueFamilyPropertiesCount, info.pQueueFamilyProperties);

  info.queueIndexGraphics = GetFamilyIndexForQueue_Ovk(&info, VK_QUEUE_GRAPHICS_BIT);
  info.queueIndexTransfer = GetFamilyIndexForQueue_Ovk(&info, VK_QUEUE_TRANSFER_BIT);
  info.queueIndexPresent = GetFamilyIndexForPresent_Ovk(&info, surface);

  return info;
}

void ShutdownGpuInfo_Ovk(OpalVulkanGpuInfo* info)
{
  OpalMemFree(info->pQueueFamilyProperties);
}

uint32_t GetFamilyIndexForQueue_Ovk(const OpalVulkanGpuInfo* const gpu, VkQueueFlags flags)
{
  uint32_t bestFit = ~0u;

  for (uint32_t i = 0; i < gpu->queueFamilyPropertiesCount; i++)
  {
    uint32_t queueFlag = gpu->pQueueFamilyProperties[i].queueFlags;

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
  queueIndices[0] = g_ovkState->gpu.queueIndexGraphics;
  queueIndices[1] = g_ovkState->gpu.queueIndexTransfer;
  queueIndices[2] = g_ovkState->gpu.queueIndexPresent;
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

  VkPhysicalDeviceFeatures enabledFeatures = {0};

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

  OPAL_ATTEMPT_VK(vkCreateDevice(g_ovkState->gpu.device, &createInfo, NULL, &g_ovkState->device));

  vkGetDeviceQueue(g_ovkState->device, queueIndices[0], 0, &g_ovkState->queueGraphics);
  vkGetDeviceQueue(g_ovkState->device, queueIndices[1], 0, &g_ovkState->queueTransfer);
  vkGetDeviceQueue(g_ovkState->device, queueIndices[2], 0, &g_ovkState->queuePresent);

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
  VkCommandPoolCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;

  VkCommandPool* outPool = NULL;

  if (isTransient)
  {
    createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    createInfo.queueFamilyIndex = g_ovkState->gpu.queueIndexTransfer;
    outPool = &g_ovkState->transientCommandPool;
  }
  else
  {
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = g_ovkState->gpu.queueIndexGraphics;
    outPool = &g_ovkState->graphicsCommandPool;
  }

  OPAL_ATTEMPT_VK(vkCreateCommandPool(g_ovkState->device, &createInfo, NULL, outPool));

  return Opal_Success;
}

OpalResult InitDescriptorPool_Ovk()
{
  // TODO : Find a good way to determine required resource counts

  VkDescriptorPoolSize sizes[2];
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

  OPAL_ATTEMPT_VK(
    vkCreateDescriptorPool(g_ovkState->device, &createInfo, NULL, &g_ovkState->descriptorPool));

  return Opal_Success;
}

OpalResult InitGlobalResources_Ovk(OpalInitInfo initInfo)
{
  // Vertex layout ==========

  OpalVertexLayoutInfo* vLayout = &initInfo.vertexLayout;

  uint32_t vertBackupCount = 3;
  OpalFormat vertBackupFormats[3] = { Opal_Format_RGB32, Opal_Format_RGB32, Opal_Format_RG32 };
  //if (vLayout->elementCount == 0 || vLayout->pElementFormats == NULL)
  //{
  //  OpalLog("Using backup vertex layout");
  //  vLayout->elementCount = vertBackupCount;
  //  vLayout->pElementFormats = vertBackupFormats;
  //}

  VkVertexInputAttributeDescription* pAttribs = OpalMemAllocArray(VkVertexInputAttributeDescription, vLayout->elementCount);

  uint32_t offsetSum = 0;
  for (uint32_t i = 0; i < vLayout->elementCount; i++)
  {
    pAttribs[i].binding = 0;
    pAttribs[i].location = i;
    pAttribs[i].offset = offsetSum;
    pAttribs[i].format = OpalFormatToVkFormat_Ovk(vLayout->pElementFormats[i]);
    offsetSum += OpalFormatToSize(vLayout->pElementFormats[i]);
  }

  VkVertexInputBindingDescription binding;
  binding.binding = 0;
  binding.stride = offsetSum;
  binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  g_ovkState->vertex.attribCount = vLayout->elementCount;
  g_ovkState->vertex.pAttribDescriptions = pAttribs;
  g_ovkState->vertex.bindingDescription = binding;

  // Buffer ==========

  OPAL_ATTEMPT(TransferBufferInit_Ovk());

  // Rendering ==========

  VkCommandBufferAllocateInfo allocInfo = { 0 };
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.pNext = NULL;
  allocInfo.commandBufferCount = 1;
  allocInfo.commandPool = g_ovkState->graphicsCommandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

  OPAL_ATTEMPT_VK(vkAllocateCommandBuffers(g_ovkState->device, &allocInfo, &g_ovkState->renderCmd));

  VkSemaphoreCreateInfo semInfo = { 0 };
  semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  semInfo.pNext = NULL;
  semInfo.flags = 0;

  OPAL_ATTEMPT_VK(vkCreateSemaphore(g_ovkState->device, &semInfo, NULL, &g_ovkState->renderCompleteSem));

  return Opal_Success;
}

// Shutdown
// ============================================================

void OpalVulkanShutdown()
{
  // Global resources
  TransferBufferShutdown_Ovk();

  // Core
  vkFreeCommandBuffers(g_ovkState->device, g_ovkState->graphicsCommandPool, 1, &g_ovkState->renderCmd);
  vkDestroySemaphore(g_ovkState->device, g_ovkState->renderCompleteSem, NULL);
  vkDestroyDescriptorPool(g_ovkState->device, g_ovkState->descriptorPool, NULL);
  vkDestroyCommandPool(g_ovkState->device, g_ovkState->graphicsCommandPool, NULL);
  vkDestroyCommandPool(g_ovkState->device, g_ovkState->transientCommandPool, NULL);
  vkDestroyDevice(g_ovkState->device, NULL);
  ShutdownGpuInfo_Ovk(&g_ovkState->gpu);
  vkDestroyInstance(g_ovkState->instance, NULL);
}

// Tools
// ============================================================

void OpalVulkanWaitIdle()
{
  vkDeviceWaitIdle(g_ovkState->device);
}
