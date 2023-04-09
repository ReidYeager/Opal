
#include "src/defines.h"
#include "src/vulkan/vulkan.h"

#include <vulkan/vulkan.h>
#include <lapis.h>

OpalResult OvkCreateShader(OpalState _oState, OpalCreateShaderInfo _createInfo, OpalShader _oShader)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;

  VkShaderModuleCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.codeSize = _createInfo.sourceSize;
  createInfo.pCode = (uint32_t*)_createInfo.sourceCode;

  OVK_ATTEMPT(
    vkCreateShaderModule(state->device, &createInfo, NULL, &_oShader->backend.vulkan.module),
    return Opal_Failure_Vk_Create);

  return Opal_Success;
}

void OvkDestroyShader(OpalState _oState, OpalShader _oShader)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;
  vkDestroyShaderModule(state->device, _oShader->backend.vulkan.module, NULL);
}
