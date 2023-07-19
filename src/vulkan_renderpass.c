
#include "src/define_renderpass.h"
#include "src/common.h"

OpalResult OvkRenderpassInit(OpalRenderpass_T* _renderpass)
{
  VkAttachmentDescription attachment = { 0 };
  attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  attachment.format = oState.window.vk.format;
  attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachment.samples = VK_SAMPLE_COUNT_1_BIT; // used for multi-sampling

  VkAttachmentReference reference = { 0 };
  reference.attachment = 0;
  reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = { 0 };
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.pDepthStencilAttachment = NULL;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &reference;
  subpass.inputAttachmentCount = 0;
  subpass.pInputAttachments = NULL;
  subpass.preserveAttachmentCount = 0;
  subpass.pPreserveAttachments = NULL;

  //VkSubpassDependency dependency = { 0 };

  VkRenderPassCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  createInfo.attachmentCount = 1;
  createInfo.pAttachments = &attachment;
  createInfo.subpassCount = 1;
  createInfo.pSubpasses = &subpass;
  createInfo.dependencyCount = 0;
  createInfo.pDependencies = NULL;

  OVK_ATTEMPT(vkCreateRenderPass(oState.vk.device, &createInfo, oState.vk.allocator, &_renderpass->vk.renderpass));

  return Opal_Success;
}

// Renderpass should rarely, if ever, need to re-init.
// Can shut it down and initialize a new one if needed.

void OvkRenderpassShutdown(OpalRenderpass_T* _renderpass)
{
  vkDestroyRenderPass(oState.vk.device, _renderpass->vk.renderpass, oState.vk.allocator);
}
