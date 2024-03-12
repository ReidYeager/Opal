
#include "src/vulkan/vulkan_common.h"

// Local types
// ============================================================

typedef struct DependencyPair_Ovk
{
  int producer, consumer;
} DependencyPair_Ovk;

typedef struct RenderpassData_Ovk
{
  uint32_t attachmentCount;
  VkAttachmentDescription* pAttachments;
  VkClearValue* pClearValues;

  VkAttachmentReference* pReferences;

  uint32_t subpassCount;
  VkSubpassDescription* pSubpasses;
  uint32_t* pSubpassColorOutputCount;

  uint32_t dependencyCount;
  VkSubpassDependency* pDependencies;
} RenderpassData_Ovk;

// Declarations
// ============================================================

// Core ==========
//OpalResult OpalVulkanRenderpassInit    (OpalRenderpass* pRenderpass, OpalRenderpassInitInfo initInfo)
//void       OpalVulkanRenderpassShutdown(OpalRenderpass* pRenderpass)
OpalResult   BuildAttachments_Ovk        (OpalRenderpassInitInfo initInfo, RenderpassData_Ovk* data);
OpalResult   BuildSubpasses_Ovk          (OpalRenderpassInitInfo initInfo, RenderpassData_Ovk* data);
OpalResult   BuildDependencies_Ovk       (OpalRenderpassInitInfo initInfo, RenderpassData_Ovk* data);

// Tools ==========
void DestroyRenderpassData_Ovk(RenderpassData_Ovk* data);

// Core
// ============================================================

OpalResult OpalVulkanRenderpassInit(OpalRenderpass* pRenderpass, OpalRenderpassInitInfo initInfo)
{
  RenderpassData_Ovk data;

  OPAL_ATTEMPT(BuildAttachments_Ovk (initInfo, &data), DestroyRenderpassData_Ovk(&data));
  OPAL_ATTEMPT(BuildSubpasses_Ovk   (initInfo, &data), DestroyRenderpassData_Ovk(&data));
  OPAL_ATTEMPT(BuildDependencies_Ovk(initInfo, &data), DestroyRenderpassData_Ovk(&data));

  VkRenderPassCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.attachmentCount = data.attachmentCount;
  createInfo.pAttachments    = data.pAttachments;
  createInfo.dependencyCount = data.dependencyCount;
  createInfo.pDependencies   = data.pDependencies;
  createInfo.subpassCount    = data.subpassCount;
  createInfo.pSubpasses      = data.pSubpasses;

  OPAL_ATTEMPT_VK(
    vkCreateRenderPass(g_ovkState->device, &createInfo, NULL, &pRenderpass->api.vk.renderpass),
    DestroyRenderpassData_Ovk(&data));

  pRenderpass->api.vk.pClearValues = OpalMemAllocArray(VkClearValue, initInfo.attachmentCount);
  pRenderpass->api.vk.pFinalLayouts = OpalMemAllocArray(VkImageLayout, initInfo.attachmentCount);
  for (int i = 0; i < initInfo.attachmentCount; i++)
  {
    pRenderpass->api.vk.pClearValues[i] = *(VkClearValue*)(&initInfo.pAttachments[i].clearValue);
    pRenderpass->api.vk.pFinalLayouts[i] = data.pAttachments[i].finalLayout;
  }

  pRenderpass->api.vk.subpassColorOutputCount = OpalMemAllocArray(uint32_t, initInfo.subpassCount);
  for (int i = 0; i < initInfo.subpassCount; i++)
  {
    pRenderpass->api.vk.subpassColorOutputCount[i] = data.pSubpassColorOutputCount[i];
  }

  pRenderpass->attachmentCount = initInfo.attachmentCount;
  pRenderpass->subpassCount = initInfo.subpassCount;

  DestroyRenderpassData_Ovk(&data);
  return Opal_Success;
}

void OpalVulkanRenderpassShutdown(OpalRenderpass* pRenderpass)
{
  vkDestroyRenderPass(g_ovkState->device, pRenderpass->api.vk.renderpass, NULL);
  OpalMemFree(pRenderpass->api.vk.pClearValues);
  OpalMemFree(pRenderpass->api.vk.pFinalLayouts);
  OpalMemFree(pRenderpass->api.vk.subpassColorOutputCount);
}

OpalResult BuildAttachments_Ovk(OpalRenderpassInitInfo initInfo, RenderpassData_Ovk* data)
{
  data->attachmentCount = initInfo.attachmentCount;
  data->pAttachments = OpalMemAllocArrayZeroed(VkAttachmentDescription, initInfo.attachmentCount);
  data->pClearValues = OpalMemAllocArrayZeroed(VkClearValue, initInfo.attachmentCount);
  data->pReferences  = OpalMemAllocArrayZeroed(VkAttachmentReference, initInfo.attachmentCount * initInfo.subpassCount);
  data->pSubpassColorOutputCount = OpalMemAllocArrayZeroed(uint32_t, initInfo.subpassCount);

  for (int attachIndex = 0; attachIndex < initInfo.attachmentCount; attachIndex++)
  {
    data->pClearValues[attachIndex] = *(VkClearValue*)(&initInfo.pAttachments[attachIndex].clearValue);

    data->pAttachments[attachIndex].format         = OpalFormatToVkFormat_Ovk(initInfo.pAttachments[attachIndex].format);
    data->pAttachments[attachIndex].samples        = VK_SAMPLE_COUNT_1_BIT;
    data->pAttachments[attachIndex].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    data->pAttachments[attachIndex].finalLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
    data->pAttachments[attachIndex].loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    data->pAttachments[attachIndex].storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    data->pAttachments[attachIndex].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    data->pAttachments[attachIndex].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    // Reference layout ==========

    for (int subIndex = 0; subIndex < initInfo.subpassCount; subIndex++)
    {
      int refIndex = subIndex * initInfo.attachmentCount + attachIndex;

      data->pReferences[refIndex].attachment = attachIndex;

      OpalAttachmentUsage use = initInfo.pAttachments[attachIndex].pSubpassUsages[subIndex];
      if (use == Opal_Attachment_Usage_Input)
      {
        data->pReferences[refIndex].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      }
      else
      {
        if (initInfo.pAttachments[attachIndex].format == Opal_Format_D32
          || initInfo.pAttachments[attachIndex].format == Opal_Format_D24_S8)
        {
          data->pReferences[refIndex].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
        else
        {
          data->pSubpassColorOutputCount[subIndex] += use == Opal_Attachment_Usage_Output
            | use == Opal_Attachment_Usage_Output_Uniform
            | use == Opal_Attachment_Usage_Output_Presented;

          data->pReferences[refIndex].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
      }
    }

    // Final layout ==========

    switch (initInfo.pAttachments[attachIndex].pSubpassUsages[initInfo.subpassCount - 1])
    {
    case Opal_Attachment_Usage_Output_Uniform:
    case Opal_Attachment_Usage_Input:
    {
      data->pAttachments[attachIndex].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    } break;
    case Opal_Attachment_Usage_Output_Presented:
    {
      data->pAttachments[attachIndex].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    } break;
    default:
    {
      int finalSubpassOffset = (initInfo.subpassCount - 1) * initInfo.attachmentCount;
      data->pAttachments[attachIndex].finalLayout = data->pReferences[finalSubpassOffset + attachIndex].layout;
    } break;
    }

    // Load op ==========

    switch (initInfo.pAttachments[attachIndex].loadOp)
    {
    case Opal_Attachment_Load_Op_Clear:
    {
      data->pAttachments[attachIndex].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    } break;
    case Opal_Attachment_Load_Op_Load:
    {
      data->pAttachments[attachIndex].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      data->pAttachments[attachIndex].initialLayout = data->pAttachments[attachIndex].finalLayout;
    } break;
    case Opal_Attachment_Load_Op_Dont_Care:
    {
      data->pAttachments[attachIndex].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    } break;
    default:
    {
      OpalLogError("Unknown renderpass attachment load op : %u", initInfo.pAttachments[attachIndex].loadOp);
      return Opal_Failure_Invalid_Input;
    }
    }

    // Store ==========

    if (initInfo.pAttachments[attachIndex].shouldStore)
    {
      data->pAttachments[attachIndex].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    }
    else
    {
      data->pAttachments[attachIndex].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }
  }

  return Opal_Success;
}

OpalResult BuildSubpasses_Ovk(OpalRenderpassInitInfo initInfo, RenderpassData_Ovk* data)
{
  data->subpassCount = initInfo.subpassCount;
  data->pSubpasses = OpalMemAllocArrayZeroed(VkSubpassDescription, initInfo.subpassCount);

  int colorCount = 0;
  int inputCount = 0;
  int preserveCount = 0;

  for (int subIndex = 0; subIndex < initInfo.subpassCount; subIndex++)
  {
    VkSubpassDescription* sub = &data->pSubpasses[subIndex];
    sub->pDepthStencilAttachment = NULL;
    colorCount = 0;
    inputCount = 0;
    preserveCount = 0;
    VkAttachmentReference* colorRefs = OpalMemAllocArrayZeroed(VkAttachmentReference, initInfo.attachmentCount);
    VkAttachmentReference* inputRefs = OpalMemAllocArrayZeroed(VkAttachmentReference, initInfo.attachmentCount);
    uint32_t* preserveRefs = OpalMemAllocArrayZeroed(uint32_t, initInfo.attachmentCount);

    sub->pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    for (int attachIndex = 0; attachIndex < initInfo.attachmentCount; attachIndex++)
    {
      int refIndex = subIndex * initInfo.attachmentCount + attachIndex;

      const OpalAttachmentInfo* att = &initInfo.pAttachments[attachIndex];

      switch (att->pSubpassUsages[subIndex])
      {
      case Opal_Attachment_Usage_Preserved:
      {
        preserveRefs[preserveCount] = attachIndex;
        preserveCount++;
      } break;
      case Opal_Attachment_Usage_Input:
      {
        inputRefs[inputCount] = data->pReferences[refIndex];
        inputCount++;
      } break;
      case Opal_Attachment_Usage_Output:
      case Opal_Attachment_Usage_Output_Uniform:
      case Opal_Attachment_Usage_Output_Presented:
      {
        if (initInfo.pAttachments[attachIndex].format == Opal_Format_D32
          || initInfo.pAttachments[attachIndex].format == Opal_Format_D24_S8)
        {
          // Depth image
          if (sub->pDepthStencilAttachment != NULL)
          {
            OpalLogError("Renderpass may not have more than one depth attachment");

            OpalMemFree(colorRefs);
            OpalMemFree(inputRefs);
            OpalMemFree(preserveRefs);

            return Opal_Failure_Invalid_Input;
          }

          sub->pDepthStencilAttachment = &data->pReferences[refIndex];
        }
        else
        {
          // Color image
          colorRefs[colorCount] = data->pReferences[refIndex];
          colorCount++;
        }
      } break;
      case Opal_Attachment_Usage_Unused: break;
      default:
      {
        OpalLogError("Invalid attachment usage '%d' for attachment '%d'", att->pSubpassUsages[subIndex], attachIndex);
        return Opal_Failure_Invalid_Input;
      }
      }
    }

    sub->colorAttachmentCount = colorCount;
    sub->pColorAttachments = colorRefs;
    sub->inputAttachmentCount = inputCount;
    sub->pInputAttachments = inputRefs;
    sub->preserveAttachmentCount = preserveCount;
    sub->pPreserveAttachments = preserveRefs;
  }

  return Opal_Success;
}

OpalResult BuildDependencies_Ovk(OpalRenderpassInitInfo initInfo, RenderpassData_Ovk* data)
{
  int count = 0;
  data->pDependencies = OpalMemAllocArrayZeroed(VkSubpassDependency,  initInfo.subpassCount * initInfo.subpassCount);

  int prevOut = -1;
  for (int attachIndex = 0; attachIndex < initInfo.attachmentCount; attachIndex++)
  {
    const OpalAttachmentInfo* at = &initInfo.pAttachments[attachIndex];

    for (int subIndex = 0; subIndex < initInfo.subpassCount; subIndex++)
    {
      if (at->pSubpassUsages[subIndex] == Opal_Attachment_Usage_Output
        || at->pSubpassUsages[subIndex] == Opal_Attachment_Usage_Output_Uniform
        || at->pSubpassUsages[subIndex] == Opal_Attachment_Usage_Output_Presented)
      {
        prevOut = subIndex;
      }

      if (prevOut >= 0 && at->pSubpassUsages[subIndex] == Opal_Attachment_Usage_Input)
      {
        int exists = 0;
        for (int i = 0; i < count; i++)
        {
          exists |= data->pDependencies[i].srcSubpass == prevOut
            && data->pDependencies[i].dstSubpass == subIndex;
        }

        if (!exists)
        {
          data->pDependencies[count].srcSubpass = prevOut;
          data->pDependencies[count].dstSubpass = subIndex;
          data->pDependencies[count].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
          data->pDependencies[count].dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
          count++;
          prevOut = -1;
        }
      }
    }
  }

  data->dependencyCount = count;
  return Opal_Success;
}

// Tools
// ============================================================

void DestroyRenderpassData_Ovk(RenderpassData_Ovk* data)
{
  if (data->pAttachments)
    OpalMemFree(data->pAttachments);

  if (data->pReferences)
    OpalMemFree(data->pReferences);

  if (data->pClearValues)
    OpalMemFree(data->pClearValues);

  for (uint32_t i = 0; data->pSubpasses != NULL && i < data->subpassCount; i++)
  {
    if (data->pSubpasses[i].pColorAttachments)
      OpalMemFree((void*)data->pSubpasses[i].pColorAttachments);

    if (data->pSubpasses[i].pInputAttachments)
      OpalMemFree((void*)data->pSubpasses[i].pInputAttachments);

    if (data->pSubpasses[i].pPreserveAttachments)
      OpalMemFree((void*)data->pSubpasses[i].pPreserveAttachments);

    if (data->pSubpasses[i].pResolveAttachments)
      OpalMemFree((void*)data->pSubpasses[i].pResolveAttachments);
  }
  if (data->pSubpasses)
    OpalMemFree(data->pSubpasses);

  if (data->pDependencies)
    OpalMemFree(data->pDependencies);

  if (data->pSubpassColorOutputCount)
    OpalMemFree(data->pSubpassColorOutputCount);
}
