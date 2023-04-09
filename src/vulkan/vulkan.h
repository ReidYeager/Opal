
#ifndef GEM_OPAL_VULKAN_VULKAN_H
#define GEM_OPAL_VULKAN_VULKAN_H

#include "src/defines.h"
#include "src/vulkan/vulkan_defines.h"

#define OVK_ATTEMPT(fn, failureAction) \
{                                      \
  VkResult attemptResult = (fn);       \
  if (attemptResult != VK_SUCCESS)     \
  {                                    \
    failureAction;                     \
  }                                    \
}

OpalResult OvkInitState(OpalCreateStateInfo _createInfo, OpalState _oState);
void OvkShutdownState(OpalState _oState);

OpalResult OvkCreateShader(
  OpalState _oState,
  OpalCreateShaderInfo _createInfo,
  OpalShader _oShader);
void OvkDestroyShader(OpalState _oState, OpalShader _oShader);


#endif // !GEM_OPAL_VULKAN_VULKAN_H
