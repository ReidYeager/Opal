
#include "src/common.h"

OpalResult OvkFramebufferInit(OpalFramebuffer_T* _framebuffer, OpalFramebufferInitInfo _initInfo)
{
  VkFramebufferCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  createInfo.renderPass = _initInfo.renderpass->vk.renderpass;
  createInfo.attachmentCount = 1;
  createInfo.pAttachments = &oState.window.renderBufferImage->vk.view;
  createInfo.width = oState.window.extents.width;
  createInfo.height = oState.window.extents.height;
  createInfo.layers = 1;

  OVK_ATTEMPT(vkCreateFramebuffer(oState.vk.device, &createInfo, oState.vk.allocator, &_framebuffer->vk.framebuffer));

  return Opal_Success;
}

OpalResult OvkFramebufferReinit(OpalFramebuffer_T* _framebuffer)
{
  vkDestroyFramebuffer(oState.vk.device, _framebuffer->vk.framebuffer, oState.vk.allocator);

  OpalFramebufferInitInfo newInitInfo = { 0 };
  newInitInfo.renderpass = _framebuffer->ownerRenderpass;

  OPAL_ATTEMPT(OvkFramebufferInit(_framebuffer, newInitInfo));

  return Opal_Success;
}

void OvkFramebufferShutdown(OpalFramebuffer_T* _framebuffer)
{
  vkDestroyFramebuffer(oState.vk.device, _framebuffer->vk.framebuffer, oState.vk.allocator);
}

