
#include "src/defines.h"
#include "src/vulkan/vulkan.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

  switch (_info.api)
  {
  case Opal_Api_Vulkan:
  {
    // Define vulkan backend functions
    newState->backend.ShutdownState = OvkShutdownState;

    OPAL_ATTEMPT(OvkInitState(_info, newState), return Opal_Failure_Backend);
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
