
#include "src/defines.h"
#include "src/vulkan/vulkan.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void PopulateVertexLayout(OpalState _state, OpalCreateStateInfo _createInfo)
{
  OpalVertexLayoutInfo* stateLayout = &_state->vertexLayout;
  OpalVertexLayoutInfo* inLayout = _createInfo.pCustomVertexLayout;

  if (inLayout == NULL)
  {
    stateLayout->elementCount = 3;
    stateLayout->pElementSizes = (uint32_t*)LapisMemAlloc(
      sizeof(uint32_t) * stateLayout->elementCount);

    stateLayout->pElementSizes[0] = sizeof(uint32_t) * 3; // Vec3 Position
    stateLayout->pElementSizes[1] = sizeof(uint32_t) * 3; // Vec3 Normal
    stateLayout->pElementSizes[2] = sizeof(uint32_t) * 2; // Vec2 Uv

    return;
  }

  stateLayout->structSize = inLayout->structSize;
  if (stateLayout->structSize == 0)
  {
    for (uint32_t i = 0; i < inLayout->elementCount; i++)
    {
      stateLayout->structSize += inLayout->pElementSizes[i];
    }
  }

  stateLayout->elementCount = inLayout->elementCount;
  stateLayout->pElementSizes = (uint32_t*)LapisMemAlloc(
    sizeof(uint32_t) * stateLayout->elementCount);
  LapisMemCopy(
    inLayout->pElementSizes,
    stateLayout->pElementSizes,
    sizeof(uint32_t) * stateLayout->elementCount);
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

  switch (_info.api)
  {
  case Opal_Api_Vulkan:
  {
    // Define vulkan backend functions
    newState->backend.ShutdownState = OvkShutdownState;
    newState->backend.RenderFrame = OvkRenderFrame;
    // Buffer =====
    newState->backend.CreateBuffer = OvkCreateBuffer;
    newState->backend.DestroyBuffer = OvkDestroyBuffer;
    newState->backend.BufferPushData = OvkBufferPushData;
    // Material =====
    newState->backend.CreateShader = OvkCreateShader;
    newState->backend.DestroyShader = OvkDestroyShader;
    newState->backend.CreateMaterial = OvkCreateMaterial;
    newState->backend.DestroyMaterial = OvkDestroyMaterial;


    OPAL_ATTEMPT(
      OvkInitState(_info, newState),
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
