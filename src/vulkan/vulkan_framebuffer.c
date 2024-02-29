
#include "src/vulkan/vulkan_common.h"

OpalResult OpalVulkanFramebufferInit(OpalFramebuffer* pFramebuffer, OpalFramebufferInitInfo initInfo)
{
  VkImageView* attachments = OpalMemAllocArray(VkImageView, initInfo.imageCount);
  for (int i = 0; i < initInfo.imageCount; i++)
  {
    attachments[i] = initInfo.pImages[i].api.vk.view;
  }

  VkFramebufferCreateInfo createinfo;
  createinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  createinfo.flags = 0;
  createinfo.pNext = NULL;
  createinfo.width = initInfo.pImages[0].width;
  createinfo.height = initInfo.pImages[0].height;
  createinfo.layers = 1;
  createinfo.renderPass = initInfo.renderpass.api.vk.renderpass;
  createinfo.attachmentCount = initInfo.imageCount;
  createinfo.pAttachments = attachments;

  OPAL_ATTEMPT_VK(vkCreateFramebuffer(g_OpalState.api.vk.device, &createinfo, NULL, &pFramebuffer->api.vk.framebuffer));

  OpalMemFree(attachments);
  return Opal_Success;
}

void OpalVulkanFramebufferShutdown(OpalFramebuffer* pFramebuffer)
{
  
}