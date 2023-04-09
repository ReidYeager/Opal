
#ifndef GEM_OPAL_DEFINES_H
#define GEM_OPAL_DEFINES_H

#include <lapis.h>
#include <stdint.h>

// =====
// Opal generic
// =====

typedef enum OpalResult
{
  Opal_Success,
  Opal_Failure,
  Opal_Failure_Backend,
  Opal_Failure_Vk_Create,
  Opal_Failure_Vk_Init,
  Opal_Failure_Vk_Render,
  Opal_Unknown
} OpalResult;

typedef enum OpalApi
{
  Opal_Api_Vulkan,
  Opal_Api_Unknown
} OpalApi;

// =====
// State
// =====

typedef struct OpalState_T* OpalState;

typedef struct OpalCreateStateInfo
{
  OpalApi api;
  LapisWindow window; // TODO : Allow headless when nullptr
} OpalCreateStateInfo;

// =====
// Material
// =====

typedef struct OpalShader_T* OpalShader;
typedef struct OpalMaterial_T* OpalMaterial;

typedef enum OpalShaderTypes
{
  Opal_Shader_Vertex,
  Opal_Shader_Fragment
} OpalShaderTypes;

typedef struct OpalCreateShaderInfo
{
  OpalShaderTypes type;
  const char* sourceCode;
  uint64_t sourceSize;
} OpalCreateShaderInfo;

typedef struct OpalCreateMaterialInfo
{
  uint32_t shaderCount;
  OpalShader* pShaders;
} OpalCreateMaterialInfo;

#endif // !GEM_OPAL_DEFINES_H
