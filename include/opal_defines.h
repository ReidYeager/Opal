
#ifndef GEM_OPAL_DEFINES_H
#define GEM_OPAL_DEFINES_H 1

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "include/opal_define_image.h"
#include <lapis.h>

typedef enum OpalResult
{
  Opal_Success = 0,
  Opal_Failure
} OpalResult;

#define DEFINE_OPAL_HANDLE(name) typedef struct name##_T* name

DEFINE_OPAL_HANDLE(OpalWindow);
DEFINE_OPAL_HANDLE(OpalRenderpass);
DEFINE_OPAL_HANDLE(OpalFramebuffer);
DEFINE_OPAL_HANDLE(OpalShader);
DEFINE_OPAL_HANDLE(OpalMaterial);

typedef struct OpalRenderpassInitInfo
{
  uint32_t imageCount;
  OpalClearValue* pClearValues;
} OpalRenderpassInitInfo;

typedef struct OpalFramebufferInitInfo
{
  OpalRenderpass renderpass;
} OpalFramebufferInitInfo;

typedef enum OpalShaderType
{
  Opal_Shader_Vertex,
  Opal_Shader_Fragment,
  //Opal_Shader_Compute, // Not supported yet
  //Opal_Shader_Geometry // Not supported yet
} OpalShaderType;

typedef struct OpalShaderInitInfo
{
  char* pSource;
  uint32_t size;
  OpalShaderType type;
} OpalShaderInitInfo;

typedef struct OpalMaterialInitInfo
{
  OpalRenderpass renderpass;
  uint32_t subpassIndex;

  OpalShader* pShaders;
  uint32_t shaderCount;
} OpalMaterialInitInfo;

typedef struct OpalInitInfo
{
  LapisWindow* window;
} OpalInitInfo;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !GEM_OPAL_DEFINES_H
