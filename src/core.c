
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
