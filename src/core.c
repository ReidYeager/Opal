
#include "src/common.h"
#include "src/vulkan/vulkan_common.h"

#include <stdarg.h>

// Global variables
// ============================================================

OpalState g_OpalState = {0};

// Declarations
// ============================================================

// Core ==========
//OpalResult OpalInit(OpalInitInfo initInfo)
//void OpalShutdown()
//void OpalWaitIdle()

// Window ==========
//OpalResult OpalWindowInit(OpalWindow* pWindow, OpalWindowInitInfo initInfo)
//void OpalWindowShutdown(OpalWindow* pWindow)

// Buffer ==========
//OpalResult OpalBufferInit(OpalBuffer* pBuffer, OpalBufferInitInfo initInfo)
//void OpalBufferShutdown(OpalBuffer* pBuffer)

// Image ==========
//OpalResult OpalImageInit(OpalImage* pImage, OpalImageInitInfo initInfo)
//void OpalImageShutdown(OpalImage* pImage)

// Renderpass ==========
//OpalResult OpalRenderpassInit(OpalRenderpass* pRenderpass, OpalRenderpassInitInfo initInfo)
//void OpalRenderpassShutdown(OpalRenderpass* pRenderpass)

// Framebuffer ==========
//OpalResult OpalFramebufferInit(OpalFramebuffer* pFramebuffer, OpalFramebufferInitInfo initInfo)
//void OpalFramebufferShutdown(OpalFramebuffer* pFramebuffer)

// Shader ==========
//OpalResult OpalShaderInit(OpalShader* pShader, OpalShaderInitInfo initInfo)
//void OpalShaderShutdown(OpalShader* pShader)

// ShaderGroup ==========
//OpalResult OpalShaderGroupInit(OpalShaderGroup* pShaderGroup, OpalShaderGroupInitInfo initInfo)
//void OpalShaderGroupShutdown(OpalShaderGroup* pShaderGroup)

// ShaderInput ==========
//OpalResult OpalShaderInputInit(OpalShaderInput* pShaderInput, OpalShaderInputInitInfo initInfo)
//void OpalShaderInputShutdown(OpalShaderInput* pShaderInput)

// Rendering ==========
//OpalResult OpalRenderBegin()
//OpalResult OpalRenderEnd()
//OpalResult OpalRenderToWindowBegin(OpalWindow* pWindow)
//OpalResult OpalRenderToWindowEnd(OpalWindow* pWindow)
//void OpalRenderRenderpassBegin(const OpalRenderpass* pRenderpass, const OpalFramebuffer* pFramebuffer)
//void OpalRenderRenderpassEnd(const OpalRenderpass* pRenderpass)

// Tools ==========
//void     OpalOutputMessage(OpalMessageType type, const char* message, ...)
//uint32_t OpalFormatToSize (OpalFormat format)

// Core
// ============================================================

OpalResult OpalInit(OpalInitInfo initInfo)
{
  switch (initInfo.api)
  {
  case Opal_Api_Vulkan:
  {
    g_OpalState.api.functions.Shutdown              = OpalVulkanShutdown;
    g_OpalState.api.functions.WaitIdle              = OpalVulkanWaitIdle;
    // Window
    g_OpalState.api.functions.WindowInit            = OpalVulkanWindowInit;
    g_OpalState.api.functions.WindowShutdown        = OpalVulkanWindowShutdown;
    // Buffer
    g_OpalState.api.functions.BufferInit            = OpalVulkanBufferInit;
    g_OpalState.api.functions.BufferShutdown        = OpalVulkanBufferShutdown;
    g_OpalState.api.functions.BufferPushData        = OpalVulkanBufferPushData;
    g_OpalState.api.functions.BufferPushDataSegment = OpalVulkanBufferPushDataSegment;
    // Image
    g_OpalState.api.functions.ImageInit             = OpalVulkanImageInit;
    g_OpalState.api.functions.ImageShutdown         = OpalVulkanImageShutdown;
    // Renderpass
    g_OpalState.api.functions.RenderpassInit        = OpalVulkanRenderpassInit;
    g_OpalState.api.functions.RenderpassShutdown    = OpalVulkanRenderpassShutdown;
    // Framebuffer
    g_OpalState.api.functions.FramebufferInit       = OpalVulkanFramebufferInit;
    g_OpalState.api.functions.FramebufferShutdown   = OpalVulkanFramebufferShutdown;
    // Shader
    g_OpalState.api.functions.ShaderInit            = OpalVulkanShaderInit;
    g_OpalState.api.functions.ShaderShutdown        = OpalVulkanShaderShutdown;
    // ShaderGroup
    g_OpalState.api.functions.ShaderGroupInit       = OpalVulkanShaderGroupInit;
    g_OpalState.api.functions.ShaderGroupShutdown   = OpalVulkanShaderGroupShutdown;
    // ShaderInput
    g_OpalState.api.functions.ShaderInputLayoutInit     = OpalVulkanShaderInputLayoutInit;
    g_OpalState.api.functions.ShaderInputLayoutShutdown = OpalVulkanShaderInputLayoutShutdown;
    g_OpalState.api.functions.ShaderInputInit           = OpalVulkanShaderInputInit;
    g_OpalState.api.functions.ShaderInputShutdown       = OpalVulkanShaderInputShutdown;
    // Rendering
    g_OpalState.api.functions.RenderBegin           = OpalVulkanRenderBegin;
    g_OpalState.api.functions.RenderEnd             = OpalVulkanRenderEnd;
    g_OpalState.api.functions.RenderToWindowBegin   = OpalVulkanRenderToWindowBegin;
    g_OpalState.api.functions.RenderToWindowEnd     = OpalVulkanRenderToWindowEnd;
    g_OpalState.api.functions.RenderSetViewportDimensions = OpalVulkanRenderSetViewportDimensions;
    // Rendering - objects
    g_OpalState.api.functions.RenderRenderpassBegin = OpalVulkanRenderRenderpassBegin;
    g_OpalState.api.functions.RenderRenderpassEnd   = OpalVulkanRenderRenderpassEnd;
    g_OpalState.api.functions.RenderBindShaderGroup = OpalVulkanRenderBindShaderGroup;
    g_OpalState.api.functions.RenderBindShaderInput = OpalVulkanRenderBindShaderInput;

    OPAL_ATTEMPT(OpalVulkanInit(initInfo));
  } break;
  default:
  {
    OpalLogError("Invalid Api type : %d", initInfo.api);
    return Opal_Failure_Unknown;
  }
  }

  return Opal_Success;
}

void OpalShutdown()
{
  g_OpalState.api.functions.Shutdown();
}

void OpalWaitIdle()
{
  g_OpalState.api.functions.WaitIdle();
}

// Window
// ============================================================

OpalResult OpalWindowInit(OpalWindow* pWindow, OpalWindowInitInfo initInfo)
{
  OPAL_ATTEMPT(g_OpalState.api.functions.WindowInit(pWindow, initInfo));

  // pWindow->imageCount set in the api function as this may vary
  pWindow->width = initInfo.width;
  pWindow->height = initInfo.height;

  return Opal_Success;
}

void OpalWindowShutdown(OpalWindow* pWindow)
{
  g_OpalState.api.functions.WindowShutdown(pWindow);
}

// Buffer
// ============================================================

OpalResult OpalBufferInit(OpalBuffer* pBuffer, OpalBufferInitInfo initInfo)
{
  return g_OpalState.api.functions.BufferInit(pBuffer, initInfo);
}

void OpalBufferShutdown(OpalBuffer* pBuffer)
{
  g_OpalState.api.functions.BufferShutdown(pBuffer);
}

OpalResult OpalBufferPushData(OpalBuffer* pBuffer, void* data)
{
  return g_OpalState.api.functions.BufferPushData(pBuffer, data);
}
OpalResult OpalBufferPushDataSegment(OpalBuffer* pBuffer, void* data, uint64_t size, uint64_t bufferOffset)
{
  return g_OpalState.api.functions.BufferPushDataSegment(pBuffer, data, size, bufferOffset);
}

// Image
// ============================================================

OpalResult OpalImageInit(OpalImage* pImage, OpalImageInitInfo initInfo)
{
  return g_OpalState.api.functions.ImageInit(pImage, initInfo);
}

void OpalImageShutdown(OpalImage* pImage)
{
  g_OpalState.api.functions.ImageShutdown(pImage);
}

// Renderpass
// ============================================================

OpalResult OpalRenderpassInit(OpalRenderpass* pRenderpass, OpalRenderpassInitInfo initInfo)
{
  return g_OpalState.api.functions.RenderpassInit(pRenderpass, initInfo);
}

void OpalRenderpassShutdown(OpalRenderpass* pRenderpass)
{
  g_OpalState.api.functions.RenderpassShutdown(pRenderpass);
}

// Framebuffer
// ============================================================

OpalResult OpalFramebufferInit(OpalFramebuffer* pFramebuffer, OpalFramebufferInitInfo initInfo)
{
  OPAL_ATTEMPT(g_OpalState.api.functions.FramebufferInit(pFramebuffer, initInfo));

  pFramebuffer->width = initInfo.pImages[0].width;
  pFramebuffer->height = initInfo.pImages[0].height;

  return Opal_Success;
}

void OpalFramebufferShutdown(OpalFramebuffer* pFramebuffer)
{
  g_OpalState.api.functions.FramebufferShutdown(pFramebuffer);
}

// Shader
// ============================================================

OpalResult OpalShaderInit(OpalShader* pShader, OpalShaderInitInfo initInfo)
{
  return g_OpalState.api.functions.ShaderInit(pShader, initInfo);
}

void OpalShaderShutdown(OpalShader* pShader)
{
  g_OpalState.api.functions.ShaderShutdown(pShader);
}

// ShaderGroup
// ============================================================

OpalResult OpalShaderGroupInit(OpalShaderGroup* pShaderGroup, OpalShaderGroupInitInfo initInfo)
{
  return g_OpalState.api.functions.ShaderGroupInit(pShaderGroup, initInfo);
}

void OpalShaderGroupShutdown(OpalShaderGroup* pShaderGroup)
{
  g_OpalState.api.functions.ShaderGroupShutdown(pShaderGroup);
}

// ShaderInput
// ============================================================

OpalResult OpalShaderInputLayoutInit(OpalShaderInputLayout* pLayout, OpalShaderInputLayoutInitInfo initInfo)
{
  return g_OpalState.api.functions.ShaderInputLayoutInit(pLayout, initInfo);
}

void OpalShaderInputLayoutShutdown(OpalShaderInputLayout* pLayout)
{
  g_OpalState.api.functions.ShaderInputLayoutShutdown(pLayout);
}

OpalResult OpalShaderInputInit(OpalShaderInput* pShaderInput, OpalShaderInputInitInfo initInfo)
{
  return g_OpalState.api.functions.ShaderInputInit(pShaderInput, initInfo);
}

void OpalShaderInputShutdown(OpalShaderInput* pShaderInput)
{
  g_OpalState.api.functions.ShaderInputShutdown(pShaderInput);
}

// Rendering
// ============================================================

OpalResult OpalRenderBegin()
{
  return g_OpalState.api.functions.RenderBegin();
}

OpalResult OpalRenderEnd()
{
  return g_OpalState.api.functions.RenderEnd();
}

OpalResult OpalRenderToWindowBegin(OpalWindow* pWindow)
{
  return g_OpalState.api.functions.RenderToWindowBegin(pWindow);
}

OpalResult OpalRenderToWindowEnd(OpalWindow* pWindow)
{
  return g_OpalState.api.functions.RenderToWindowEnd(pWindow);
}

void OpalRenderRenderpassBegin(const OpalRenderpass* pRenderpass, const OpalFramebuffer* pFramebuffer)
{
  g_OpalState.api.functions.RenderRenderpassBegin(pRenderpass, pFramebuffer);
}

void OpalRenderRenderpassEnd(const OpalRenderpass* pRenderpass)
{
  g_OpalState.api.functions.RenderRenderpassEnd(pRenderpass);
}

void OpalRenderSetViewportDimensions(uint32_t width, uint32_t height)
{
  g_OpalState.api.functions.RenderSetViewportDimensions(width, height);
}

void OpalRenderBindShaderGroup(const OpalShaderGroup* pGroup)
{
  g_OpalState.api.functions.RenderBindShaderGroup(pGroup);
}

void OpalRenderBindShaderInput(const OpalShaderInput* pInput)
{
  g_OpalState.api.functions.RenderBindShaderInput(pInput);
}

void OpalRenderDrawCount_DEBUG(uint32_t vertexCount)
{
  vkCmdDraw(g_OpalState.api.vk.renderState.cmd, vertexCount, 1, 0, 0);
}

// Tools
// ============================================================

void OpalOutputMessage(OpalMessageType type, const char* message, ...)
{
  if (g_OpalState.messageCallback != NULL)
  {
    char messageBuffer[1024];

    va_list args;
    va_start(args, message);
    vsnprintf(messageBuffer, 1024, message, args);
    va_end(args);

    g_OpalState.messageCallback(type, messageBuffer);
  }
}

uint32_t OpalFormatToSize(OpalFormat format)
{
  switch (format)
  {
    // R G B A variants
  case Opal_Format_R8:     case Opal_Format_R8_I:     case Opal_Format_R8_U:     return 1;
  case Opal_Format_RG8:    case Opal_Format_RG8_I:    case Opal_Format_RG8_U:    return 2;
  case Opal_Format_RGB8:   case Opal_Format_RGB8_I:   case Opal_Format_RGB8_U:   return 3;
  case Opal_Format_RGBA8:  case Opal_Format_RGBA8_I:  case Opal_Format_RGBA8_U:  return 4;
  case Opal_Format_R16:    case Opal_Format_R16_I:    case Opal_Format_R16_U:    return 2;
  case Opal_Format_RG16:   case Opal_Format_RG16_I:   case Opal_Format_RG16_U:   return 4;
  case Opal_Format_RGB16:  case Opal_Format_RGB16_I:  case Opal_Format_RGB16_U:  return 6;
  case Opal_Format_RGBA16: case Opal_Format_RGBA16_I: case Opal_Format_RGBA16_U: return 8;
  case Opal_Format_R32:    case Opal_Format_R32_I:    case Opal_Format_R32_U:    return 4;
  case Opal_Format_RG32:   case Opal_Format_RG32_I:   case Opal_Format_RG32_U:   return 8;
  case Opal_Format_RGB32:  case Opal_Format_RGB32_I:  case Opal_Format_RGB32_U:  return 12;
  case Opal_Format_RGBA32: case Opal_Format_RGBA32_I: case Opal_Format_RGBA32_U: return 16;
  case Opal_Format_R64:    case Opal_Format_R64_I:    case Opal_Format_R64_U:    return 8;
  case Opal_Format_RG64:   case Opal_Format_RG64_I:   case Opal_Format_RG64_U:   return 16;
  case Opal_Format_RGB64:  case Opal_Format_RGB64_I:  case Opal_Format_RGB64_U:  return 24;
  case Opal_Format_RGBA64: case Opal_Format_RGBA64_I: case Opal_Format_RGBA64_U: return 32;

    // Depth stencils
  case Opal_Format_D24_S8: case Opal_Format_D32: return 4;

    // Other
  case Opal_Format_Mat4x4: return 64;

  default: return 0;
  }
}

