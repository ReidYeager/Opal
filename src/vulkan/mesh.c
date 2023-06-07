
#include "src/vulkan/vulkan_common.h"

OpalResult OvkCreateMesh(OvkState_T* _state, uint32_t _objectArgCount, OpalShaderArg* _objectArguments, OpalMesh _mesh)
{
  OPAL_ATTEMPT(OpalVkCreateDescriptorSet(_state, _state->objectSetLayout, &_mesh->backend.vulkan.descriptorSet),
    return Opal_Failure_Vk_Create);

  OPAL_ATTEMPT(OvkUpdateShaderArguments(_state, _objectArgCount, _objectArguments, _mesh->backend.vulkan.descriptorSet),
    return Opal_Failure_Vk_Create);

  return Opal_Success;
}
