
#ifndef GEM_OPAL_H
#define GEM_OPAL_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#include "include/opal_defines.h"

OpalResult OpalCreateState(OpalCreateStateInfo _info,  OpalState* _outState);
void OpalDestroyState(OpalState* _state);
OpalResult OpalRenderFrame(OpalState _state, const OpalFrameData* _frameData);

// =====
// Buffer
// =====
OpalResult OpalCreateBuffer(
  OpalState _state,
  OpalCreateBufferInfo _createInfo,
  OpalBuffer* _outBuffer);
void OpalDestroyBuffer(OpalState _state, OpalBuffer* _buffer);
OpalResult OpalBufferPushData(OpalState _state, OpalBuffer _buffer, void* _data);

// =====
// Material
// =====
OpalResult OpalCreateShaders(
  OpalState _state,
  uint32_t _createCount,
  OpalCreateShaderInfo* _pCreateInfos,
  OpalShader* _pOutShaders);
void OpalDestroyShader(OpalState _state, OpalShader* _shader);
OpalResult OpalCreateMaterial(
  OpalState _state,
  OpalCreateMaterialInfo _createInfo,
  OpalMaterial* _outMaterial);
void OpalDestroyMaterial(OpalState _state, OpalMaterial* _material);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !define GEM_OPAL_H
