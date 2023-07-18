
#include "src/common.h"

#include <vulkan/vulkan.h>

#include <stdio.h>

OpalState_T oState = { 0 };

OpalResult OpalInit(OpalInitInfo _initInfo)
{
  if (oState.window.lWindow)
    return Opal_Failure;

  oState.window.lWindow = _initInfo.window;
  oState.vk.allocator = NULL;

  OPAL_ATTEMPT(OvkInit());
  OvkShutdown();

  return Opal_Success;
}

