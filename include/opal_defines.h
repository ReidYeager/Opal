
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
DEFINE_OPAL_HANDLE(OpalBuffer);
DEFINE_OPAL_HANDLE(OpalRenderpass);
DEFINE_OPAL_HANDLE(OpalFramebuffer);
DEFINE_OPAL_HANDLE(OpalShader);
DEFINE_OPAL_HANDLE(OpalMaterial);

typedef enum OpalBufferUsageBits
{
  Opal_Buffer_Usage_Transfer_Src = 0x01,
  Opal_Buffer_Usage_Transfer_Dst = 0x02,
  Opal_Buffer_Usage_Uniform = 0x04,
  Opal_Buffer_Usage_Vertex = 0x08,
  Opal_Buffer_Usage_Index = 0x10,
  Opal_Buffer_Usage_Cpu_Read = 0x20
} OpalBufferUsageBits;

typedef struct OpalBufferInitInfo
{
  uint64_t size;
  uint32_t usage;
} OpalBufferInitInfo;

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
