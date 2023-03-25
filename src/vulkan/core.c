
#include "src/defines.h"
#include "src/vulkan/vulkan.h"

#include <vulkan/vulkan.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

OpalResult CreateInstance(OvkState_T* _state)
{
  VkApplicationInfo appInfo = {0};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pNext = NULL;
  appInfo.apiVersion = VK_API_VERSION_1_3;
  appInfo.pEngineName = "Gem:Opal";
  appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
  appInfo.pApplicationName = "Opal application name";
  appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);

  uint32_t extensionCount = 0;
  LapisWindowVulkanGetRequiredExtensions(&extensionCount, NULL);
  extensionCount += 1;

  const char** extensions = (char**)LapisMemAllocZero(sizeof(char*) * extensionCount);
  LapisWindowVulkanGetRequiredExtensions(NULL, extensions);
  extensions[1] = VK_KHR_SURFACE_EXTENSION_NAME;

  VkInstanceCreateInfo createInfo = {0};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.enabledExtensionCount = extensionCount;
  createInfo.ppEnabledExtensionNames = extensions;
  createInfo.enabledLayerCount = 0;
  createInfo.ppEnabledLayerNames = NULL;
  createInfo.pApplicationInfo = &appInfo;

  OVK_ATTEMPT(vkCreateInstance(&createInfo, NULL, &_state->instance), return Opal_Vk_Init_Failure);

  LapisMemFree(extensions);
  return Opal_Success;
}

OpalResult CreateSurface(LapisWindow _window, OvkState_T* _state)
{
  LapisResult result = LapisWindowVulkanCreateSurface(_window, _state->instance, &_state->surface);
  if (result != Lapis_Success)
  {
    return Opal_Vk_Init_Failure;
  }

  return Opal_Success;
}

int32_t RatePhysicalDeviceSuitability(VkPhysicalDevice _device)
{
  VkPhysicalDeviceProperties props;
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceMemoryProperties memProps;
  vkGetPhysicalDeviceProperties(_device, &props);
  vkGetPhysicalDeviceFeatures(_device, &features);
  vkGetPhysicalDeviceMemoryProperties(_device, &memProps);

  int score = 0;

  score += 100 * (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);

  OPAL_LOG_VK(Lapis_Console_Debug, "\"%s\" final score = %d\n", props.deviceName, score);
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

  _state->physicalDevice = devices[winningIndex];
  return Opal_Success;
}

OpalResult CreateDevice(OvkState_T* _state)
{
  VkPhysicalDeviceFeatures enabledFeatures = {0};

  VkDeviceCreateInfo createInfo = {0};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.pEnabledFeatures = &enabledFeatures;
  createInfo.enabledExtensionCount = 0;
  createInfo.ppEnabledExtensionNames = NULL;
  createInfo.enabledLayerCount = 0;
  createInfo.ppEnabledLayerNames = NULL;
  createInfo.queueCreateInfoCount = 0;
  createInfo.pQueueCreateInfos = NULL;

  OVK_ATTEMPT(
    vkCreateDevice(_state->physicalDevice, &createInfo, NULL, &_state->device),
    return Opal_Vk_Init_Failure);

  return Opal_Success;
}

OpalResult OvkInitState(OpalCreateStateInfo _createInfo, OpalState _state)
{
  OvkState_T* vkState = (OvkState_T*)LapisMemAllocZero(sizeof(OvkState_T));

  OPAL_ATTEMPT(
    CreateInstance(vkState),
    {
      OPAL_LOG_VK_ERROR("Failed to create vulkan instance\n");
      return Opal_Vk_Init_Failure;
    });

  OPAL_ATTEMPT(
    CreateSurface(_createInfo.window, vkState),
    {
      OPAL_LOG_VK_ERROR("Failed to create vulkan surface\n");
      return Opal_Vk_Init_Failure;
    });

  OPAL_ATTEMPT(
    SelectPhysicalDevice(vkState),
    {
      OPAL_LOG_VK_ERROR("Failed to select a physical device\n");
      return Opal_Vk_Init_Failure;
    });

  OPAL_ATTEMPT(
    CreateDevice(vkState),
    {
      OPAL_LOG_VK_ERROR("Failed to create vkDevice\n");
      return Opal_Vk_Init_Failure;
    });

  OPAL_LOG_VK(Lapis_Console_Info, "Init complete\n");

  _state->backend.state = (void*)vkState;
  return Opal_Success;
}
