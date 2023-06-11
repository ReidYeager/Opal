
#include "src/common.h"

bool CompareExtents(OpalExtents2D _a, OpalExtents2D _b)
{
  return (_a.width == _b.width) && (_a.height == _b.height);
}

OpalResult OpalCreateRenderpass(OpalState _state, OpalCreateRenderpassInfo _createInfo, OpalRenderpass* _outRenderpass)
{
  if (_createInfo.imageCount == 0 || _createInfo.subpassCount == 0)
  {
    return Opal_Failure;
  }

  bool usesSwapchain = _createInfo.rendersToSwapchain != 0;
  uint32_t totalImageCount = _createInfo.imageCount + usesSwapchain;

  OpalRenderpass newRenderpass = (OpalRenderpass)LapisMemAllocZero(sizeof(OpalRenderpass_T));
  newRenderpass->clearValues = LapisMemAllocZeroArray(OpalClearValue, totalImageCount);

  OpalExtents2D matchExtents = _createInfo.images[0]->extents;
  if (usesSwapchain)
  {
    matchExtents = OpalGetSwapchainExtents(_state);
    newRenderpass->clearValues[totalImageCount - 1].color = (OpalClearColor){ 0.4f, 0.2f, 0.6f, 0.0f };
  }

  for (uint32_t i = 0; i < _createInfo.imageCount; i++)
  {
    if (!CompareExtents(_createInfo.images[i]->extents, matchExtents))
    {
      LapisMemFree(newRenderpass->clearValues);
      LapisMemFree(newRenderpass);
      OPAL_LOG_ERROR("Failed to create renderpass. All images must have the same extents\n");
      return Opal_Failure;
    }

    newRenderpass->clearValues[i] = _createInfo.imageAttachments[i].clearValues;
  }

  // Fill subpass info
  newRenderpass->subpassCount = _createInfo.subpassCount;
  newRenderpass->subpasses = LapisMemAllocZeroArray(OpalSubpass_T, _createInfo.subpassCount);
  for (uint32_t i = 0; i < newRenderpass->subpassCount; i++)
  {
    newRenderpass->subpasses[i].colorAttachmentCount = _createInfo.subpasses[i].colorAttachmentCount;
    if (_createInfo.rendersToSwapchain && i == newRenderpass->subpassCount - 1)
    {
      // Add color attachment for the swapchain
      newRenderpass->subpasses[i].colorAttachmentCount++;
    }

    newRenderpass->subpasses[i].usesDepth = _createInfo.subpasses[i].depthAttachmentIndex != OPAL_SUBPASS_NO_DEPTH;
  }

  newRenderpass->attachmentCount = totalImageCount;
  newRenderpass->extents = matchExtents;
  newRenderpass->Render = _createInfo.RenderFunction;

  OPAL_ATTEMPT(_state->backend.CreateRenderpass(_state, _createInfo, newRenderpass),
  {
    LapisMemFree(newRenderpass->clearValues);
    LapisMemFree(newRenderpass);
    return Opal_Failure_Backend;
  });

  *_outRenderpass = newRenderpass;

  return Opal_Success;
}

void OpalBindMaterial(OpalState _state, OpalMaterial _material)
{
  _state->backend.BindMaterial(_state, _material);
}

void OpalBindObject(OpalState _state, OpalObject _object)
{
  _state->backend.BindObject(_state, _object);
}

void OpalRenderMesh(OpalState _state, OpalMesh _mesh)
{
  _state->backend.RenderMesh(_mesh);
}

void OpalNextSubpass(OpalState _state)
{
  _state->backend.NextSubpass();
}
