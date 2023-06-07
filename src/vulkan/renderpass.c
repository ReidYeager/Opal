
#include "src/defines.h"
#include "src/vulkan/vulkan.h"

#include <vulkan/vulkan.h>

OpalResult ConstructVkAttachments(
  OvkState_T* _state,
  OvkCreateRenderpassInfo _createInfo,
  VkAttachmentDescription** _descriptions,
  VkAttachmentReference** _references)
{
  VkAttachmentDescription* descriptions = LapisMemAllocZeroShort(VkAttachmentDescription, _createInfo.attachmentCount);
  VkAttachmentReference* references = LapisMemAllocZeroShort(VkAttachmentReference, _createInfo.attachmentCount);

  uint32_t depthAttachmentIndex = OPAL_SUBPASS_NO_DEPTH;

  for (int i = 0; i < _createInfo.attachmentCount; i++)
  {
    OvkRenderpassAttachment inAttachment = _createInfo.attachments[i];

    references[i].attachment = i;

    descriptions[i].flags = 0;
    descriptions[i].format = inAttachment.dataFormat;
    descriptions[i].samples = VK_SAMPLE_COUNT_1_BIT;
    descriptions[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    descriptions[i].finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    descriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    descriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    switch (inAttachment.usage)
    {
    case Ovk_Attachment_Usage_Color:
    {
      references[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      descriptions[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    } break;
    case Ovk_Attachment_Usage_Depth:
    {
      depthAttachmentIndex = i;
      references[i].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      descriptions[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    } break;
    case Ovk_Attachment_Usage_Presented:
    {
      references[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      descriptions[i].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    } break;

    default:
    {
      OPAL_LOG_VK_ERROR("Unknown renderpass attachment usage : %u\n", inAttachment.usage);
      return Opal_Failure;
    }
    }

    switch (inAttachment.loadOperation)
    {
    case Ovk_Attachment_LoadOp_Load:
    {
      descriptions[i].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

      if (inAttachment.usage == Ovk_Attachment_Usage_Presented)
        descriptions[i].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
      else
        descriptions[i].initialLayout = references[i].layout;
    } break;
    case Ovk_Attachment_LoadOp_Clear:
    {
      descriptions[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    } break;
    case Ovk_Attachment_LoadOp_Dont_Care:
    {
      descriptions[i].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    } break;
    default:
    {
      OPAL_LOG_VK_ERROR("Unknown renderpass attachment load operation : %u\n", inAttachment.loadOperation);
      return Opal_Failure;
    }
    }

    if (inAttachment.shouldStoreReneredData)
    {
      descriptions[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    }
    else
    {
      descriptions[i].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }
  }

  *_references = references;
  *_descriptions = descriptions;

  return Opal_Success;
}

OpalResult ConstructVkSubpasses(
  OvkState_T* _state,
  OvkCreateRenderpassInfo _createInfo,
  VkAttachmentReference* _references,
  VkSubpassDescription** _subpasses)
{
  *_subpasses = LapisMemAllocZeroShort(VkSubpassDescription, _createInfo.subpassCount);

  for (uint32_t subpassIndex = 0; subpassIndex < _createInfo.subpassCount; subpassIndex++)
  {
    OpalRenderpassSubpass inSub = _createInfo.subpasses[subpassIndex];

    VkAttachmentReference* colorAttachments = LapisMemAllocZeroShort(VkAttachmentReference, inSub.colorAttachmentCount);
    for (uint32_t i = 0; i < inSub.colorAttachmentCount; i++)
    {
      colorAttachments[i] = _references[inSub.pColorAttachmentIndices[i]];
    }

    VkAttachmentReference* inputAttachments = LapisMemAllocZeroShort(VkAttachmentReference, inSub.inputAttachmentCount);
    for (uint32_t i = 0; i < inSub.inputAttachmentCount; i++)
    {
      inputAttachments[i] = _references[inSub.pInputAttachmentIndices[i]];
    }

    VkSubpassDescription sub = { 0 };
    sub.flags = 0;
    sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    if (_createInfo.subpasses[subpassIndex].depthAttachmentIndex != OPAL_SUBPASS_NO_DEPTH)
    {
      sub.pDepthStencilAttachment = &_references[_createInfo.subpasses[subpassIndex].depthAttachmentIndex];
    }
    sub.colorAttachmentCount = inSub.colorAttachmentCount;
    sub.pColorAttachments = colorAttachments;
    sub.inputAttachmentCount = inSub.inputAttachmentCount;
    sub.pInputAttachments = inputAttachments;
    sub.preserveAttachmentCount = inSub.preserveAttachmentCount;
    sub.pPreserveAttachments = inSub.pPreserveAttachmentIndices;
    sub.pResolveAttachments = NULL;

    (*_subpasses)[subpassIndex] = sub;
  }

  return Opal_Success;
}

OpalResult ConstructVkDependencies(
  OvkState_T* _state,
  OvkCreateRenderpassInfo _createInfo,
  uint32_t* _outDepCount,
  VkSubpassDependency** _outDeps)
{
  *_outDepCount = _createInfo.dependencyCount;
  VkSubpassDependency* newDeps = LapisMemAllocZeroShort(VkSubpassDependency, *_outDepCount);

  for (uint32_t i = 0; i < _createInfo.dependencyCount; i++)
  {
    newDeps[i].srcSubpass = _createInfo.pDependencies[i].srcSubpassIndex;
    newDeps[i].dstSubpass = _createInfo.pDependencies[i].dstSubpassIndex;
    newDeps[i].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    newDeps[i].dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    newDeps[i].srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    newDeps[i].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    newDeps[i].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
  }

  *_outDeps = newDeps;

  return Opal_Success;
}

// TODO : Allow renderpass to be headless
OpalResult OvkCreateRenderpass(
  OvkState_T* _state,
  OvkCreateRenderpassInfo _createInfo,
  VkRenderPass* _outRenderpass)
{
  VkAttachmentDescription* attachments;
  VkAttachmentReference* attachmentRefs;
  VkSubpassDescription* subpasses;
  uint32_t depCount = 0;
  VkSubpassDependency* deps;

  OPAL_ATTEMPT(ConstructVkAttachments(_state, _createInfo, &attachments, &attachmentRefs), return Opal_Failure_Vk_Create);
  OPAL_ATTEMPT(ConstructVkSubpasses(_state, _createInfo, attachmentRefs, &subpasses), return Opal_Failure_Vk_Create);
  OPAL_ATTEMPT(ConstructVkDependencies(_state, _createInfo, &depCount, &deps), return Opal_Failure_Vk_Create);

  VkRenderPassCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.attachmentCount = _createInfo.attachmentCount;
  createInfo.pAttachments = attachments;
  createInfo.subpassCount = _createInfo.subpassCount;
  createInfo.pSubpasses = subpasses;
  createInfo.dependencyCount = depCount;
  createInfo.pDependencies = deps;

  OVK_ATTEMPT(
    vkCreateRenderPass(_state->device, &createInfo, NULL, _outRenderpass),
    return Opal_Failure_Vk_Create);

  for (uint32_t i = 0; i < _createInfo.subpassCount; i++)
  {
    LapisMemFree(subpasses[i].pColorAttachments);
    LapisMemFree(subpasses[i].pInputAttachments);
  }
  LapisMemFree(subpasses);
  LapisMemFree(attachmentRefs);
  LapisMemFree(attachments);
  LapisMemFree(deps);

  return Opal_Success;
}

OpalResult OpalVkCreateRenderpassAndFramebuffers(
  OpalState _oState,
  OpalCreateRenderpassInfo _createInfo,
  OpalRenderpass _outRenderpass)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;
  OvkRenderpass_T* outOvkRp = &_outRenderpass->backend.vulkan;

  uint32_t fbCount = _createInfo.rendersToSwapchain ? state->swapchain.imageCount : 1;
  uint32_t attachmentCount = _createInfo.imageCount + (_createInfo.rendersToSwapchain ? 1 : 0);
  VkImageView* views = (VkImageView*)LapisMemAllocZero(sizeof(VkImageView) * attachmentCount);

  // Renderpass =====

  OvkCreateRenderpassInfo rpInfo = { 0 };
  rpInfo.attachmentCount = attachmentCount;
  rpInfo.attachments = (OvkRenderpassAttachment*)LapisMemAllocZero(sizeof(OvkRenderpassAttachment) * attachmentCount);
  rpInfo.subpassCount = _createInfo.subpassCount;
  rpInfo.subpasses = _createInfo.subpasses;
  rpInfo.dependencyCount = _createInfo.dependencyCount;
  rpInfo.pDependencies = _createInfo.pDependencies;

  for (uint32_t i = 0; i < _createInfo.imageCount; i++)
  {
    rpInfo.attachments[i].dataFormat = _createInfo.images[i]->backend.vulkan.format;
    rpInfo.attachments[i].shouldStoreReneredData = _createInfo.imageAttachments[i].shouldStoreReneredData;
    rpInfo.attachments[i].loadOperation = (OvkRenderpassAttachmentLoadOp)_createInfo.imageAttachments[i].loadOperation;
    rpInfo.attachments[i].usage = (OvkRenderpassAttachmentUsage)_createInfo.imageAttachments[i].usage;

    views[i] = _createInfo.images[i]->backend.vulkan.view;
  }
  if (_createInfo.rendersToSwapchain)
  {
    rpInfo.attachments[attachmentCount - 1].dataFormat = state->swapchain.format.format;
    rpInfo.attachments[attachmentCount - 1].shouldStoreReneredData = 1;
    rpInfo.attachments[attachmentCount - 1].loadOperation = Ovk_Attachment_LoadOp_Clear;
    rpInfo.attachments[attachmentCount - 1].usage = Ovk_Attachment_Usage_Presented;

    OpalRenderpassSubpass finalSub = _createInfo.subpasses[_createInfo.subpassCount - 1];

    uint32_t colorCount = finalSub.colorAttachmentCount;
    uint32_t* colorIndices = (uint32_t*)LapisMemAlloc(sizeof(uint32_t) * (colorCount + 1));

    for (uint32_t i = 0; i < colorCount; i++)
    {
      colorIndices[i] = finalSub.pColorAttachmentIndices[i];
    }
    colorIndices[colorCount] = _createInfo.imageCount;
    finalSub.pColorAttachmentIndices = colorIndices;
    finalSub.colorAttachmentCount++;

    _createInfo.subpasses[_createInfo.subpassCount - 1] = finalSub;
  }

  OvkCreateRenderpass(state, rpInfo, &outOvkRp->renderpass);

  // Framebuffers =====

  outOvkRp->framebufferCount = fbCount;
  outOvkRp->framebuffers = (VkFramebuffer*)LapisMemAlloc(sizeof(VkFramebuffer) * fbCount);

  for (uint32_t i = 0; i < fbCount; i++)
  {
    views[_createInfo.imageCount] = state->swapchain.imageViews[i];
    OvkCreateFramebuffer(
      state,
      (VkExtent2D){_createInfo.images[0]->extents.width, _createInfo.images[0]->extents.height},
      outOvkRp->renderpass,
      attachmentCount,
      views,
      &outOvkRp->framebuffers[i]);
  }

  return Opal_Success;
}
