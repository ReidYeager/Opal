
#ifndef GEM_OPAL_VULKAN_VULKAN_H
#define GEM_OPAL_VULKAN_VULKAN_H

#include "src/defines.h"
#include "src/vulkan/vulkan_defines.h"

#define OVK_ATTEMPT(fn, failureAction) \
{                                      \
  VkResult attemptResult = (fn);       \
  if (attemptResult != VK_SUCCESS)     \
  {                                    \
    OPAL_LOG_ERROR("Vk attempt failed : Result %d : \"%s\"\n\t\"%s\" : %u\n", attemptResult, #fn, __FILE__, __LINE__); \
    __debugbreak();                    \
    failureAction;                     \
  }                                    \
}

VkFormat OpalFormatToVkFormat(OpalFormat _inFormat);

OpalResult OvkInitState(OpalCreateStateInfo _createInfo, OpalState _oState);
void OvkShutdownState(OpalState _oState);
OpalResult OvkRenderFrame(OpalState _oState, const OpalFrameData* _oFrameData);

OpalResult OvkCreateDescriptorSetLayout(
  OvkState_T* _state,
  uint32_t _shaderArgCount,
  OpalShaderArgTypes* _pShaderArgs,
  VkDescriptorSetLayout* _outLayout);
OpalResult OvkCreateDescriptorSet(
  OvkState_T* _state,
  VkDescriptorSetLayout _layout,
  VkDescriptorSet* _outSet);

// Buffer =====
OpalResult OvkCreateBuffer(
  OpalState _oState,
  OpalCreateBufferInfo _createInfo,
  OpalBuffer _oBuffer);
void OvkDestroyBuffer(OpalState _oState, OpalBuffer _oBuffer);
OpalResult OvkBufferPushData(OpalState _oState, OpalBuffer _oBuffer, void* _data);

// Image =====
OpalResult OvkCreateImage(OpalState _state, OpalCreateImageInfo _createInfo, OpalImage _outImage);
void OvkDestroyImage(OpalState _state, OpalImage _image);

// Material =====
OpalResult OvkCreateShader(
  OpalState _oState,
  OpalCreateShaderInfo _createInfo,
  OpalShader _oShader);
void OvkDestroyShader(OpalState _oState, OpalShader _oShader);
OpalResult OvkCreateMaterial(
  OpalState _state,
  OpalCreateMaterialInfo _createInfo,
  OpalMaterial _Material);
void OvkDestroyMaterial(OpalState _oState, OpalMaterial _oMaterial);

// Mesh =====
OpalResult OvkCreateMesh(OpalState _state, OpalCreateMeshInfo _createInfo, OpalMesh _outMesh);
void OvkDestroyMesh(OpalState _state, OpalMesh _mesh);

// Renderable =====
OpalResult OvkCreateRenderable(
  OpalState _oState,
  OpalShaderArg* _objectArguments,
  OpalRenderable _renderable);

OpalResult UpdateShaderArguments(
  OvkState_T* _state,
  uint32_t _argCount,
  OpalShaderArg* args,
  VkDescriptorSet _descriptorSet);

// =====
// Vulkan internal
// =====

// Commands =====
OpalResult OvkBeginSingleUseCommand(OvkState_T* _state, VkCommandPool _pool, VkCommandBuffer* _cmd);
OpalResult OvkEndSingleUseCommand(OvkState_T* _state, VkCommandPool _pool, VkQueue _queue, VkCommandBuffer _cmd);
OpalResult OvkRecordCommandBuffer(OvkState_T* _state, OvkFrame_T* _frame, const OpalFrameData* _data);


#endif // !GEM_OPAL_VULKAN_VULKAN_H
