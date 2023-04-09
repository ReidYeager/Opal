
#ifndef GEM_OPAL_H
#define GEM_OPAL_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#include "include/opal_defines.h"

OpalResult OpalCreateState(OpalCreateStateInfo _info, OpalState* _outState);
void OpalDestroyState(OpalState* _state);
OpalResult OpalRenderFrame(OpalState _state);

// TODO : Opal create/destroy window (If not headless)

OpalResult OpalCreateShaders(
  OpalState _state,
  uint32_t _createCount,
  OpalCreateShaderInfo* _pCreateInfos,
  OpalShader* _pOutShaders);
void OpalDestroyShader(OpalState _state, OpalShader* _shader);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !define GEM_OPAL_H
