
#include "src/defines.h"
#include "src/vulkan/vulkan.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Not my favorite, but needed because vulkan handles these types differently
uint32_t OpalFormatToSize(OpalFormat _format) // TODO : Find a better place to put this
{
  switch (_format)
  {
  case Opal_Format_8_Bit_Int_1: return 1;
  case Opal_Format_8_Bit_Int_2: return 2;
  case Opal_Format_8_Bit_Int_3: return 3;
  case Opal_Format_8_Bit_Int_4: return 4;
  case Opal_Format_8_Bit_Uint_1: return 1;
  case Opal_Format_8_Bit_Uint_2: return 2;
  case Opal_Format_8_Bit_Uint_3: return 3;
  case Opal_Format_8_Bit_Uint_4: return 4;
  case Opal_Format_8_Bit_Unorm_1: return 1;
  case Opal_Format_8_Bit_Unorm_2: return 2;
  case Opal_Format_8_Bit_Unorm_3: return 3;
  case Opal_Format_8_Bit_Unorm_4: return 4;
  case Opal_Format_32_Bit_Int_1: return 4;
  case Opal_Format_32_Bit_Int_2: return 8;
  case Opal_Format_32_Bit_Int_3: return 12;
  case Opal_Format_32_Bit_Int_4: return 16;
  case Opal_Format_32_Bit_Uint_1: return 4;
  case Opal_Format_32_Bit_Uint_2: return 8;
  case Opal_Format_32_Bit_Uint_3: return 12;
  case Opal_Format_32_Bit_Uint_4: return 16;
  case Opal_Format_32_Bit_Float_1: return 4;
  case Opal_Format_32_Bit_Float_2: return 8;
  case Opal_Format_32_Bit_Float_3: return 12;
  case Opal_Format_32_Bit_Float_4: return 16;
  case Opal_Format_24_Bit_Depth_8_Bit_Stencil: return 4;
  default: return 0;
  }
}

void PopulateVertexLayout(OpalState _state, OpalCreateStateInfo _createInfo)
{
  OpalVertexLayoutInfo* stateLayout = &_state->vertexLayout;
  OpalVertexLayoutInfo* inLayout = _createInfo.pCustomVertexLayout;

  if (inLayout == NULL)
  {
    stateLayout->elementCount = 3;
    stateLayout->pElementFormats = (OpalFormat*)LapisMemAlloc(
      sizeof(OpalFormat) * stateLayout->elementCount);

    stateLayout->pElementFormats[0] = Opal_Format_32_Bit_Float_3; // Vec3 Position
    stateLayout->pElementFormats[1] = Opal_Format_32_Bit_Float_3; // Vec3 Normal
    stateLayout->pElementFormats[2] = Opal_Format_32_Bit_Float_2; // Vec2 Uv

    stateLayout->structSize = (4 * 3) * 2 + (4 * 2);

    return;
  }

  stateLayout->structSize = inLayout->structSize;
  if (stateLayout->structSize == 0)
  {
    for (uint32_t i = 0; i < inLayout->elementCount; i++)
    {
      stateLayout->structSize += OpalFormatToSize(inLayout->pElementFormats[i]);
    }
  }

  stateLayout->elementCount = inLayout->elementCount;
  stateLayout->pElementFormats = (OpalFormat*)LapisMemAlloc(
    sizeof(OpalFormat) * stateLayout->elementCount);
  LapisMemCopy(
    inLayout->pElementFormats,
    stateLayout->pElementFormats,
    sizeof(OpalFormat) * stateLayout->elementCount);
}

OpalResult OpalCreateState(OpalCreateStateInfo _info,  OpalState* _outState)
{
  if (_outState == NULL)
  {
    printf("Opal CreateState input error\n");
    return Opal_Unknown;
  }

  OpalState_T* newState = (OpalState_T*)LapisMemAllocZero(sizeof(OpalState_T));

  if (newState == NULL)
  {
    printf("Opal state memory not allocated\n");
    return Opal_Unknown;
  }

  PopulateVertexLayout(newState, _info);
  newState->objectShaderArgsInfo = *_info.pCustomObjectShaderArgumentLayout;

  switch (_info.api)
  {
  case Opal_Api_Vulkan:
  {
    // Define vulkan backend functions
    newState->backend.ShutdownState = OpalVkShutdownState;
    newState->backend.RenderFrame = OpalVkRenderFrame;
    newState->backend.GetSwapchainExtents = OpalVkGetSwapchainExtents;
    // Buffer =====
    newState->backend.CreateBuffer = OpalVkCreateBuffer;
    newState->backend.DestroyBuffer = OpalVkDestroyBuffer;
    newState->backend.BufferPushData = OpalVkBufferPushData;
    // Image =====
    newState->backend.CreateImage = OpalVkCreateImage;
    newState->backend.DestroyImage = OpalVkDestroyImage;
    // Material =====
    newState->backend.CreateShader = OpalVkCreateShader;
    newState->backend.DestroyShader = OpalVkDestroyShader;
    newState->backend.CreateMaterial = OpalVkCreateMaterial;
    newState->backend.DestroyMaterial = OpalVkDestroyMaterial;
    // Renderable =====
    newState->backend.CreateRenderable = OpalVkCreateRenderable;
    newState->backend.CreateRenderpass = OpalVkCreateRenderpassAndFramebuffers;


    OPAL_ATTEMPT(
      OpalVkInitState(_info, newState),
      {
        //OPAL_LOG_ERROR("Failed to initialize the Vulkan backend\n");
        return Opal_Failure_Backend;
      });
  } break;

  default: break;
  }
  newState->api = _info.api;

  *_outState = newState;
  return Opal_Success;
}

void OpalDestroyState(OpalState* _state)
{
  OpalState_T* state = *_state;

  state->backend.ShutdownState(state);

  LapisMemFree(state);
  *_state = NULL;
}

OpalResult OpalRenderFrame(OpalState _state, const OpalFrameData* _frameData)
{
  OPAL_ATTEMPT(
    _state->backend.RenderFrame(_state, _frameData),
    {
      OPAL_LOG_ERROR("Failed to render the frame\n");
      return Opal_Failure_Backend;
    });

  return Opal_Success;
}

OpalResult OpalCreateRenderable(
  OpalState _state,
  OpalMesh _mesh,
  OpalMaterial _material,
  OpalShaderArg* _objectArguments,
  OpalRenderable* _renderable)
{
  *_renderable = (OpalRenderable)LapisMemAllocZero(sizeof(OpalRenderable_T));

  (*_renderable)->mesh = _mesh;
  (*_renderable)->material = _material;

  OPAL_ATTEMPT(
    _state->backend.CreateRenderable(_state, _objectArguments, *_renderable),
    return Opal_Failure_Backend);
  return Opal_Success;
}

OpalResult OpalCreateRenderpass(
  OpalState _state,
  OpalCreateRenderpassInfo _createInfo,
  OpalRenderpass* _outRenderpass)
{
  if (_createInfo.imageCount == 0 || _createInfo.subpassCount == 0)
  {
    OPAL_LOG_ERROR("Failed to create renderpass. At least one image and subpass are rquired\n");
    return Opal_Failure;
  }

  OpalRenderpass newRenderpass = (OpalRenderpass)LapisMemAllocZero(sizeof(OpalRenderpass_T));
  OpalExtents2D matchExtents = _createInfo.images[0]->extents;

  newRenderpass->clearValues =
    (OpalRenderpassAttachmentClearValues*)LapisMemAllocZero(
      sizeof(OpalRenderpassAttachmentClearValues)
      * (_createInfo.imageCount + (_createInfo.rendersToSwapchain != 0)));

  for (uint32_t i = 0; i < _createInfo.imageCount; i++)
  {
    if (_createInfo.images[i]->extents.width != matchExtents.width
      || _createInfo.images[i]->extents.height != matchExtents.height)
    {
      LapisMemFree(newRenderpass->clearValues);
      LapisMemFree(newRenderpass);
      OPAL_LOG_ERROR("Failed to create renderpass. All images must have the same extents\n");
      return Opal_Failure;
    }

    newRenderpass->clearValues[i] = _createInfo.imageAttachments[i].clearValues;
  }
  if (_createInfo.rendersToSwapchain)
  {
    // TODO : Check if the swapchain extents match input
    newRenderpass->clearValues[_createInfo.imageCount].color =
      (OpalClearColor){ 0.4f, 0.2f, 0.6f, 0.0f };
  }

  OPAL_ATTEMPT(
    _state->backend.CreateRenderpass(_state, _createInfo, newRenderpass),
    {
      LapisMemFree(newRenderpass);
      return Opal_Failure_Backend;
    });

  newRenderpass->attachmentCount = _createInfo.imageCount + (_createInfo.rendersToSwapchain != 0);
  newRenderpass->extents = _createInfo.images[0]->extents;
  newRenderpass->Render = _createInfo.RenderFunction;

  *_outRenderpass = newRenderpass;

  return Opal_Success;
}

OpalExtents2D OpalGetSwapchainExtents(OpalState _state)
{
  return _state->backend.GetSwapchainExtents(_state);
}
