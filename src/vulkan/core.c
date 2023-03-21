
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

  VkInstanceCreateInfo createInfo = {0};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.enabledExtensionCount = 0;
  createInfo.ppEnabledExtensionNames = NULL;
  createInfo.enabledLayerCount = 0;
  createInfo.ppEnabledLayerNames = NULL;
  createInfo.pApplicationInfo = &appInfo;

  OVK_ATTEMPT(vkCreateInstance(&createInfo, NULL, &_state->instance), return Opal_Vk_Init_Failure);

  return Opal_Success;
}

OpalResult OvkInitState(OpalCreateStateInfo _createInfo, OpalState _state)
{
  OvkState_T* vkState = (OvkState_T*)LapisMemAllocZero(sizeof(OvkState_T));

  OPAL_ATTEMPT(
    CreateInstance(vkState),
    {
      LapisConsolePrintMessage(Lapis_Console_Error, "Opal :: Failed to create vulkan instance\n");
      return Opal_Vk_Init_Failure;
    });

  // Create surface
  // Select physical device
  // Create device

  _state->backend.state = (void*)vkState;
  return Opal_Success;
}
