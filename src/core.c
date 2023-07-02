
#include "src/common.h"
#include "src/vulkan/vulkan_common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Not my favorite, but needed because vulkan handles these types differently
uint32_t OpalFormatToSize(OpalFormat _format) // TODO : Find a better place to put this
{
  switch (_format)
  {
  case Opal_Format_Int8_1: return 1;
  case Opal_Format_Int8_2: return 2;
  case Opal_Format_Int8_3: return 3;
  case Opal_Format_Int8_4: return 4;
  case Opal_Format_Uint8_1: return 1;
  case Opal_Format_Uint8_2: return 2;
  case Opal_Format_Uint8_3: return 3;
  case Opal_Format_Uint8_4: return 4;
  case Opal_Format_Unorm8_1: return 1;
  case Opal_Format_Unorm8_2: return 2;
  case Opal_Format_Unorm8_3: return 3;
  case Opal_Format_Unorm8_4: return 4;
  case Opal_Format_Int32_1: return 4;
  case Opal_Format_Int32_2: return 8;
  case Opal_Format_Int32_3: return 12;
  case Opal_Format_Int32_4: return 16;
  case Opal_Format_Uint32_1: return 4;
  case Opal_Format_Uint32_2: return 8;
  case Opal_Format_Uint32_3: return 12;
  case Opal_Format_Uint32_4: return 16;
  case Opal_Format_Float32_1: return 4;
  case Opal_Format_Float32_2: return 8;
  case Opal_Format_Float32_3: return 12;
  case Opal_Format_Float32_4: return 16;
  case Opal_Format_Float64_1: return 8;
  case Opal_Format_Float64_2: return 16;
  case Opal_Format_Float64_3: return 24;
  case Opal_Format_Float64_4: return 32;
  case Opal_Format_Depth24_Stencil8: return 4;
  default: return 0;
  }
}

OpalResult OpalCreateState(OpalCreateStateInfo _createInfo,  OpalState* _outState)
{
  if (_outState == NULL)
  {
    OPAL_LOG_ERROR("Opal CreateState input error\n");
    return Opal_Unknown;
  }

  OpalState_T* newState = (OpalState_T*)LapisMemAllocZero(sizeof(OpalState_T));

  if (newState == NULL)
  {
    LapisMemFree(newState);
    OPAL_LOG_ERROR("Opal state memory not allocated\n");
    return Opal_Unknown;
  }

  DefineVertexLayout_Opal(newState, _createInfo);
  newState->objectShaderArgsInfo = *_createInfo.pCustomObjectShaderArgumentLayout;

  switch (_createInfo.api)
  {
  case Opal_Api_Vulkan:
  {
    // Define vulkan backend functions

    newState->backend.ShutdownState       = OpalVkShutdownState;
    newState->backend.RenderFrame         = OpalVkRenderFrame;
    newState->backend.GetSwapchainExtents = OpalVkGetSwapchainExtents;

    // Buffer =====
    newState->backend.CreateBuffer        = OpalVkCreateBuffer;
    newState->backend.DestroyBuffer       = OpalVkDestroyBuffer;
    newState->backend.BufferPushData      = OpalVkBufferPushData;

    // Image =====
    newState->backend.CreateImage         = OpalVkCreateImage;
    newState->backend.DestroyImage        = OpalVkDestroyImage;

    // Material =====
    newState->backend.CreateShader        = OpalVkCreateShader;
    newState->backend.DestroyShader       = OpalVkDestroyShader;

    newState->backend.CreateMaterial      = OpalVkCreateMaterial;
    newState->backend.DestroyMaterial     = OpalVkDestroyMaterial;

    newState->backend.RecreateMaterial    = OpalVkMaterialRecreate;

    // Rendering =====
    newState->backend.CreateObject    = OpalVkCreateObject;
    newState->backend.CreateRenderpass    = OpalVkCreateRenderpassAndFramebuffers;

    newState->backend.BindMaterial        = OpalVkBindMaterial;
    newState->backend.BindObject          = OpalVkBindObject;
    newState->backend.RenderMesh          = OpalVkRenderMesh;
    newState->backend.NextSubpass         = OpalVkNextSubpass;

    // Initialize

    OPAL_ATTEMPT(OpalVkInitState(_createInfo, newState),
    {
      OPAL_LOG_ERROR("Failed to initialize the Vulkan backend\n");
      return Opal_Failure_Backend;
    });
  } break; // Opal_Api_Vulkan

  default: break;
  }

  newState->api = _createInfo.api;

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
  OPAL_ATTEMPT(_state->backend.RenderFrame(_state, _frameData),
  {
    OPAL_LOG_ERROR("Failed to render the frame\n");
    return Opal_Failure_Backend;
  });

  return Opal_Success;
}

OpalExtents2D OpalGetSwapchainExtents(OpalState _state)
{
  return _state->backend.GetSwapchainExtents(_state);
}

OpalResult OpalCreateObject(OpalState _state, OpalShaderArg* _objectArguments, OpalObject* _renderable)
{
  *_renderable = (OpalObject_T*)LapisMemAllocZero(sizeof(OpalObject_T));

  OPAL_ATTEMPT(_state->backend.CreateObject(_state, _objectArguments, *_renderable),
  {
    LapisMemFree(*_renderable);
    return Opal_Failure_Backend;
  });

  return Opal_Success;
}
