#ifndef OPAL_DEFINES_H
#define OPAL_DEFINES_H 1

#include "./vulkan_defines.h"

#include <stdint.h>
#include <stdbool.h>

// Primitives
// ============================================================

typedef enum OpalResult
{
  Opal_Success,               // Normal execution
  Opal_Failure_Unknown,       // Unknown issue
  Opal_Failure_Api,           // Graphics Api function failure
  Opal_Failure_Invalid_Input, // Invalid input from client
} OpalResult;

typedef enum OpalFormat
{
  Opal_Format_Unknown,

  // Float variants
  Opal_Format_R8 , Opal_Format_RG8 , Opal_Format_RGB8 , Opal_Format_RGBA8 ,
  Opal_Format_R16, Opal_Format_RG16, Opal_Format_RGB16, Opal_Format_RGBA16,
  Opal_Format_R32, Opal_Format_RG32, Opal_Format_RGB32, Opal_Format_RGBA32,
  Opal_Format_R64, Opal_Format_RG64, Opal_Format_RGB64, Opal_Format_RGBA64,

  // Int variants
  Opal_Format_R8_I , Opal_Format_RG8_I , Opal_Format_RGB8_I , Opal_Format_RGBA8_I ,
  Opal_Format_R16_I, Opal_Format_RG16_I, Opal_Format_RGB16_I, Opal_Format_RGBA16_I,
  Opal_Format_R32_I, Opal_Format_RG32_I, Opal_Format_RGB32_I, Opal_Format_RGBA32_I,
  Opal_Format_R64_I, Opal_Format_RG64_I, Opal_Format_RGB64_I, Opal_Format_RGBA64_I,

  // Uint variants
  Opal_Format_R8_U , Opal_Format_RG8_U , Opal_Format_RGB8_U , Opal_Format_RGBA8_U ,
  Opal_Format_R16_U, Opal_Format_RG16_U, Opal_Format_RGB16_U, Opal_Format_RGBA16_U,
  Opal_Format_R32_U, Opal_Format_RG32_U, Opal_Format_RGB32_U, Opal_Format_RGBA32_U,
  Opal_Format_R64_U, Opal_Format_RG64_U, Opal_Format_RGB64_U, Opal_Format_RGBA64_U,

  // Depth
  Opal_Format_D24_S8,
  Opal_Format_D32,

  // Other
  Opal_Format_Mat4x4
} OpalFormat;

typedef enum OpalMessageType
{
  Opal_Message_Info,
  Opal_Message_Error,
} OpalMessageType;

typedef enum OpalStageFlagBits
{
  Opal_Stage_Undefined    = 0,
  Opal_Stage_Vertex       = 0x01,
  Opal_Stage_Geometry     = 0x02,
  Opal_Stage_Tesselation  = 0x04,
  Opal_Stage_Fragment     = 0x08,
  Opal_Stage_All_Graphics = 0x0f,

  Opal_Stage_Compute = 0x10,

  Opal_Stage_All = 0xff,
} OpalStageFlagBits;
typedef uint32_t OpalStageFlags;

// Core
// ============================================================

typedef enum OpalGraphicsApi
{
  Opal_Api_Invalid,
  Opal_Api_Vulkan,
} OpalGraphicsApi;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
typedef struct OpalPlatformWindowInfo
{
  void* hwnd;
  void* hinstance;
} OpalPlatformWindowInfo;
#else
#error "Unsupported platform"
typedef struct OpalPlatformWindowInfo
{
  int unused;
} OpalPlatformWindowInfo;
#endif // OPAL_PLATFORM_*

// Window
// ============================================================

typedef struct OpalWindowInitInfo
{
  OpalPlatformWindowInfo platform;
  uint32_t width;
  uint32_t height;
} OpalWindowInitInfo;

typedef struct OpalWindow
{
  uint32_t width, height;
  uint32_t imageCount;

  union
  {
    OpalVulkanWindow vk;
  }api;
} OpalWindow;

// Buffer
// ============================================================

typedef enum OpalBufferUsageFlagBits
{
  Opal_Buffer_Usage_Undefined    = 0,
  Opal_Buffer_Usage_Transfer_Src = 0x01,
  Opal_Buffer_Usage_Transfer_Dst = 0x02,
  Opal_Buffer_Usage_Uniform      = 0x04,
  Opal_Buffer_Usage_Vertex       = 0x08,
  Opal_Buffer_Usage_Index        = 0x10,
  Opal_Buffer_Usage_Cpu_Read     = 0x20
} OpalBufferUsageFlagBits;
typedef uint32_t OpalBufferUsageFlags;

typedef struct OpalBufferInitInfo
{
  uint64_t size;
  OpalBufferUsageFlags usage;

} OpalBufferInitInfo;

typedef struct OpalBuffer
{
  uint64_t size;
  uint64_t paddedSize;

  union
  {
    OpalVulkanBuffer vk;
  } api;
} OpalBuffer;

// Image
// ============================================================

typedef enum OpalImageUsageFlagBits
{
  Opal_Image_Usage_Color    = 0x01,
  Opal_Image_Usage_Depth    = 0x02,
  Opal_Image_Usage_Copy_Src = 0x04,
  Opal_Image_Usage_Copy_Dst = 0x08,
  Opal_Image_Usage_Uniform  = 0x10
} OpalImageUsageFlagBits;
typedef uint32_t OpalImageUsageFlags;

typedef enum OpalImageFilterType
{
  Opal_Image_Filter_Point,
  Opal_Image_Filter_Linear,
} OpalImageFilterType;

typedef enum OpalImageSampleMode
{
  Opal_Image_Sample_Wrap,
  Opal_Image_Sample_Clamp,
  Opal_Image_Sample_Reflect
} OpalImageSampleMode;

typedef struct OpalImageInitInfo
{
  uint32_t width, height;
  uint32_t mipLevels;
  OpalFormat format;
  OpalImageUsageFlags usage;
  OpalImageFilterType filter;
  OpalImageSampleMode sampleMode;
} OpalImageInitInfo;

typedef struct OpalImage
{
  uint32_t width, height;
  OpalFormat format;

  union
  {
    OpalVulkanImage vk;
  } api;
} OpalImage;

// Renderpass
// ============================================================

typedef union OpalColorValue
{
  float float32[4];
  int32_t int32[4];
  uint32_t uint32[4];
} OpalColorValue;

typedef struct OpalDepthStencilValue
{
  float depth;
  uint32_t stencil;
} OpalDepthStencilValue;

typedef union OpalClearValue
{
  OpalColorValue color;
  OpalDepthStencilValue depthStencil;
} OpalClearValue;

typedef enum OpalAttachmentUsage
{
  Opal_Attachment_Usage_Unused,
  Opal_Attachment_Usage_Preserved,
  Opal_Attachment_Usage_Input,
  Opal_Attachment_Usage_Output_Color,
  Opal_Attachment_Usage_Output_Depth,
  Opal_Attachment_Usage_Output_Presented,
} OpalAttachmentUsage;

typedef enum OpalAttachmentLoadOp
{
  Opal_Attachment_Load_Op_Dont_Care,
  Opal_Attachment_Load_Op_Clear,
  Opal_Attachment_Load_Op_Load
} OpalAttachmentLoadOp;

typedef struct OpalAttachmentInfo
{
  OpalFormat format;
  OpalClearValue clearValue;
  OpalAttachmentLoadOp loadOp;
  bool shouldStore;

  const OpalAttachmentUsage* pSubpassUsages;
} OpalAttachmentInfo;

typedef struct OpalRenderpassInitInfo
{
  uint32_t subpassCount;
  uint32_t attachmentCount;
  const OpalAttachmentInfo* pAttachments;
} OpalRenderpassInitInfo;

typedef struct OpalRenderpass
{
  uint32_t subpassCount;
  uint32_t attachmentCount;

  union
  {
    OpalVulkanRenderpass vk;
  } api;
} OpalRenderpass;

// Framebuffer
// ============================================================

typedef struct OpalFramebufferInitInfo
{
  uint32_t imageCount;
  const OpalImage* images;

  OpalRenderpass renderpass;

} OpalFramebufferInitInfo;

typedef struct OpalFramebuffer
{
  uint32_t width, height;

  union
  {
    OpalVulkanFramebuffer vk;
  } api;
} OpalFramebuffer;

// Shader
// ============================================================

typedef enum OpalShaderType
{
  Opal_Shader_Vertex,
  Opal_Shader_Fragment,
  Opal_Shader_Geometry,
  Opal_Shader_Compute
} OpalShaderType;

typedef struct OpalShaderInitInfo
{
  uint32_t sourceSize;
  void* pSource;
  OpalShaderType type;
} OpalShaderInitInfo;

typedef struct OpalShader
{
  OpalShaderType type;

  union
  {
    OpalVulkanShader vk;
  } api;
} OpalShader;

// Shader group
// ============================================================

typedef enum OpalPipelineFlagBits
{
  Opal_Pipeline_Cull_Back  = (0 << 0),
  Opal_Pipeline_Cull_Front = (1 << 0),
  Opal_Pipeline_Cull_Both  = (2 << 0),
  Opal_Pipeline_Cull_None  = (3 << 0),
  Opal_Pipeline_Cull_BITS  = (3 << 0),

  Opal_Pipeline_Depth_Compare_Less      = (0 << 2),
  Opal_Pipeline_Depth_Compare_LessEqual = (1 << 2),
  Opal_Pipeline_Depth_Compare_BITS      = (1 << 2),
} OpalPipelineFlagBits;
typedef uint64_t OpalPipelineFlags;

typedef struct OpalShaderGroupInitInfo
{
  OpalRenderpass renderpass;
  uint32_t subpassIndex;

  uint32_t shaderCount;
  OpalShader* pShaders;

  OpalPipelineFlags flags;
} OpalShaderGroupInitInfo;

typedef struct OpalShaderGroup
{
  union
  {
    OpalVulkanShaderGroup vk;
  } api;
} OpalShaderGroup;

// Shader input
// ============================================================

typedef enum OpalShaderInputType
{
  Opal_Shader_Input_Buffer,
  Opal_Shader_Input_Image
} OpalShaderInputType;

typedef struct OpalShaderInputLayout
{
  uint32_t count;
  OpalShaderInputType* pTypes;
  OpalStageFlags* pStages;
} OpalShaderInputLayout;

typedef union OpalShaderInputValue
{
  OpalBuffer* buffer;
  OpalImage* image;
} OpalShaderInputValue;

typedef struct OpalShaderInputInitInfo
{
  OpalShaderInputLayout layout;
  OpalShaderInputValue* pValues;
} OpalShaderInputInitInfo;

typedef struct OpalShaderInput
{
  union
  {
    OpalVulkanShaderInput vk;
  } api;
} OpalShaderInput;

// State
// ============================================================

typedef struct OpalInitInfo
{
  OpalGraphicsApi api;
  bool useDebug;

  OpalPlatformWindowInfo window;
} OpalInitInfo;

typedef struct OpalState
{
  struct // Backend
  {
    struct // Function pointers
    {
      // ==============================
      // Objects
      // ==============================

      // Window ==========
      OpalResult (*WindowInit)         (OpalWindow* pWindow, OpalWindowInitInfo initInfo);
      void       (*WindowShutdown)     (OpalWindow* pWindow);
      OpalResult (*WindowSwapBuffers)  (const OpalWindow* pWindow);
      OpalResult (*WindowGetFrameImage)(const OpalWindow* pWindow, uint32_t frameIndex, OpalImage* pImage);

      // Buffer ==========
      OpalResult (*BufferInit)         (OpalBuffer* pBuffer, OpalBufferInitInfo initInfo);
      void       (*BufferShutdown)     (OpalBuffer* pBuffer);

      // Image ==========
      OpalResult (*ImageInit)          (OpalImage* pImage, OpalImageInitInfo initInfo);
      void       (*ImageShutdown)      (OpalImage* pImage);

      // Renderpass ==========
      OpalResult (*RenderpassInit)     (OpalRenderpass* pRenderpass, OpalRenderpassInitInfo initInfo);
      void       (*RenderpassShutdown) (OpalRenderpass* pRenderpass);

      // Framebuffer ==========
      OpalResult (*FramebufferInit)    (OpalFramebuffer* pFramebuffer, OpalFramebufferInitInfo initInfo);
      void       (*FramebufferShutdown)(OpalFramebuffer* pFramebuffer);

      // Shader ==========
      OpalResult (*ShaderInit)         (OpalShader* pShader, OpalShaderInitInfo initInfo);
      void       (*ShaderShutdown)     (OpalShader* pShader);

      // ShaderGroup ==========
      OpalResult (*ShaderGroupInit)    (OpalShaderGroup* pShaderGroup, OpalShaderGroupInitInfo initInfo);
      void       (*ShaderGroupShutdown)(OpalShaderGroup* pShaderGroup);

      // ShaderInput ==========
      OpalResult (*ShaderInputInit)    (OpalShaderInput* pShaderInput, OpalShaderInputInitInfo initInfo);
      void       (*ShaderInputShutdown)(OpalShaderInput* pShaderInput);

      // ==============================
      // Rendering
      // ==============================

      OpalResult (*RenderBegin)          ();
      OpalResult (*RenderEnd)            ();
      void       (*RenderRenderpassBegin)(const OpalRenderpass* pRenderpass, const OpalFramebuffer* pFramebuffer);
      void       (*RenderRenderpassEnd)  (const OpalRenderpass* pRenderpass);

    } functions;

    union // Backend states
    {
      OpalVulkanState vk;
      // Opengl state
      // Directx state
    };
  } api;

  void(*messageCallback)(OpalMessageType, const char*);
} OpalState;

#endif // !OPAL_DEFINES_H
