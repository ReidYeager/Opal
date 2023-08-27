
#include "src/common.h"

OpalResult OvkFramebufferInit(OpalFramebuffer_T* _framebuffer, OpalFramebufferInitInfo _initInfo)
{
  OpalExtent extent = _initInfo.pImages[0]->extents;

  VkImageView* pViews = LapisMemAllocArray(VkImageView, _initInfo.imageCount);
  for (uint32_t i = 0; i < _initInfo.imageCount; i++)
  {
    pViews[i] = _initInfo.pImages[i]->vk.view;
  }

  VkFramebufferCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  createInfo.renderPass = _initInfo.renderpass->vk.renderpass;
  createInfo.attachmentCount = _initInfo.imageCount;
  createInfo.pAttachments = pViews;
  createInfo.width = extent.width;
  createInfo.height = extent.height;
  createInfo.layers = 1;

  OVK_ATTEMPT(
    vkCreateFramebuffer(oState.vk.device, &createInfo, oState.vk.allocator, &_framebuffer->vk.framebuffer),
    LapisMemFree(pViews));

  return Opal_Success;
}

OpalResult OvkFramebufferReinit(OpalFramebuffer_T* _framebuffer)
{
  vkDestroyFramebuffer(oState.vk.device, _framebuffer->vk.framebuffer, oState.vk.allocator);

  OpalFramebufferInitInfo newInitInfo = { 0 };
  newInitInfo.renderpass = _framebuffer->ownerRenderpass;
  newInitInfo.imageCount = _framebuffer->imageCount;
  newInitInfo.pImages = _framebuffer->ppImages;

  OPAL_ATTEMPT(OvkFramebufferInit(_framebuffer, newInitInfo));

  return Opal_Success;
}

void OvkFramebufferShutdown(OpalFramebuffer_T* _framebuffer)
{
  vkDestroyFramebuffer(oState.vk.device, _framebuffer->vk.framebuffer, oState.vk.allocator);
}

