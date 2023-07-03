
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
  case Opal_Format_Int8_1: return VK_FORMAT_R8_SINT;
  case Opal_Format_Int8_2: return VK_FORMAT_R8G8_SINT;
  case Opal_Format_Int8_3: return VK_FORMAT_R8G8B8_SINT;
  case Opal_Format_Int8_4: return VK_FORMAT_R8G8B8A8_SINT;
  case Opal_Format_Uint8_1: return VK_FORMAT_R8_UINT;
  case Opal_Format_Uint8_2: return VK_FORMAT_R8G8_UINT;
  case Opal_Format_Uint8_3: return VK_FORMAT_R8G8B8_UINT;
  case Opal_Format_Uint8_4: return VK_FORMAT_R8G8B8A8_UINT;
  case Opal_Format_Unorm8_1: return VK_FORMAT_R8_UNORM;
  case Opal_Format_Unorm8_2: return VK_FORMAT_R8G8_UNORM;
  case Opal_Format_Unorm8_3: return VK_FORMAT_R8G8B8_UNORM;
  case Opal_Format_Unorm8_4: return VK_FORMAT_R8G8B8A8_UNORM;
  case Opal_Format_Int32_1: return VK_FORMAT_R32_SINT;
  case Opal_Format_Int32_2: return VK_FORMAT_R32G32_SINT;
  case Opal_Format_Int32_3: return VK_FORMAT_R32G32B32_SINT;
  case Opal_Format_Int32_4: return VK_FORMAT_R32G32B32A32_SINT;
  case Opal_Format_Uint32_1: return VK_FORMAT_R32_UINT;
  case Opal_Format_Uint32_2: return VK_FORMAT_R32G32_UINT;
  case Opal_Format_Uint32_3: return VK_FORMAT_R32G32B32_UINT;
  case Opal_Format_Uint32_4: return VK_FORMAT_R32G32B32A32_UINT;
  case Opal_Format_Float32_1: return VK_FORMAT_R32_SFLOAT;
  case Opal_Format_Float32_2: return VK_FORMAT_R32G32_SFLOAT;
  case Opal_Format_Float32_3: return VK_FORMAT_R32G32B32_SFLOAT;
  case Opal_Format_Float32_4: return VK_FORMAT_R32G32B32A32_SFLOAT;
  case Opal_Format_Float64_1: return VK_FORMAT_R64_SFLOAT;
  case Opal_Format_Float64_2: return VK_FORMAT_R64G64_SFLOAT;
  case Opal_Format_Float64_3: return VK_FORMAT_R64G64B64_SFLOAT;
  case Opal_Format_Float64_4: return VK_FORMAT_R64G64B64A64_SFLOAT;
  case Opal_Format_Depth24_Stencil8 : return VK_FORMAT_D24_UNORM_S8_UINT;
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
  gpu.queueIndexPresent = GetFamilyIndexForPresent(&gpu, _state->window->surface);

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

OpalResult OpalVkInitState(OpalCreateStateInfo _createInfo, OpalState _oState)
{
  OvkState_T* state = (OvkState_T*)LapisMemAllocZero(sizeof(OvkState_T));
  state->window = &_oState->window.backend.vulkan;
  _oState->backend.state = (void*)state;

  if (_createInfo.window == NULL)
  {
    return Opal_Failure;
    //state->isHeadless = 1;
  }
  _oState->window.lapisWindow = _createInfo.window;

  OPAL_ATTEMPT(CreateInstance(state),
  {
    LapisMemFree(state);
    OVK_LOG_ERROR("Failed to create vulkan instance\n");
    return Opal_Failure_Vk_Init;
  });

  OPAL_ATTEMPT(InitWindowPartialA_OpalVk(_oState, _createInfo.window, state->window),
    return Opal_Failure);

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
  OpalShaderArgumentTypes* pArguments = NULL;
  if (_createInfo.pCustomObjectShaderArgumentLayout != NULL)
  {
    count = _createInfo.pCustomObjectShaderArgumentLayout->argumentCount;
    pArguments = _createInfo.pCustomObjectShaderArgumentLayout->pArguments;
  }
  OPAL_ATTEMPT(OpalVkCreateDescriptorSetLayout(state, count, pArguments, &state->objectSetLayout),
  {
    LapisMemFree(state);
    OVK_LOG_ERROR("Failed to create object descriptor set layout\n");
    return Opal_Failure_Vk_Init;
  });

  OPAL_ATTEMPT(InitWindowPartialB_OpalVk(_oState, _createInfo.window, state->window),
    return Opal_Failure);

  OVK_LOG(Lapis_Console_Info, "Init complete\n");

  return Opal_Success;
}

void OpalVkShutdownState(OpalState _oState)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;

  vkDeviceWaitIdle(state->device);

  OpalVkShutdownWindow(_oState, state->window);

  vkDestroyDescriptorPool(state->device, state->descriptorPool, NULL);
  vkDestroyCommandPool(state->device, state->transientCommantPool, NULL);
  vkDestroyCommandPool(state->device, state->graphicsCommandPool, NULL);
  vkDestroyDevice(state->device, NULL);
  vkDestroyInstance(state->instance, NULL);

  LapisMemFree(state->gpu.queueFamilyProperties);

  OVK_LOG(Lapis_Console_Info, "Shutdown complete\n");

  LapisMemFree(state);
  _oState->backend.state = NULL;
}

OpalResult OpalVkRenderFrame(OpalState _oState, const OpalFrameData* _oFrameData)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;
  uint32_t slotIndex = state->window->currentFrame;
  OvkFrame_T* frameSlot = &state->window->pFrames[slotIndex];

  // Setup =====

  OVK_ATTEMPT(vkWaitForFences(state->device, 1, &frameSlot->fenceFrameAvailable, VK_TRUE, UINT64_MAX),
    return Opal_Failure_Vk_Render);

  OVK_ATTEMPT(
    vkAcquireNextImageKHR(
      state->device,
      state->window->swapchain.swapchain,
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
  presentInfo.pSwapchains = &state->window->swapchain.swapchain;
  presentInfo.pImageIndices = &frameSlot->swapchainImageIndex;

  OVK_ATTEMPT(vkQueuePresentKHR(state->queuePresent, &presentInfo),
    return Opal_Failure_Vk_Render);

  state->window->currentFrame = (state->window->currentFrame + 1) % maxFlightSlotCount;

  return Opal_Success;
}

OpalExtents2D OpalVkGetSwapchainExtents(OpalState _oState)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;
  OpalExtents2D out = { state->window->extents.width, state->window->extents.height };
  return out;
}
