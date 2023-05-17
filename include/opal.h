
#ifndef GEM_OPAL_H
#define GEM_OPAL_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "include/opal_defines.h"

uint32_t OpalFormatToSize(OpalFormat _format);

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
// Image
// =====
OpalResult OpalCreateImage(OpalState _state, OpalCreateImageInfo _createInfo, OpalImage* _outImage);
void OpalDestroyImage(OpalState _state, OpalImage* _image);

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

// =====
// Mesh
// =====
OpalResult OpalCreateMesh(OpalState _state, OpalCreateMeshInfo _createInfo, OpalMesh* _outMesh);
void OpalDestroyMesh(OpalState _state, OpalMesh* _mesh);

// Rendering

OpalResult OpalCreateRenderable(
  OpalState _state,
  OpalMesh _mesh,
  OpalMaterial _material,
  OpalShaderArg* objectArguments,
  OpalRenderable* _renderable);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !define GEM_OPAL_H
