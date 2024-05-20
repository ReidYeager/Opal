
#include "vulkan/vulkan_common.h"

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

  info.queueIndexGraphicsCompute = ~0u;
  info.queueIndexTransfer        = ~0u;
  info.queueIndexPresent         = ~0u;

  info.queueIndexGraphicsCompute = GetFamilyIndexForQueue_Ovk(&info, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
  info.queueIndexTransfer = GetFamilyIndexForQueue_Ovk(&info, VK_QUEUE_TRANSFER_BIT);
  info.queueIndexPresent  = GetFamilyIndexForPresent_Ovk(&info, surface);

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
      bool graphicsComputeQueueUnique =
        flags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT) || i != gpu->queueIndexGraphicsCompute;
      bool transferQueueUnique = flags & VK_QUEUE_TRANSFER_BIT || i != gpu->queueIndexTransfer;

      // Try to avoid choosing overlapping queue family indices
      if (graphicsComputeQueueUnique && transferQueueUnique)
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
    OpalLogError("Failed to find a queue family with the flag %u", flags);
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
      bool queueUnique = i != gpu->queueIndexGraphicsCompute
        && i != gpu->queueIndexTransfer;

      if (queueUnique)
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
    OpalLogError("Failed to find a queue family for presentation");
  }

  return bestFit;
}

OpalResult InitDevice_Ovk(bool useDebug)
{
  // Queues ==============================

  uint32_t queueCount = 3;
  const float queuePriorities[2] = {
    1.0f,
    1.0f
  };
  uint32_t* queueIndices = OpalMemAllocArray(uint32_t, queueCount);
  queueIndices[0] = g_ovkState->gpu.queueIndexGraphicsCompute;
  queueIndices[1] = g_ovkState->gpu.queueIndexTransfer;
  queueIndices[2] = g_ovkState->gpu.queueIndexPresent;
  VkDeviceQueueCreateInfo* queueCreateInfos = OpalMemAllocArray(VkDeviceQueueCreateInfo, queueCount);

  for (uint32_t i = 0; i < queueCount; i++)
  {
    queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[i].pNext = NULL;
    queueCreateInfos[i].flags = 0;
    queueCreateInfos[i].queueCount = (i == 0) ? 2 : 1;
    queueCreateInfos[i].queueFamilyIndex = queueIndices[i];
    queueCreateInfos[i].pQueuePriorities = queuePriorities;
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

  vkGetDeviceQueue(g_ovkState->device, g_ovkState->gpu.queueIndexGraphicsCompute, 0, &g_ovkState->queueGraphics);
  vkGetDeviceQueue(g_ovkState->device, g_ovkState->gpu.queueIndexGraphicsCompute, 1, &g_ovkState->queueCompute);
  vkGetDeviceQueue(g_ovkState->device, g_ovkState->gpu.queueIndexTransfer,        0, &g_ovkState->queueTransfer);
  vkGetDeviceQueue(g_ovkState->device, g_ovkState->gpu.queueIndexPresent,         0, &g_ovkState->queuePresent);

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
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = g_ovkState->gpu.queueIndexTransfer;
    outPool = &g_ovkState->transientCommandPool;
  }
  else
  {
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = g_ovkState->gpu.queueIndexGraphicsCompute;
    outPool = &g_ovkState->graphicsCommandPool;
  }

  OPAL_ATTEMPT_VK(vkCreateCommandPool(g_ovkState->device, &createInfo, NULL, outPool));

  return Opal_Success;
}

OpalResult InitDescriptorPool_Ovk()
{
  // TODO : Find a good way to determine required resource counts

  VkDescriptorPoolSize sizes[4];
  sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  sizes[0].descriptorCount = 1024;
  sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  sizes[1].descriptorCount = 1024;
  sizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  sizes[2].descriptorCount = 1024;
  sizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  sizes[3].descriptorCount = 1024;

  VkDescriptorPoolCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  createInfo.maxSets = 1024;
  createInfo.poolSizeCount = 4;
  createInfo.pPoolSizes = sizes;

  OPAL_ATTEMPT_VK(
    vkCreateDescriptorPool(g_ovkState->device, &createInfo, NULL, &g_ovkState->descriptorPool));

  return Opal_Success;
}

OpalResult InitGlobalResources_Ovk(OpalInitInfo initInfo)
{
  // Vertex layout ==========

  VkVertexInputAttributeDescription* pAttribs =
    OpalMemAllocArray(VkVertexInputAttributeDescription, initInfo.vertexLayout.elementCount);

  uint32_t offsetSum = 0;
  for (uint32_t i = 0; i < initInfo.vertexLayout.elementCount; i++)
  {
    pAttribs[i].binding = 0;
    pAttribs[i].location = i;
    pAttribs[i].offset = offsetSum;
    pAttribs[i].format = OpalFormatToVkFormat_Ovk(initInfo.vertexLayout.pElementFormats[i]);
    offsetSum += OpalFormatToSize(initInfo.vertexLayout.pElementFormats[i]);
  }

  VkVertexInputBindingDescription binding;
  binding.binding = 0;
  binding.stride = offsetSum;
  binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  g_ovkState->vertex.attribCount = initInfo.vertexLayout.elementCount;
  g_ovkState->vertex.pAttribDescriptions = pAttribs;
  g_ovkState->vertex.bindingDescription = binding;

  // Buffer ==========

  OPAL_ATTEMPT(TransferBufferInit_Ovk());

  // Single-use command ==========

  g_ovkState->singleUse.count = 3;

  g_ovkState->singleUse.pGraphicsCmds = OpalMemAllocArray(VkCommandBuffer, g_ovkState->singleUse.count);
  g_ovkState->singleUse.pTransferCmds = OpalMemAllocArray(VkCommandBuffer, g_ovkState->singleUse.count);
  g_ovkState->singleUse.pGraphicsFences = OpalMemAllocArray(VkFence, g_ovkState->singleUse.count);
  g_ovkState->singleUse.pTransferFences = OpalMemAllocArray(VkFence, g_ovkState->singleUse.count);
  g_ovkState->singleUse.graphicsHead = 0;
  g_ovkState->singleUse.transferHead = 0;

  VkCommandBufferAllocateInfo cmdAllocInfo;
  cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmdAllocInfo.pNext = NULL;
  cmdAllocInfo.commandBufferCount = g_ovkState->singleUse.count;
  cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

  cmdAllocInfo.commandPool = g_ovkState->graphicsCommandPool;
  OPAL_ATTEMPT_VK(vkAllocateCommandBuffers(g_ovkState->device, &cmdAllocInfo, g_ovkState->singleUse.pGraphicsCmds));

  cmdAllocInfo.commandPool = g_ovkState->transientCommandPool;
  OPAL_ATTEMPT_VK(vkAllocateCommandBuffers(g_ovkState->device, &cmdAllocInfo, g_ovkState->singleUse.pTransferCmds));

  VkFenceCreateInfo fenceCreateInfo;
  fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceCreateInfo.pNext = NULL;
  fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (uint32_t i = 0; i < g_ovkState->singleUse.count; i++)
  {
    OPAL_ATTEMPT_VK(vkCreateFence(g_ovkState->device, &fenceCreateInfo, NULL, &g_ovkState->singleUse.pGraphicsFences[i]));
    OPAL_ATTEMPT_VK(vkCreateFence(g_ovkState->device, &fenceCreateInfo, NULL, &g_ovkState->singleUse.pTransferFences[i]));
  }

  // Rendering ==========

  g_ovkState->renderState.cmdCount = 32;

  VkCommandBufferAllocateInfo allocInfo = { 0 };
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.pNext = NULL;
  allocInfo.commandBufferCount = g_ovkState->renderState.cmdCount;
  allocInfo.commandPool = g_ovkState->graphicsCommandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  g_ovkState->renderState.pCmdBuffers = OpalMemAllocArray(VkCommandBuffer, g_ovkState->renderState.cmdCount);

  OPAL_ATTEMPT_VK(vkAllocateCommandBuffers(g_ovkState->device, &allocInfo, g_ovkState->renderState.pCmdBuffers));

  VkFenceCreateInfo fenceInfo = { 0 };
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.pNext = NULL;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  g_ovkState->renderState.pCmdAvailableFence = OpalMemAllocArray(VkFence, g_ovkState->renderState.cmdCount);

  for (uint32_t i = 0; i < g_ovkState->renderState.cmdCount; i++)
  {
    OPAL_ATTEMPT_VK(vkCreateFence(g_ovkState->device, &fenceInfo, NULL, &g_ovkState->renderState.pCmdAvailableFence[i]));
  }

  g_ovkState->renderState.curIndex = 0;

  return Opal_Success;
}

// Shutdown
// ============================================================

void OpalVulkanShutdown()
{
  // Global resources
  TransferBufferShutdown_Ovk();

  for (uint32_t i = 0; i < g_ovkState->singleUse.count; i++)
  {
    vkDestroyFence(g_ovkState->device, g_ovkState->singleUse.pGraphicsFences[i], NULL);
    vkDestroyFence(g_ovkState->device, g_ovkState->singleUse.pTransferFences[i], NULL);
  }
  vkFreeCommandBuffers(g_ovkState->device, g_ovkState->graphicsCommandPool, g_ovkState->singleUse.count, g_ovkState->singleUse.pGraphicsCmds);
  vkFreeCommandBuffers(g_ovkState->device, g_ovkState->transientCommandPool, g_ovkState->singleUse.count, g_ovkState->singleUse.pTransferCmds);
  OpalMemFree(g_ovkState->singleUse.pGraphicsCmds);
  OpalMemFree(g_ovkState->singleUse.pTransferCmds);
  OpalMemFree(g_ovkState->singleUse.pGraphicsFences);
  OpalMemFree(g_ovkState->singleUse.pTransferFences);

  // Core
  for (int i = 0; i < g_ovkState->renderState.cmdCount; i++)
  {
    vkDestroyFence(g_ovkState->device, g_ovkState->renderState.pCmdAvailableFence[i], NULL);
  }
  vkFreeCommandBuffers(g_ovkState->device, g_ovkState->graphicsCommandPool, g_ovkState->renderState.cmdCount, g_ovkState->renderState.pCmdBuffers);
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
