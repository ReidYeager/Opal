#ifndef OPAL_DEFINES_H
#define OPAL_DEFINES_H 1

#include "./vulkan_defines.h"

#include <stdint.h>
#include <stdbool.h>

// Preprocessor
// ============================================================

// Platform

#if !defined(OPAL_PLATFORM_WIN32)
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define OPAL_PLATFORM_WIN32 1
#ifndef _WIN64
#error "Must have 64-bit windows"
#endif
#else
#error "Unsupported platform"
#endif // OPAL_PLATFORM_*
#endif // defined(OPAL_PLATFORM_*)


// Primitives
// ============================================================

typedef enum OpalResult
{
  Opal_Success,               // Normal execution
  Opal_Failure_Unknown,       // Unknown issue
  Opal_Failure_Api,           // Graphics Api function failure
  Opal_Failure_Invalid_Input, // Invalid input from client
  Opal_Failure_Window_Minimized, // Window for presentation is minimized
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
  Opal_Stage_Undefined = 0,

  // Graphics
  Opal_Stage_Vertex       = 0x01,
  Opal_Stage_Geometry     = 0x02,
  Opal_Stage_Tesselation  = 0x04,
  Opal_Stage_Fragment     = 0x08,
  Opal_Stage_All_Graphics = 0x0f,

  // Other
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

#if defined(OPAL_PLATFORM_WIN32)
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

// Buffer
// ============================================================

typedef enum OpalBufferUsageFlagBits
{
  Opal_Buffer_Usage_Undefined    = 0,
  Opal_Buffer_Usage_Transfer_Src = 0x01,
  Opal_Buffer_Usage_Transfer_Dst = 0x02,
  Opal_Buffer_Usage_Cpu_Read     = 0x04,
  Opal_Buffer_Usage_Vertex       = 0x08,
  Opal_Buffer_Usage_Index        = 0x10,
  Opal_Buffer_Usage_Uniform      = 0x20,
  Opal_Buffer_Usage_Storage      = 0x40
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
  OpalBufferUsageFlags usage;

  union
  {
    OpalVulkanBuffer vk;
  } api;
} OpalBuffer;

// Image
// ============================================================

typedef enum OpalImageUsageFlagBits
{
  Opal_Image_Usage_Undefined       = 0,
  Opal_Image_Usage_Output          = 0x01,
  Opal_Image_Usage_Transfer_Src    = 0x02,
  Opal_Image_Usage_Transfer_Dst    = 0x04,
  Opal_Image_Usage_Uniform         = 0x08,
  Opal_Image_Usage_Subpass_Product = 0x10,
  Opal_Image_Usage_Storage         = 0x20
} OpalImageUsageFlagBits;
typedef uint32_t OpalImageUsageFlags;

typedef enum OpalImageFilterType
{
  Opal_Image_Filter_Point,
  Opal_Image_Filter_Linear
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
  uint32_t mipCount;
  OpalFormat format;
  OpalImageUsageFlags usage;
  OpalImageFilterType filter;
  OpalImageSampleMode sampleMode;
} OpalImageInitInfo;

typedef struct OpalImage
{
  uint32_t width, height;
  uint32_t mipCount;
  OpalFormat format;
  OpalImageUsageFlags usage;
  OpalImageFilterType filter;
  OpalImageSampleMode sampleMode;
  bool isMipReference;

  union
  {
    OpalVulkanImage vk;
  } api;
} OpalImage;

// Mesh
// ============================================================

typedef struct OpalMeshInitInfo
{
  uint32_t vertexCount;
  const void* pVertices;

  uint32_t indexCount;
  const uint32_t* pIndices;
} OpalMeshInitInfo;

typedef struct OpalMesh
{
  uint32_t vertexCount;
  OpalBuffer vertexBuffer;

  uint32_t indexCount;
  OpalBuffer indexBuffer;
} OpalMesh;

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
  Opal_Attachment_Usage_Output,
  Opal_Attachment_Usage_Output_Uniform,
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
  uint32_t attachmentCount;
  uint32_t subpassCount;

  bool presents;

  union
  {
    OpalVulkanRenderpass vk;
  } api;
} OpalRenderpass;

// Framebuffer
// ============================================================

typedef struct OpalFramebufferInitInfo
{
  OpalRenderpass renderpass;
  uint32_t imageCount;
  OpalImage** ppImages;
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
  Opal_Shader_Compute,
  //Opal_Shader_Geometry,
  //Opal_Shader_Mesh,
} OpalShaderType;

typedef struct OpalShaderInitInfo
{
  uint32_t sourceSize;
  const void* pSource;
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

// ShaderInput
// ============================================================

typedef enum OpalShaderInputType
{
  Opal_Shader_Input_Buffer,
  Opal_Shader_Input_Image,
  Opal_Shader_Input_Subpass_Product,
  Opal_Shader_Input_Storage_Buffer,
  Opal_Shader_Input_Storage_Image,
} OpalShaderInputType;

typedef struct OpalShaderInputLayoutInitInfo
{
  uint32_t count;
  const OpalShaderInputType* pTypes;
  const OpalStageFlags* pStages;
} OpalShaderInputLayoutInitInfo;

typedef struct OpalShaderInputLayout
{
  uint32_t elementCount;
  OpalShaderInputType* pTypes;

  union
  {
    OpalVulkanShaderInputLayout vk;
  } api;
} OpalShaderInputLayout;

typedef union OpalShaderInputValue
{
  const OpalBuffer* buffer;
  const OpalImage* image;
} OpalShaderInputValue;

typedef struct OpalShaderInputInitInfo
{
  OpalShaderInputLayout layout;
  const OpalShaderInputValue* pValues;
} OpalShaderInputInitInfo;

typedef struct OpalShaderInput
{
  OpalShaderInputLayout layout;

  union
  {
    OpalVulkanShaderInput vk;
  } api;
} OpalShaderInput;

// ShaderGroup
// ============================================================

typedef enum OpalPipelineFlagBits
{
  Opal_Pipeline_Cull_Back               = (0 << 0),
  Opal_Pipeline_Cull_Front              = (1 << 0),
  Opal_Pipeline_Cull_Both               = (2 << 0),
  Opal_Pipeline_Cull_None               = (3 << 0),
  Opal_Pipeline_Cull_BITS               = (3 << 0),

  Opal_Pipeline_Depth_Compare_Less      = (0 << 2),
  Opal_Pipeline_Depth_Compare_LessEqual = (1 << 2),
  Opal_Pipeline_Depth_Compare_BITS      = (1 << 2),
} OpalPipelineFlagBits;
typedef uint64_t OpalPipelineFlags;

typedef enum OpalShaderGroupType
{
  Opal_Group_Graphics,
  Opal_Group_Compute,
  Opal_Group_Raytracing,
} OpalShaderGroupType;

typedef struct OpalShaderGroupInitInfo
{
  OpalShaderGroupType type;

  uint32_t shaderCount;
  OpalShader* pShaders;

  uint32_t shaderInputLayoutCount;
  OpalShaderInputLayout* pShaderInputLayouts;

  uint8_t pushConstantSize;

  union
  {
    struct
    {
      uint32_t subpassIndex;
      OpalRenderpass renderpass;
      OpalPipelineFlags flags;
    } graphics;
  };
} OpalShaderGroupInitInfo;

typedef struct OpalShaderGroup
{
  OpalShaderGroupType type;
  uint32_t pushConstSize;

  union
  {
    OpalVulkanShaderGroup vk;
  } api;
} OpalShaderGroup;

// Window
// ============================================================

typedef struct OpalWindowInitInfo
{
  OpalPlatformWindowInfo platform;
  uint32_t width;
  uint32_t height;
  OpalFormat desiredImageFormat;
  // Uses assigned to swapchain images
  // Guaranteed to include Opal_Image_Usage_Output
  OpalImageUsageFlags imageUse;
} OpalWindowInitInfo;

typedef struct OpalWindow
{
  uint32_t width, height;
  OpalFormat imageFormat;
  bool isMinimized;

  uint32_t imageCount;
  OpalImage* pImages;

  union
  {
    OpalVulkanWindow vk;
  }api;
} OpalWindow;

// Synchronization
// ============================================================

typedef struct OpalFence
{
  union
  {
    OpalVulkanFence vk;
  } api;
} OpalFence;

typedef struct OpalSemaphore
{
  union
  {
    OpalVulkanSemaphore vk;
  } api;
} OpalSemaphore;

typedef struct OpalSyncPack
{
  uint32_t waitCount;
  OpalSemaphore* pWaitSemaphores;

  uint32_t signalCount;
  OpalSemaphore* pSignalSemaphores;
} OpalSyncPack;

// State
// ============================================================

typedef struct OpalVertexLayoutInfo
{
  uint32_t elementCount;
  OpalFormat* pElementFormats;
} OpalVertexLayoutInfo;

typedef struct OpalInitInfo
{
  OpalGraphicsApi api;
  bool useDebug;

  void(*messageCallback)(OpalMessageType, const char*);

  OpalVertexLayoutInfo vertexLayout;
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

      // Core ==========
      void       (*Shutdown)             ();
      void       (*WaitIdle)             ();

      // Window ==========
      OpalResult (*WindowInit)           (OpalWindow* pWindow, OpalWindowInitInfo initInfo);
      void       (*WindowShutdown)       (OpalWindow* pWindow);

      // Buffer ==========
      OpalResult (*BufferInit)           (OpalBuffer* pBuffer, OpalBufferInitInfo initInfo);
      void       (*BufferShutdown)       (OpalBuffer* pBuffer);
      OpalResult (*BufferPushData)       (OpalBuffer* pBuffer, const void* data);
      OpalResult (*BufferPushDataSegment)(OpalBuffer* pBuffer, const void* data, uint64_t size, uint64_t bufferOffset);
      OpalResult (*BufferDumpData)       (OpalBuffer* pBuffer, void* outData);
      OpalResult (*BufferDumpDataSegment)(OpalBuffer* pBuffer, void* outData, uint64_t size, uint64_t offset);

      // Image ==========
      OpalResult (*ImageInit)            (OpalImage* pImage, OpalImageInitInfo initInfo);
      void       (*ImageShutdown)        (OpalImage* pImage);
      OpalResult (*ImagePushData)        (OpalImage* pImage, const void* data);
      OpalResult (*ImageCopyImage)       (OpalImage* pImage, OpalImage* pSourceImage, OpalImageFilterType filter);
      OpalResult (*ImageGetMipAsImage)   (OpalImage* pImage, OpalImage* pMipImage, uint32_t mipLevel);

      // Renderpass ==========
      OpalResult (*RenderpassInit)       (OpalRenderpass* pRenderpass, OpalRenderpassInitInfo initInfo);
      void       (*RenderpassShutdown)   (OpalRenderpass* pRenderpass);

      // Framebuffer ==========
      OpalResult (*FramebufferInit)      (OpalFramebuffer* pFramebuffer, OpalFramebufferInitInfo initInfo);
      void       (*FramebufferShutdown)  (OpalFramebuffer* pFramebuffer);

      // Shader ==========
      OpalResult (*ShaderInit)           (OpalShader* pShader, OpalShaderInitInfo initInfo);
      void       (*ShaderShutdown)       (OpalShader* pShader);

      // ShaderGroup ==========
      OpalResult (*ShaderGroupInit)      (OpalShaderGroup* pShaderGroup, OpalShaderGroupInitInfo initInfo);
      void       (*ShaderGroupShutdown)  (OpalShaderGroup* pShaderGroup);

      // ShaderInput ==========
      OpalResult (*ShaderInputLayoutInit)    (OpalShaderInputLayout* pLayout, OpalShaderInputLayoutInitInfo initInfo);
      void       (*ShaderInputLayoutShutdown)(OpalShaderInputLayout* pLayout);
      OpalResult (*ShaderInputInit)          (OpalShaderInput* pShaderInput, OpalShaderInputInitInfo initInfo);
      void       (*ShaderInputShutdown)      (OpalShaderInput* pShaderInput);

      // Synchronization ==========
      OpalResult (*FenceInit)                (OpalFence* pFence, bool startSignaled);
      void       (*FenceShutdown)            (OpalFence* pFence);
      OpalResult (*SemaphoreInit)            (OpalSemaphore* pSemaphore);
      void       (*SemaphoreShutdown)        (OpalSemaphore* pSemaphore);

      // ==============================
      // Rendering
      // ==============================

      OpalResult (*RenderBegin)                ();
      OpalResult (*RenderEnd)                  (OpalSyncPack syncInfo);
      OpalResult (*RenderToWindowBegin)        (OpalWindow* pWindow);
      OpalResult (*RenderToWindowEnd)          (OpalWindow* pWindow);
      void       (*RenderRenderpassBegin)      (const OpalRenderpass* pRenderpass, const OpalFramebuffer* pFramebuffer);
      void       (*RenderRenderpassEnd)        (const OpalRenderpass* pRenderpass);
      void       (*RenderRenderpassNext)       (const OpalRenderpass* pRenderpass);
      void       (*RenderSetViewportDimensions)(uint32_t width, uint32_t height);
      void       (*RenderBindShaderGroup)      (const OpalShaderGroup* pGroup);
      void       (*RenderBindShaderInput)      (const OpalShaderInput* pInput, uint32_t setIndex);
      void       (*RenderSetPushConstant)      (const void* data);
      void       (*RenderMesh)                 (const OpalMesh* p);
      void       (*ComputeDispatch)            (uint32_t x, uint32_t y, uint32_t z);

    } functions;

    union // Backend states
    {
      OpalVulkanState vk;
      // Opengl state
      // Directx state
    };

    OpalGraphicsApi currentApi;
  } api;

  struct
  {
    uint32_t size;
    uint32_t attribCount;
    OpalFormat* pFormats;
  } vertex;

  void(*messageCallback)(OpalMessageType, const char*);
} OpalState;

#endif // !OPAL_DEFINES_H
