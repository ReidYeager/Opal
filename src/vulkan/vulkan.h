
#ifndef GEM_OPAL_VULKAN_VULKAN_H
#define GEM_OPAL_VULKAN_VULKAN_H

#include "src/defines.h"
#include "src/vulkan/vulkan_defines.h"

#define OVK_ATTEMPT(fn, failureAction)                           \
{                                                                \
  VkResult attemptResult = (fn);                                 \
  if (attemptResult != VK_SUCCESS)                               \
  {                                                              \
    OPAL_LOG_ERROR(                                              \
      "Vk attempt failed : Result %d : \"%s\"\n\t\"%s\" : %u\n", \
      attemptResult,                                             \
      #fn,                                                       \
      __FILE__,                                                  \
      __LINE__);                                                 \
    __debugbreak();                                              \
    failureAction;                                               \
  }                                                              \
}

VkFormat OpalFormatToVkFormat(OpalFormat _inFormat);

OpalExtents2D OpalVkGetSwapchainExtents(OpalState _oState);

OpalResult OpalVkInitState(OpalCreateStateInfo _createInfo, OpalState _oState);
void OpalVkShutdownState(OpalState _oState);
OpalResult OpalVkRenderFrame(OpalState _oState, const OpalFrameData* _oFrameData);

OpalResult OpalVkCreateDescriptorSetLayout(
  OvkState_T* _state,
  uint32_t _shaderArgCount,
  OpalShaderArgTypes* _pShaderArgs,
  VkDescriptorSetLayout* _outLayout);
OpalResult OpalVkCreateDescriptorSet(
  OvkState_T* _state,
  VkDescriptorSetLayout _layout,
  VkDescriptorSet* _outSet);

// Buffer =====

OpalResult OpalVkCreateBuffer(
  OpalState _oState,
  OpalCreateBufferInfo _createInfo,
  OpalBuffer _oBuffer);
void OpalVkDestroyBuffer(OpalState _oState, OpalBuffer _oBuffer);
OpalResult OpalVkBufferPushData(OpalState _oState, OpalBuffer _oBuffer, void* _data);

// Image =====

OpalResult OpalVkCreateImage(OpalState _state, OpalCreateImageInfo _createInfo, OpalImage _outImage);
void OpalVkDestroyImage(OpalState _state, OpalImage _image);

OpalResult OpalVkImageFillData(
  OpalState _oState,
  OvkImage_T* _image,
  OpalExtents2D _extents,
  OpalFormat _format,
  void* _data);

OpalResult OvkCreateImage(
  OvkState_T* _state,
  VkExtent2D _extents,
  VkFormat _format,
  VkImageUsageFlags _usage,
  VkImage* _outImage);
OpalResult OvkCreateImageMemory(OvkState_T* _state, VkImage _image, VkDeviceMemory* _outMemory);
OpalResult OvkCreateImageAndMemory(
  OvkState_T* _state,
  VkExtent2D _extents,
  VkFormat _format,
  VkImageUsageFlags _usage,
  VkImage* _outImage,
  VkDeviceMemory* _outMemory);
OpalResult OvkCreateImageView(
  OvkState_T* _state,
  VkImageAspectFlags _aspectMask,
  VkImage _image,
  VkFormat _format,
  VkImageView* _outView);
OpalResult OvkCreateImageSampler(OvkState_T* _state, VkSampler* _outSampler);

// Material =====

OpalResult OpalVkCreateShader(
  OpalState _oState,
  OpalCreateShaderInfo _createInfo,
  OpalShader _oShader);
void OpalVkDestroyShader(OpalState _oState, OpalShader _oShader);
OpalResult OpalVkCreateMaterial(
  OpalState _state,
  OpalCreateMaterialInfo _createInfo,
  OpalMaterial _Material);
void OpalVkDestroyMaterial(OpalState _oState, OpalMaterial _oMaterial);

// Renderable =====

OpalResult OpalVkCreateRenderable(
  OpalState _oState,
  OpalShaderArg* _objectArguments,
  OpalRenderable _renderable);

OpalResult OvkUpdateShaderArguments(
  OvkState_T* _state,
  uint32_t _argCount,
  OpalShaderArg* args,
  VkDescriptorSet _descriptorSet);

// Rendering =====

OpalResult OvkCreateRenderpass(
  OvkState_T* _state,
  OvkCreateRenderpassInfo _createInfo,
  VkRenderPass* _outRenderpass);

OpalResult OvkCreateFramebuffer(
  OvkState_T* _state,
  VkExtent2D _extents,
  VkRenderPass _renderpass,
  uint32_t _viewCount,
  VkImageView* _views,
  VkFramebuffer* _outFramebuffer);

OpalResult OpalVkCreateRenderpassAndFramebuffers(
  OpalState _oState,
  OpalCreateRenderpassInfo _createInfo,
  OpalRenderpass _outRenderpass);

void OpalVkBindMaterial(OpalState _oState, OpalMaterial _material);
void OpalVkBindRenderable(OpalState _oState, OpalRenderable _renderable);
void OpalVkRenderMesh(OpalMesh _mesh);
void OpalVkNextSubpass();

// Commands =====
OpalResult OvkBeginSingleUseCommand(OvkState_T* _state, VkCommandPool _pool, VkCommandBuffer* _cmd);
OpalResult OvkEndSingleUseCommand(OvkState_T* _state, VkCommandPool _pool, VkQueue _queue, VkCommandBuffer _cmd);
OpalResult OvkRecordCommandBuffer(OvkState_T* _state, OvkFrame_T* _frame, const OpalFrameData* _data);


#endif // !GEM_OPAL_VULKAN_VULKAN_H
