
#include "vulkan/vulkan_common.h"

// Declarations
// ============================================================

// Core ==========
//OpalResult OpalVulkanFramebufferInit    (OpalFramebuffer* pFramebuffer, OpalFramebufferInitInfo initInfo)
//void       OpalVulkanFramebufferShutdown(OpalFramebuffer* pFramebuffer)

// Core
// ============================================================

OpalResult OpalVulkanFramebufferInit(OpalFramebuffer* pFramebuffer, OpalFramebufferInitInfo initInfo)
{
  VkImageView* attachments = OpalMemAllocArray(VkImageView, initInfo.imageCount);
  for (int i = 0; i < initInfo.imageCount; i++)
  {
    attachments[i] = initInfo.ppImages[i]->api.vk.view;
  }

  VkFramebufferCreateInfo createinfo = { 0 };
  createinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  createinfo.pNext = NULL;
  createinfo.flags = 0;
  createinfo.width = initInfo.ppImages[0]->width;
  createinfo.height = initInfo.ppImages[0]->height;
  createinfo.layers = 1;
  createinfo.renderPass = initInfo.renderpass.api.vk.renderpass;
  createinfo.attachmentCount = initInfo.imageCount;
  createinfo.pAttachments = attachments;

  OPAL_ATTEMPT_VK(vkCreateFramebuffer(g_ovkState->device, &createinfo, NULL, &pFramebuffer->api.vk.framebuffer));

  pFramebuffer->api.vk.pImagePointers = OpalMemAllocArray(OpalImage*, initInfo.imageCount);
  for (int i = 0; i < initInfo.imageCount; i++)
  {
    pFramebuffer->api.vk.pImagePointers[i] = initInfo.ppImages[i];
  }

  OpalMemFree(attachments);
  return Opal_Success;
}

void OpalVulkanFramebufferShutdown(OpalFramebuffer* pFramebuffer)
{
  vkDestroyFramebuffer(g_ovkState->device, pFramebuffer->api.vk.framebuffer, NULL);
  OpalMemFree(pFramebuffer->api.vk.pImagePointers);
}