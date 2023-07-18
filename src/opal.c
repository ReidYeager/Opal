
#include "src/common.h"

#include <vulkan/vulkan.h>

#include <stdio.h>

OpalState_T oState = { 0 };

OpalResult OpalInit(OpalInitInfo _initInfo)
{
  if (oState.window.lWindow)
  {
    OpalLog("Failed to init Opal. Already in use.\n");
    return Opal_Failure;
  }

  oState.window.lWindow = _initInfo.window;
  oState.vk.allocator = NULL;

  OPAL_ATTEMPT(OvkInit());

  OPAL_ATTEMPT(OvkWindowInit(&oState.window));

  OPAL_ATTEMPT(OvkWindowReinit(&oState.window));

  return Opal_Success;
}

void OpalShutdown()
{
  vkDeviceWaitIdle(oState.vk.device);

  OvkWindowShutdown(&oState.window);
  OvkShutdown();
}

