
#include "src/common.h"

typedef struct RenderpassData_Ovk
{
  uint32_t attachmentCount;
  VkAttachmentDescription* pDescriptions;
  VkAttachmentReference* pReferences;
  VkClearValue* pClearValues;

  uint32_t subpassCount;
  VkSubpassDescription* pSubpasses;

  uint32_t dependencyCount;
  VkSubpassDependency* pDependencies;
} RenderpassData_Ovk;

void FreeRenderpassData_Ovk(RenderpassData_Ovk* _data)
{
  if (_data->pDescriptions)
    OpalMemFree(_data->pDescriptions);

  if (_data->pReferences)
    OpalMemFree(_data->pReferences);

  for (uint32_t i = 0; _data->pSubpasses != NULL && i < _data->subpassCount; i++)
  {
    if (_data->pSubpasses[i].pColorAttachments)
      OpalMemFree((void*)_data->pSubpasses[i].pColorAttachments);

    if (_data->pSubpasses[i].pColorAttachments)
      OpalMemFree((void*)_data->pSubpasses[i].pInputAttachments);

    if (_data->pSubpasses[i].pColorAttachments)
      OpalMemFree((void*)_data->pSubpasses[i].pPreserveAttachments);

    if (_data->pSubpasses[i].pColorAttachments)
      OpalMemFree((void*)_data->pSubpasses[i].pResolveAttachments);
  }
  if (_data->pSubpasses)
    OpalMemFree(_data->pSubpasses);

  if (_data->pDependencies)
  OpalMemFree(_data->pDependencies);
}

OpalResult BuildAttachmentArray_Ovk(OpalRenderpassInitInfo _initInfo, RenderpassData_Ovk* _data)
{
  _data->pDescriptions = OpalMemAllocZeroArray(VkAttachmentDescription, _data->attachmentCount);
  _data->pReferences = OpalMemAllocZeroArray(VkAttachmentReference, _data->attachmentCount);
  _data->pClearValues = OpalMemAllocArray(VkClearValue, _data->attachmentCount);

  for (uint32_t i = 0; i < _initInfo.imageCount; i++)
  {
    _data->pClearValues[i] = *(VkClearValue*)(&_initInfo.pAttachments[i].clearValue);

    _data->pDescriptions[i].format = OpalFormatToVkFormat_Ovk(_initInfo.pAttachments[i].format);
    _data->pDescriptions[i].samples = VK_SAMPLE_COUNT_1_BIT;
    _data->pDescriptions[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _data->pDescriptions[i].finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _data->pDescriptions[i].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    _data->pDescriptions[i].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    _data->pDescriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    _data->pDescriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    _data->pReferences[i].attachment = i;

    switch (_initInfo.pAttachments[i].usage)
    {
      case Opal_Attachment_Usage_Color:
      {
        _data->pDescriptions[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        _data->pReferences[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      } break;
      case Opal_Attachment_Usage_Depth:
      {
        _data->pDescriptions[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        _data->pReferences[i].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      } break;
      case Opal_Attachment_Usage_Presented:
      {
        _data->pDescriptions[i].finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        _data->pReferences[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      } break;
      default:
      {
        FreeRenderpassData_Ovk(_data);
        OpalLogError("Unknown renderpass attachment usage : %u\n", _initInfo.pAttachments[i].usage);
        return Opal_Failure;
      }
    } // switch (_initInfo.pAttachments[i].usage)

    switch (_initInfo.pAttachments[i].loadOp)
    {
      case Opal_Attachment_Op_Clear:
      {
        _data->pDescriptions[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      } break;
      case Opal_Attachment_Op_Load:
      {
        _data->pDescriptions[i].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        _data->pDescriptions[i].initialLayout = _data->pReferences[i].layout;
      } break;
      case Opal_Attachment_Op_Dont_Care:
      {
        _data->pDescriptions[i].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      } break;
      default:
      {
        FreeRenderpassData_Ovk(_data);
        OpalLogError("Unknown renderpass attachment load op : %u\n", _initInfo.pAttachments[i].loadOp);
        return Opal_Failure;
      }
    } // switch (_initInfo.pAttachments[i].loadOp)

    if (_initInfo.pAttachments[i].shouldStore)
    {
      _data->pDescriptions[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    }

  }

  return Opal_Success;
}

OpalResult BuildSubpasses_Ovk(OpalRenderpassInitInfo _initInfo, RenderpassData_Ovk* _data)
{
  _data->subpassCount = _initInfo.subpassCount;
  _data->pSubpasses = OpalMemAllocZeroArray(VkSubpassDescription, _data->subpassCount);

  for (uint32_t i = 0; i < _initInfo.subpassCount; i++)
  {
    _data->pSubpasses[i].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    // Depth =====

    if (_initInfo.pSubpasses[i].depthAttachmentIndex != -1)
    {
      _data->pSubpasses[i].pDepthStencilAttachment = &_data->pReferences[_initInfo.pSubpasses[i].depthAttachmentIndex];
    }

    // Color =====

    VkAttachmentReference* pColorAttachments = OpalMemAllocZeroArray(VkAttachmentReference, _initInfo.pSubpasses[i].colorAttachmentCount);
    for (uint32_t c = 0; c < _initInfo.pSubpasses[i].colorAttachmentCount; c++)
    {
      pColorAttachments[c] = _data->pReferences[_initInfo.pSubpasses[i].pColorAttachmentIndices[c]];
    }
    _data->pSubpasses[i].colorAttachmentCount = _initInfo.pSubpasses[i].colorAttachmentCount;
    _data->pSubpasses[i].pColorAttachments = pColorAttachments;

    // Input =====

    VkAttachmentReference* pInputAttachments = OpalMemAllocZeroArray(VkAttachmentReference, _initInfo.pSubpasses[i].inputAttachmentCount);
    for (uint32_t c = 0; c < _initInfo.pSubpasses[i].inputAttachmentCount; c++)
    {
      pInputAttachments[c] = _data->pReferences[_initInfo.pSubpasses[i].pInputColorAttachmentIndices[c]];
    }
    _data->pSubpasses[i].colorAttachmentCount = _initInfo.pSubpasses[i].colorAttachmentCount;
    _data->pSubpasses[i].pInputAttachments = pInputAttachments;
  }

  // Dependencies =====

  VkSubpassDependency* pDependencies = OpalMemAllocZeroArray(VkSubpassDependency, _initInfo.dependencyCount);
  for (uint32_t i = 0; i < _initInfo.dependencyCount; i++)
  {
    pDependencies[i].srcSubpass = _initInfo.pDependencies[i].srcIndex;
    pDependencies[i].dstSubpass = _initInfo.pDependencies[i].dstIndex;
  }
  _data->dependencyCount = _initInfo.dependencyCount;
  _data->pDependencies = pDependencies;

  return Opal_Success;
}

OpalResult OvkRenderpassInit(OpalRenderpass_T* _renderpass, OpalRenderpassInitInfo _initInfo)
{
  RenderpassData_Ovk renderpassData = { 0 };
  renderpassData.attachmentCount = _initInfo.imageCount;

  OPAL_ATTEMPT(BuildAttachmentArray_Ovk(_initInfo, &renderpassData));
  OPAL_ATTEMPT(BuildSubpasses_Ovk(_initInfo, &renderpassData));

  VkRenderPassCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  createInfo.attachmentCount = renderpassData.attachmentCount;
  createInfo.pAttachments = renderpassData.pDescriptions;
  createInfo.subpassCount = renderpassData.subpassCount;
  createInfo.pSubpasses = renderpassData.pSubpasses;
  createInfo.dependencyCount = renderpassData.dependencyCount;
  createInfo.pDependencies = renderpassData.pDependencies;

  OVK_ATTEMPT(vkCreateRenderPass(oState.vk.device, &createInfo, oState.vk.allocator, &_renderpass->vk.renderpass));

  _renderpass->vk.pClearValues = renderpassData.pClearValues;

  FreeRenderpassData_Ovk(&renderpassData);

  return Opal_Success;
}

// Renderpass should rarely, if ever, need to re-init.
// Can shut it down and initialize a new one if needed.

void OvkRenderpassShutdown(OpalRenderpass_T* _renderpass)
{
  vkDestroyRenderPass(oState.vk.device, _renderpass->vk.renderpass, oState.vk.allocator);
}
