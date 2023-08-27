
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

#define OPAL_NULL_HANDLE NULL
#define DEFINE_OPAL_HANDLE(name) typedef struct name##_T* name

DEFINE_OPAL_HANDLE(OpalWindow);
DEFINE_OPAL_HANDLE(OpalBuffer);
DEFINE_OPAL_HANDLE(OpalRenderpass);
DEFINE_OPAL_HANDLE(OpalFramebuffer);
DEFINE_OPAL_HANDLE(OpalShader);
DEFINE_OPAL_HANDLE(OpalInputSet);
DEFINE_OPAL_HANDLE(OpalMaterial);
DEFINE_OPAL_HANDLE(OpalMesh);

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

typedef enum OpalAttachmentUsage
{
  Opal_Attachment_Usage_Color,
  Opal_Attachment_Usage_Depth,
  Opal_Attachment_Usage_Presented,
} OpalAttachmentUsage;

typedef enum OpalAttachmentLoadOp
{
  Opal_Attachment_Op_Clear,
  Opal_Attachment_Op_Load,
  Opal_Attachment_Op_Dont_Care,
} OpalAttachmentLoadOp;

typedef struct OpalAttachmentInfo
{
  OpalClearValue clearValue;
  OpalFormat format;
  OpalAttachmentUsage usage;
  OpalAttachmentLoadOp loadOp;
  bool shouldStore;
} OpalAttachmentInfo;

typedef struct OpalSubpassInfo
{
  uint32_t depthAttachmentIndex;
  uint32_t colorAttachmentCount;
  uint32_t* pColorAttachmentIndices;
  uint32_t inputAttachmentCount;
  uint32_t* pInputColorAttachmentIndices;
} OpalSubpassInfo;

typedef struct OpalDependencyInfo
{
  uint32_t srcIndex;
  uint32_t dstIndex;
} OpalDependencyInfo;

typedef struct OpalRenderpassInitInfo
{
  uint32_t imageCount;
  OpalAttachmentInfo* pAttachments;

  uint32_t subpassCount;
  OpalSubpassInfo* pSubpasses;

  uint32_t dependencyCount;
  OpalDependencyInfo* pDependencies;
} OpalRenderpassInitInfo;

typedef struct OpalFramebufferInitInfo
{
  OpalRenderpass renderpass;

  uint32_t imageCount;
  OpalImage* pImages;
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

typedef enum OpalInputType
{
  Opal_Input_Type_Uniform_Buffer,
  Opal_Input_Type_Samped_Image,
  Opal_Input_Type_Subpass_Input
} OpalInputType;

typedef union OpalMaterialInputValue
{
  OpalBuffer buffer;
  OpalImage image;
  struct {
    uint32_t attachmentIndex;
    OpalImage image;
  } inputAttachment;
} OpalMaterialInputValue;

typedef struct OpalInputInfo
{
  uint32_t index;
  OpalInputType type;
  OpalMaterialInputValue inputValue;
} OpalInputInfo;

typedef struct OpalInputSetInitInfo
{
  uint32_t count;
  OpalInputInfo* pInputs;
} OpalInputSetInitInfo;

typedef struct OpalMaterialInitInfo
{
  OpalRenderpass renderpass;
  uint32_t subpassIndex;

  uint32_t shaderCount;
  OpalShader* pShaders;

  uint32_t inputSetCount;
  OpalInputSet* pInputSets;

} OpalMaterialInitInfo;

typedef struct OpalMeshInitInfo
{
  uint32_t vertexCount;
  void* pVertices;
  uint32_t indexCount;
  uint32_t* pIndices;
} OpalMeshInitInfo;

typedef struct OpalInitInfo
{
  LapisWindow* window;
  bool debug;

  struct
  {
    uint32_t count;
    OpalFormat* pFormats;
  } vertexStruct;
} OpalInitInfo;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !GEM_OPAL_DEFINES_H
