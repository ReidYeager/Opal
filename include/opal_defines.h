
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

typedef struct OpalExtents2D
{
  uint32_t width;
  uint32_t height;
} OpalExtents2D;

typedef struct OpalRenderpass_T* OpalRenderpass;

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
  Opal_Image_Usage_Shader_Sampled = 0x01,
  Opal_Image_Usage_Color_Attachment = 0x02,
  Opal_Image_Usage_Depth_Attachment = 0x04,
  Opal_Image_Usage_Stencil_Attachment = 0x08,
  Opal_Image_Usage_Input_Attachment = 0x10
} OpalImageUsageBits;
typedef uint32_t OpalImageUsageFlags;

typedef struct OpalCreateImageInfo
{
  OpalImageUsageFlags usage;

  OpalFormat pixelFormat;
  OpalExtents2D extents;
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
  Opal_Shader_Arg_Subpass_Input
} OpalShaderArgTypes;

typedef union OpalShaderArgInputTypeValues
{
  OpalBuffer buffer;
  OpalImage image;
  struct {
    uint32_t attachmentIndex;
    OpalImage image;
  } inputAttachment;
} OpalShaderArgInputTypeValues;

typedef struct OpalShaderArg
{
  uint32_t index;

  OpalShaderArgTypes type;
  OpalShaderArgInputTypeValues inputValue;
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
  OpalRenderpass renderpass;
  uint32_t subpassIndex;
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

typedef union OpalClearColor {
  float float32[4];
  uint32_t uint32[4];
  int32_t int32[4];
} OpalClearColor;

typedef struct OpalClearDepth {
  float depth;
  uint32_t stencil;
} OpalClearDepth;

typedef union OpalRenderpassAttachmentClearValues{
  OpalClearColor color;
  OpalClearDepth depthStencil;
} OpalRenderpassAttachmentClearValues;

typedef struct OpalRenderpassAttachment {
  OpalRenderpassAttachmentUsage usage;

  OpalRenderpassAttachmentLoadOp loadOperation;
  uint8_t shouldStoreReneredData;

  OpalRenderpassAttachmentClearValues clearValues;
} OpalRenderpassAttachment;

#define OPAL_SUBPASS_NO_DEPTH ~0u

typedef struct OpalRenderpassSubpass {
  uint32_t depthAttachmentIndex;
  uint32_t colorAttachmentCount;
  uint32_t* pColorAttachmentIndices;
  uint32_t inputAttachmentCount;
  uint32_t* pInputAttachmentIndices;
  uint32_t preserveAttachmentCount;
  uint32_t* pPreserveAttachmentIndices;
} OpalRenderpassSubpass;

typedef struct OpalCreateRenderpassInfo {
  uint32_t imageCount;
  OpalImage* images;
  OpalRenderpassAttachment* imageAttachments;
  uint32_t subpassCount;
  OpalRenderpassSubpass* subpasses;

  OpalResult(*RenderFunction)();
  uint8_t rendersToSwapchain;
} OpalCreateRenderpassInfo;

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
  OpalRenderpassAttachment* pCustomRenderpasses;
} OpalCreateStateInfo;

typedef struct OpalRenderable_T* OpalRenderable;

typedef struct OpalFrameData
{
  uint32_t renderpassCount;
  OpalRenderpass* renderpasses;

  uint32_t renderableCount;
  OpalRenderable renderables[512];
} OpalFrameData;

#endif // !GEM_OPAL_DEFINES_H
