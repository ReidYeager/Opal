
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

typedef enum OpalFormat
{
  Opal_Format_8_Bit_Int_1,
  Opal_Format_8_Bit_Int_2,
  Opal_Format_8_Bit_Int_3,
  Opal_Format_8_Bit_Int_4,
  Opal_Format_8_Bit_Uint_1,
  Opal_Format_8_Bit_Uint_2,
  Opal_Format_8_Bit_Uint_3,
  Opal_Format_8_Bit_Uint_4,
  Opal_Format_8_Bit_Unorm_1,
  Opal_Format_8_Bit_Unorm_2,
  Opal_Format_8_Bit_Unorm_3,
  Opal_Format_8_Bit_Unorm_4,
  Opal_Format_32_Bit_Int_1,
  Opal_Format_32_Bit_Int_2,
  Opal_Format_32_Bit_Int_3,
  Opal_Format_32_Bit_Int_4,
  Opal_Format_32_Bit_Uint_1,
  Opal_Format_32_Bit_Uint_2,
  Opal_Format_32_Bit_Uint_3,
  Opal_Format_32_Bit_Uint_4,
  Opal_Format_32_Bit_Float_1,
  Opal_Format_32_Bit_Float_2,
  Opal_Format_32_Bit_Float_3,
  Opal_Format_32_Bit_Float_4,
  Opal_Format_24_Bit_Depth_8_Bit_Stencil
} OpalFormat;

// =====
// Buffer
// =====

typedef struct OpalBuffer_T* OpalBuffer;

typedef enum OpalBufferUsageBits
{
  Opal_Buffer_Usage_Cpu_Read       = 0x01,
  Opal_Buffer_Usage_Vertex         = 0x02,
  Opal_Buffer_Usage_Index          = 0x04,
  Opal_Buffer_Usage_Shader_Uniform = 0x08,
} OpalBufferUsageBits;
typedef uint32_t OpalBufferUsageFlags;

typedef struct OpalCreateBufferInfo
{
  uint64_t size;
  OpalBufferUsageFlags usage;
} OpalCreateBufferInfo;

// =====
// Image
// =====

typedef struct OpalImage_T* OpalImage;

typedef enum OpalImageUsageBits
{
  Opal_Image_Usage_Shader_Sampled = 0x01
} OpalImageUsageBits;
typedef uint32_t OpalImageUsageFlags;

typedef struct OpalCreateImageInfo
{
  OpalImageUsageFlags usage;

  OpalFormat pixelFormat;
  uint32_t width;
  uint32_t height;
  void* pixelData;
} OpalCreateImageInfo;

// =====
// Material
// =====

typedef struct OpalShader_T* OpalShader;
typedef struct OpalMaterial_T* OpalMaterial;

// Argument =====
typedef enum OpalShaderArgTypes
{
  Opal_Shader_Arg_Uniform_Buffer,
  Opal_Shader_Arg_Samped_Image,
} OpalShaderArgTypes;

typedef struct OpalShaderArg
{
  uint32_t index;

  OpalShaderArgTypes type;
  union {
    OpalBuffer buffer;
    OpalImage image;
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
  OpalFormat* pElementFormats;
} OpalVertexLayoutInfo;

typedef struct OpalCreateMeshInfo
{
  uint32_t vertexCount;
  void* pVertices;

  uint32_t indexCount;
  uint32_t* pIndices;
} OpalCreateMeshInfo;

// =====
// Rendering
// =====

typedef enum OpalRenderpassAttachmentLoadOp {
  Opal_Attachment_LoadOp_Clear,
  Opal_Attachment_LoadOp_Load,
  Opal_Attachment_LoadOp_Dont_Care
} OpalRenderpassAttachmentLoadOp;

typedef enum OpalRenderpassAttachmentUsage {
  Opal_Attachment_Usage_Color,
  Opal_Attachment_Usage_Depth,
  Opal_Attachment_Usage_Presented
} OpalRenderpassAttachmentUsage;

typedef struct OpalRenderpassAttachment {
  uint32_t imageCount;
  OpalImage* images;
  OpalRenderpassAttachmentUsage usage;

  OpalRenderpassAttachmentLoadOp loadOperation;
  uint8_t shouldStoreReneredData;
} OpalRenderpassAttachment;

// =====
// Core
// =====

typedef struct OpalState_T* OpalState;

typedef struct OpalObjectShaderArgumentsInfo
{
  uint32_t argumentCount;
  OpalShaderArgTypes* args;
} OpalObjectShaderArgumentsInfo;

typedef struct OpalCreateStateInfo
{
  OpalApi api;
  LapisWindow window; // TODO : Allow headless when nullptr
  OpalVertexLayoutInfo* pCustomVertexLayout;
  OpalObjectShaderArgumentsInfo* pCustomObjectShaderArgumentLayout;

  uint32_t renderpassCount;
  OpalRenderpassAttachment pCustomRenderpasses;
} OpalCreateStateInfo;

typedef struct OpalRenderable_T* OpalRenderable;

typedef struct OpalFrameData
{
  uint32_t renderableCount;
  OpalRenderable renderables[512];
} OpalFrameData;

#endif // !GEM_OPAL_DEFINES_H
