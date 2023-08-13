
#include "src/common.h"

#include <vulkan/vulkan.h>

#include <stdio.h>

OpalState_T oState = { 0 };

OpalResult OpalInit(OpalInitInfo _initInfo)
{
  if (oState.window.lWindow)
  {
    OpalLog("Failed to init Opal. Already in use.\n");
    return Opal_Failure;
  }

  oState.window.lWindow = _initInfo.window;
  oState.vk.allocator = NULL;

  OPAL_ATTEMPT(OvkInit(_initInfo));

  OPAL_ATTEMPT(OvkWindowInit(&oState.window));

  //OPAL_ATTEMPT(OvkWindowReinit(&oState.window));

  return Opal_Success;
}

void OpalShutdown()
{
  vkDeviceWaitIdle(oState.vk.device);

  OvkWindowShutdown(&oState.window);
  OvkShutdown();
}

void OpalGetDefaultWindow(OpalWindow* _window)
{
  *_window = &oState.window;
}

void OpalWindowGetImage(OpalWindow _window, OpalImage* _outImage)
{
  *_outImage = _window->renderBufferImage;
}

OpalResult OpalWindowReinit(OpalWindow _window)
{
  OPAL_ATTEMPT(OvkWindowReinit(_window));

  OpalLog("Window resized to %04u, %04u\r", _window->extents.width, _window->extents.height);
  return Opal_Success;
}

OpalResult OpalRenderpassInit(OpalRenderpass* _renderpass, OpalRenderpassInitInfo _initInfo)
{
  OpalRenderpass_T* newRenderpass = LapisMemAllocZeroSingle(OpalRenderpass_T);

  OPAL_ATTEMPT(OvkRenderpassInit(newRenderpass, _initInfo), LapisMemFree(newRenderpass););

  newRenderpass->imageCount = _initInfo.imageCount;
  newRenderpass->pClearValues = LapisMemAllocZeroArray(OpalClearValue, _initInfo.imageCount);
  LapisMemCopy(_initInfo.pClearValues, newRenderpass->pClearValues, sizeof(OpalClearValue) * _initInfo.imageCount);

  OpalLog("Renderpass init complete\n");
  *_renderpass = newRenderpass;
  return Opal_Success;
}

void OpalRenderpassShutdown(OpalRenderpass* _renderpass)
{
  OvkRenderpassShutdown(*_renderpass);
  LapisMemFree(*_renderpass);
  *_renderpass = NULL;
  OpalLog("Renderpass shutdown complete\n");
}

OpalResult OpalFramebufferInit(OpalFramebuffer* _framebuffer, OpalFramebufferInitInfo _initInfo)
{
  OpalFramebuffer_T* newFramebuffer = LapisMemAllocZeroSingle(OpalFramebuffer_T);

  OPAL_ATTEMPT(OvkFramebufferInit(newFramebuffer, _initInfo));

  newFramebuffer->ppImages = NULL;
  newFramebuffer->extent = oState.window.extents;
  newFramebuffer->ownerRenderpass = _initInfo.renderpass;

  OpalLog("Framebuffer init complete\n");
  *_framebuffer = newFramebuffer;
  return Opal_Success;
}

void OpalFramebufferShutdown(OpalFramebuffer* _framebuffer)
{
  OvkFramebufferShutdown(*_framebuffer);
  LapisMemFree(*_framebuffer);
  *_framebuffer = NULL;
  OpalLog("Framebuffer shutdown complete\n");
}

OpalResult OpalFramebufferReinit(OpalFramebuffer _framebuffer)
{
  OPAL_ATTEMPT(OvkFramebufferReinit(_framebuffer));

  _framebuffer->extent = oState.window.extents;

  return Opal_Success;
}

OpalResult OpalShaderInit(OpalShader* _shader, OpalShaderInitInfo _initInfo)
{
  OpalShader_T* newShader = LapisMemAllocZeroSingle(OpalShader_T);

  OPAL_ATTEMPT(OvkShaderInit(newShader, _initInfo), LapisMemFree(newShader));

  OpalLog("Shader init complete\n");
  *_shader = newShader;
  return Opal_Success;
}

void OpalShaderShutdown(OpalShader* _shader)
{
  OvkShaderShutdown(*_shader);
  LapisMemFree(*_shader);
  *_shader = NULL;
  OpalLog("Shader shutdown complete\n");
}

OpalResult OpalMaterialInit(OpalMaterial* _material, OpalMaterialInitInfo _initInfo)
{
  OpalMaterial_T* newMaterial = LapisMemAllocZeroSingle(OpalMaterial_T);

  OPAL_ATTEMPT(OvkMaterialInit(newMaterial, _initInfo), LapisMemFree(newMaterial));

  newMaterial->ownerRenderpass = _initInfo.renderpass;
  newMaterial->subpassIndex = _initInfo.subpassIndex;
  newMaterial->shaderCount = _initInfo.shaderCount;
  newMaterial->pShaders = LapisMemAllocZeroArray(OpalShader, _initInfo.shaderCount);
  LapisMemCopy(_initInfo.pShaders, newMaterial->pShaders, sizeof(OpalShader) * _initInfo.shaderCount);

  OpalLog("Material init complete\n");
  *_material = newMaterial;
  return Opal_Success;
}

void OpalMaterialShutdown(OpalMaterial* _material)
{
  OvkMaterialShutdown(*_material);
  LapisMemFree(*_material);
  *_material = NULL;
  OpalLog("Material shutdown complete\n");
}

OpalResult OpalMaterialReinit(OpalMaterial _material)
{
  OPAL_ATTEMPT(OvkMaterialReinit(_material));
  return Opal_Success;
}

OpalResult OpalMaterialDefineInputs(OpalMaterial _material, uint32_t _count, OpalMaterialInputInfo* _pInputs)
{
  
  return Opal_Success;
}

OpalResult OpalRenderBegin()
{
  OPAL_ATTEMPT(OvkRenderBegin());
  return Opal_Success;
}

OpalResult OpalRenderEnd()
{
  OPAL_ATTEMPT(OvkRenderEnd());
  return Opal_Success;
}

void OpalRenderBeginRenderpass(OpalRenderpass _renderpass, OpalFramebuffer _framebuffer)
{
  OvkRenderBeginRenderpass(_renderpass, _framebuffer);
}

void OpalRenderEndRenderpass(OpalRenderpass _renderpass)
{
  OvkRenderEndRenderpass(_renderpass);
}

void OpalRenderBindMaterial(OpalMaterial _material)
{
  OvkRenderBindMaterial(_material);
}

void OpalRenderVertices(uint32_t _count)
{
  OvkRenderVertices(_count);
}

OpalResult OpalBufferInit(OpalBuffer* _buffer, OpalBufferInitInfo _initInfo)
{
  OpalBuffer_T* newBuffer = LapisMemAllocSingle(OpalBuffer_T);

  OPAL_ATTEMPT(OvkBufferInit(newBuffer, _initInfo), LapisMemFree(newBuffer));

  newBuffer->size = _initInfo.size;

  OpalLog("Buffer init complete : %llu (%llu) bytes\n", newBuffer->size, newBuffer->paddedSize);
  *_buffer = newBuffer;
  return Opal_Success;
}

void OpalBufferShutdown(OpalBuffer* _buffer)
{
  OvkBufferShutdown(*_buffer);
  LapisMemFree(*_buffer);
  *_buffer = NULL;
  OpalLog("Buffer shutdown complete\n");
}

OpalResult OpalBufferPushData(OpalBuffer _buffer, void* _data)
{
  OPAL_ATTEMPT(OvkBufferPushData(_buffer, _data));
  return Opal_Success;
}

