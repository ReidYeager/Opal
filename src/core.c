
#include "src/common.h"
#include "src/vulkan/vulkan_common.h"

#include <stdarg.h>

// ============================================================
// ============================================================
//
// Global variables ==========
// g_OpalState
//
// Core ==========
// OpalOutputMessage()
// OpalInit()
//
// ============================================================
// ============================================================


// Global variables
// ============================================================

OpalState g_OpalState = {};

// Core
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

OpalResult OpalInit(OpalInitInfo initInfo)
{
  switch (initInfo.api)
  {
  case Opal_Api_Vulkan:
  {
    OPAL_ATTEMPT(OpalVulkanInit(initInfo));
    // Window
    g_OpalState.api.functions.WindowInit          = OpalVulkanWindowInit;
    g_OpalState.api.functions.WindowShutdown      = OpalVulkanWindowShutdown;
    g_OpalState.api.functions.WindowSwapBuffers   = OpalVulkanWindowSwapBuffers;
    g_OpalState.api.functions.WindowGetFrameImage = OpalVulkanWindowGetFrameImage;
    // Buffer
    g_OpalState.api.functions.BufferInit          = OpalVulkanBufferInit;
    g_OpalState.api.functions.BufferShutdown      = OpalVulkanBufferShutdown;
    // Image
    g_OpalState.api.functions.ImageInit           = OpalVulkanImageInit;
    g_OpalState.api.functions.ImageShutdown       = OpalVulkanImageShutdown;
    // Renderpass
    g_OpalState.api.functions.RenderpassInit      = OpalVulkanRenderpassInit;
    g_OpalState.api.functions.RenderpassShutdown  = OpalVulkanRenderpassShutdown;
    // Framebuffer
    g_OpalState.api.functions.FramebufferInit     = OpalVulkanFramebufferInit;
    g_OpalState.api.functions.FramebufferShutdown = OpalVulkanFramebufferShutdown;
    // Shader
    g_OpalState.api.functions.ShaderInit          = OpalVulkanShaderInit;
    g_OpalState.api.functions.ShaderShutdown      = OpalVulkanShaderShutdown;
    // ShaderGroup
    g_OpalState.api.functions.ShaderGroupInit     = OpalVulkanShaderGroupInit;
    g_OpalState.api.functions.ShaderGroupShutdown = OpalVulkanShaderGroupShutdown;
    // ShaderInput
    g_OpalState.api.functions.ShaderInputInit     = OpalVulkanShaderInputInit;
    g_OpalState.api.functions.ShaderInputShutdown = OpalVulkanShaderInputShutdown;

    g_OpalState.api.functions.RenderBegin           = OpalVulkanRenderBegin;
    g_OpalState.api.functions.RenderEnd             = OpalVulkanRenderEnd;
    g_OpalState.api.functions.RenderRenderpassBegin = OpalVulkanRenderRenderpassBegin;
    g_OpalState.api.functions.RenderRenderpassEnd   = OpalVulkanRenderRenderpassEnd;

  } break;
  default:
  {
    OpalLogError("Invalid Api type : %d", initInfo.api);
    return Opal_Failure_Unknown;
  }
  }

  return Opal_Success;
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

OpalResult OpalWindowSwapBuffers(const OpalWindow* pWindow)
{
  return g_OpalState.api.functions.WindowSwapBuffers(pWindow);
}

OpalResult OpalWindowGetFrameImage(const OpalWindow* pWindow, uint32_t frameIndex, OpalImage* pImage)
{
  return g_OpalState.api.functions.WindowGetFrameImage(pWindow, frameIndex, pImage);
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

void OpalRenderRenderpassBegin(const OpalRenderpass* pRenderpass, const OpalFramebuffer* pFramebuffer)
{
  g_OpalState.api.functions.RenderRenderpassBegin(pRenderpass, pFramebuffer);
}

void OpalRenderRenderpassEnd(const OpalRenderpass* pRenderpass)
{
  g_OpalState.api.functions.RenderRenderpassEnd(pRenderpass);
}



