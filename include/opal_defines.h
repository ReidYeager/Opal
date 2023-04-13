
#ifndef GEM_OPAL_DEFINES_H
#define GEM_OPAL_DEFINES_H

#include <lapis.h>
#include <stdint.h>

typedef enum OpalResult
{
  Opal_Success,
  Opal_Failure,
  Opal_Failure_Backend,
  Opal_Failure_Vk_Create,
  Opal_Failure_Vk_Init,
  Opal_Failure_Vk_Render,
  Opal_Failure_Vk_Misc,
  Opal_Unknown
} OpalResult;

typedef enum OpalApi
{
  Opal_Api_Vulkan,
  Opal_Api_Unknown
} OpalApi;

// =====
// Buffer
// =====

typedef struct OpalBuffer_T* OpalBuffer;

typedef enum OpalBufferUsageBits
{
#define OPAL_BU(name, val) Opal_Buffer_Usage_##name = (val)
  OPAL_BU(Cpu_Read      , 0x01),
  OPAL_BU(Vertex        , 0x02),
  OPAL_BU(Index         , 0x04),
  OPAL_BU(Shader_Uniform, 0x08),
  //OPAL_BU(Image         , 0x10)
#undef OPAL_BU
} OpalBufferUsageBits;
typedef uint32_t OpalBufferUsageFlags;

typedef struct OpalCreateBufferInfo
{
  uint64_t size;
  OpalBufferUsageFlags usage;
} OpalCreateBufferInfo;

// =====
// Material
// =====

typedef struct OpalShader_T* OpalShader;
typedef struct OpalMaterial_T* OpalMaterial;

// Argument =====
typedef enum OpalShaderArgTypes
{
  Opal_Shader_Arg_Uniform_Buffer
} OpalShaderArgTypes;

typedef struct OpalShaderArg
{
  uint32_t index;

  OpalShaderArgTypes type;
  union {
    struct {
      OpalBuffer buffer;
    } bufferData;

    struct {
      int x;
    } tmp;
  };
} OpalShaderArg;

// Shader =====
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

// Material =====
typedef struct OpalCreateMaterialInfo
{
  uint32_t shaderCount;
  OpalShader* pShaders;

  uint32_t shaderArgCount;
  OpalShaderArg* pShaderArgs;
} OpalCreateMaterialInfo;

// =====
// Mesh
// =====

typedef struct OpalMesh_T* OpalMesh;

typedef struct OpalVertexLayoutInfo
{
  uint32_t structSize;
  uint32_t elementCount;
  uint32_t* pElementSizes; // TODO : Replace with a format enum (like Vulkan's)
} OpalVertexLayoutInfo;

typedef struct OpalCreateMeshInfo
{
  uint32_t vertexCount;
  void* pVertices;

  uint32_t indexCount;
  uint32_t* pIndices;
} OpalCreateMeshInfo;

// =====
// Core
// =====

typedef struct OpalState_T* OpalState;

typedef struct OpalCreateStateInfo
{
  OpalApi api;
  LapisWindow window; // TODO : Allow headless when nullptr
  OpalVertexLayoutInfo* pCustomVertexLayout;
} OpalCreateStateInfo;

typedef struct OpalFrameData
{
  uint32_t materialCount;
  OpalMaterial* materials;

  uint32_t meshCount;
  OpalMesh* meshes;

  uint32_t* meshToMaterialIndexMap;
} OpalFrameData;

#endif // !GEM_OPAL_DEFINES_H
