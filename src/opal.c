
#include "src/common.h"

#include <vulkan/vulkan.h>

#include <stdio.h>

OpalState_T oState = { 0 };

// Size in bytes
uint32_t OpalFormatToSize_Ovk(OpalFormat _format)
{
  switch (_format)
  {
    // R G B A variants
  case Opal_Format_R8:     case Opal_Format_R8I:     case Opal_Format_R8U:     case Opal_Format_R8_Nonlinear:
    return 1;
  case Opal_Format_RG8:    case Opal_Format_RG8I:    case Opal_Format_RG8U:    case Opal_Format_RG8_Nonlinear:
    return 2;
  case Opal_Format_RGB8:   case Opal_Format_RGB8I:   case Opal_Format_RGB8U:   case Opal_Format_RGB8_Nonlinear:
    return 3;
  case Opal_Format_RGBA8:  case Opal_Format_RGBA8I:  case Opal_Format_RGBA8U:  case Opal_Format_RGBA8_Nonlinear:
    return 4;
  case Opal_Format_R32:    case Opal_Format_R32I:    case Opal_Format_R32U:
    return 4;
  case Opal_Format_RG32:   case Opal_Format_RG32I:   case Opal_Format_RG32U:
    return 8;
  case Opal_Format_RGB32:  case Opal_Format_RGB32I:  case Opal_Format_RGB32U:
    return 12;
  case Opal_Format_RGBA32: case Opal_Format_RGBA32I: case Opal_Format_RGBA32U:
    return 16;
  case Opal_Format_R64:    case Opal_Format_R64I:    case Opal_Format_R64U:
    return 8;
  case Opal_Format_RG64:   case Opal_Format_RG64I:   case Opal_Format_RG64U:
    return 16;
  case Opal_Format_RGB64:  case Opal_Format_RGB64I:  case Opal_Format_RGB64U:
    return 24;
  case Opal_Format_RGBA64: case Opal_Format_RGBA64I: case Opal_Format_RGBA64U:
    return 32;

    // B G R A variants
  case Opal_Format_BGR8:
    return 3;
  case Opal_Format_BGRA8:
    return 4;
  case Opal_Format_BGR32:
    return 12;
  case Opal_Format_BGRA32:
    return 16;

    // Depth stencils
  case Opal_Format_D24_S8: case Opal_Format_D16_S8: case Opal_Format_D32:
    return 4;

    // Other
  case Opal_Format_Mat4x4:
    return 64;
  default:
    return 0;
  }
}

OpalResult OpalInit(OpalInitInfo _initInfo)
{
  if (oState.vertexFormat.pFormats)
  {
    OpalLog("Failed to init Opal. Already in use.\n");
    return Opal_Failure;
  }

  oState.vertexFormat.structSize = 0;
  for (uint32_t i = 0; i < _initInfo.vertexStruct.count; i++)
  {
    oState.vertexFormat.structSize += OpalFormatToSize_Ovk(_initInfo.vertexStruct.pFormats[i]);
  }
  oState.vertexFormat.attribCount = _initInfo.vertexStruct.count;
  oState.vertexFormat.pFormats = LapisMemAllocZeroArray(OpalFormat, _initInfo.vertexStruct.count);
  LapisMemCopy(_initInfo.vertexStruct.pFormats, oState.vertexFormat.pFormats, sizeof(OpalFormat) * _initInfo.vertexStruct.count);

  OPAL_ATTEMPT(OvkInit(_initInfo));

  return Opal_Success;
}

void OpalShutdown()
{
  OvkShutdown();
}

OpalResult OpalWindowInit(OpalWindow* _outWindow, OpalWindowInitInfo _initInfo)
{
  OpalLog("Creating new window\n");
  OpalWindow_T* newWindow = LapisMemAllocZeroSingle(OpalWindow_T);

  newWindow->platform = _initInfo.platformInfo;

  OPAL_ATTEMPT(OvkWindowInit(newWindow, _initInfo), LapisMemFree(newWindow));

  *_outWindow = newWindow;
  return Opal_Success;
}

void OpalWindowShutdown(OpalWindow* _window)
{
  OvkWindowShutdown(*_window);
  LapisMemFree(*_window);
  *_window = NULL;
}

void OpalWindowGetBufferImage(OpalWindow _window, OpalImage* _outImage)
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
  newRenderpass->pAttachments = LapisMemAllocZeroArray(OpalAttachmentInfo, _initInfo.imageCount);
  LapisMemCopy(_initInfo.pAttachments, newRenderpass->pAttachments, sizeof(OpalAttachmentInfo) * _initInfo.imageCount);
  newRenderpass->pSubpasses = LapisMemAllocZeroArray(OpalSubpassInfo, _initInfo.imageCount);
  LapisMemCopy(_initInfo.pSubpasses, newRenderpass->pSubpasses, sizeof(OpalSubpassInfo) * _initInfo.imageCount);

  OpalLog("Renderpass init complete\n");
  *_renderpass = newRenderpass;
  return Opal_Success;
}

void OpalRenderpassShutdown(OpalRenderpass* _renderpass)
{
  OvkRenderpassShutdown(*_renderpass);
  LapisMemFree(*_renderpass);
  *_renderpass = OPAL_NULL_HANDLE;
  OpalLog("Renderpass shutdown complete\n");
}

bool CompareExtents_Opal(OpalExtent _a, OpalExtent _b)
{
  return _a.width == _b.width
    && _a.height == _b.height
    && ((_a.depth <= 1 && _b.depth <= 1) || _a.depth == _b.depth);
}

OpalResult OpalFramebufferInit(OpalFramebuffer* _framebuffer, OpalFramebufferInitInfo _initInfo)
{
  OpalFramebuffer_T* newFramebuffer = LapisMemAllocZeroSingle(OpalFramebuffer_T);

  OPAL_ATTEMPT(OvkFramebufferInit(newFramebuffer, _initInfo));

  newFramebuffer->ownerRenderpass = _initInfo.renderpass;
  newFramebuffer->extent = _initInfo.pImages[0]->extents;
  newFramebuffer->imageCount = _initInfo.imageCount;
  newFramebuffer->ppImages = LapisMemAllocArray(OpalImage_T*, _initInfo.imageCount);
  for (uint32_t i = 0; i < _initInfo.imageCount; i++)
  {
    if (!CompareExtents_Opal(newFramebuffer->extent, _initInfo.pImages[i]->extents))
    {
      OpalLogError(
        "All framebuffer input images must have matching extents : (%u, %u, %u)\n",
        newFramebuffer->extent.width,
        newFramebuffer->extent.height,
        newFramebuffer->extent.depth);
      return Opal_Failure;
    }

    newFramebuffer->ppImages[i] = _initInfo.pImages[i];
  }

  OpalLog("Framebuffer init complete\n");
  *_framebuffer = newFramebuffer;
  return Opal_Success;
}

void OpalFramebufferShutdown(OpalFramebuffer* _framebuffer)
{
  OvkFramebufferShutdown(*_framebuffer);
  LapisMemFree((*_framebuffer)->ppImages);
  LapisMemFree(*_framebuffer);
  *_framebuffer = OPAL_NULL_HANDLE;
  OpalLog("Framebuffer shutdown complete\n");
}

OpalResult OpalFramebufferReinit(OpalFramebuffer _framebuffer)
{
  OPAL_ATTEMPT(OvkFramebufferReinit(_framebuffer));

  _framebuffer->extent = _framebuffer->ppImages[0]->extents;

  return Opal_Success;
}

OpalResult OpalShaderInit(OpalShader* _shader, OpalShaderInitInfo _initInfo)
{
  OpalShader_T* newShader = *_shader;

  if (newShader == NULL)
  {
    newShader = LapisMemAllocZeroSingle(OpalShader_T);
  }

  OPAL_ATTEMPT(OvkShaderInit(newShader, _initInfo), LapisMemFree(newShader));

  OpalLog("Shader init complete\n");
  *_shader = newShader;
  return Opal_Success;
}

void OpalShaderShutdown(OpalShader* _shader)
{
  OvkShaderShutdown(*_shader);
  LapisMemFree(*_shader);
  *_shader = OPAL_NULL_HANDLE;
  OpalLog("Shader shutdown complete\n");
}

OpalResult OpalInputLayoutInit(OpalInputLayout* _layout, OpalInputLayoutInitInfo _initInfo)
{
  OpalInputLayout_T* newLayout = LapisMemAllocZeroSingle(OpalInputLayout_T);

  OPAL_ATTEMPT(OvkInputLayoutInit(newLayout, _initInfo), LapisMemFree(newLayout));

  newLayout->count = _initInfo.count;
  newLayout->pTypes = LapisMemAllocZeroArray(OpalInputType, _initInfo.count);
  LapisMemCopy(_initInfo.pTypes, newLayout->pTypes, sizeof(OpalInputType) * _initInfo.count);

  *_layout = newLayout;
  return Opal_Success;
}

OpalResult OpalInputSetInit(OpalInputSet* _set, OpalInputSetInitInfo _initInfo)
{
  OpalInputSet_T* newSet = LapisMemAllocZeroSingle(OpalInputSet_T);

  OPAL_ATTEMPT(OvkInputSetInit(newSet, _initInfo), LapisMemFree(newSet));

  *_set = newSet;
  return Opal_Success;
}

void OpalInputLayoutShutdown(OpalInputLayout* _layout)
{
  OvkInputLayoutShutdown(*_layout);
  LapisMemFree(*_layout);
  *_layout = OPAL_NULL_HANDLE;
}

void OpalInputSetShutdown(OpalInputSet* _set)
{
  OvkInputSetShutdown(*_set);
  LapisMemFree(*_set);
  *_set = OPAL_NULL_HANDLE;
}

OpalResult OpalInputSetUpdate(OpalInputSet _set, uint32_t _count, OpalInputInfo* _pInputs)
{
  OPAL_ATTEMPT(OvkInputSetUpdate(_set, _count, _pInputs));
  return Opal_Success;
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

  newMaterial->pushConstantSize = _initInfo.pushConstantSize;

  OpalLog("Material init complete\n");
  *_material = newMaterial;
  return Opal_Success;
}

void OpalMaterialShutdown(OpalMaterial* _material)
{
  OvkMaterialShutdown(*_material);
  LapisMemFree(*_material);
  *_material = OPAL_NULL_HANDLE;
  OpalLog("Material shutdown complete\n");
}

OpalResult OpalMaterialReinit(OpalMaterial _material)
{
  OPAL_ATTEMPT(OvkMaterialReinit(_material));
  return Opal_Success;
}

OpalResult OpalRenderBegin(OpalWindow _window)
{
  OPAL_ATTEMPT(OvkRenderBegin(_window));
  return Opal_Success;
}

OpalResult OpalRenderEnd()
{
  OPAL_ATTEMPT(OvkRenderEnd());
  return Opal_Success;
}

VkCommandBuffer OpalRenderGetCommandBuffer()
{
  return OvkRenderGetCommandBuffer();
}

void OpalRenderBeginRenderpass(OpalRenderpass _renderpass, OpalFramebuffer _framebuffer)
{
  OvkRenderBeginRenderpass(_renderpass, _framebuffer);
}

void OpalRenderEndRenderpass(OpalRenderpass _renderpass)
{
  OvkRenderEndRenderpass(_renderpass);
}

void OpalRenderBindInputSet(OpalInputSet _set, uint32_t _setIndex)
{
  OvkRenderBindInputSet(_set, _setIndex);
}

void OpalRenderBindMaterial(OpalMaterial _material)
{
  OvkRenderBindMaterial(_material);
}

void OpalRenderMesh(OpalMesh _mesh)
{
  OvkRenderMesh(_mesh);
}

void OpalRenderSetPushConstant(void* _data)
{
  OvkRenderSetPushConstant(_data);
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
  *_buffer = OPAL_NULL_HANDLE;
  OpalLog("Buffer shutdown complete\n");
}

OpalResult OpalBufferPushData(OpalBuffer _buffer, void* _data)
{
  OPAL_ATTEMPT(OvkBufferPushData(_buffer, _data));
  return Opal_Success;
}

OpalResult OpalBufferPushDataSegment(OpalBuffer _buffer, void* _data, uint32_t size, uint32_t offset)
{
  OPAL_ATTEMPT(OvkBufferPushDataSegment(_buffer, _data, size, offset));
  return Opal_Success;
}

OpalResult OpalMeshInit(OpalMesh* _mesh, OpalMeshInitInfo _initInfo)
{
  OpalMesh_T* newMesh = LapisMemAllocZeroSingle(OpalMesh_T);

  OpalBufferInitInfo info = { 0 };
  info.size = _initInfo.vertexCount * oState.vertexFormat.structSize;
  info.usage = Opal_Buffer_Usage_Vertex;

  OPAL_ATTEMPT(OpalBufferInit(&newMesh->vertBuffer, info), LapisMemFree(newMesh));

  info.size = _initInfo.indexCount * sizeof(_initInfo.pIndices[0]);
  info.usage = Opal_Buffer_Usage_Index;

  OPAL_ATTEMPT(OpalBufferInit(&newMesh->indexBuffer, info), LapisMemFree(newMesh));

  OPAL_ATTEMPT(OpalBufferPushData(newMesh->vertBuffer, _initInfo.pVertices), LapisMemFree(newMesh));
  OPAL_ATTEMPT(OpalBufferPushData(newMesh->indexBuffer, _initInfo.pIndices), LapisMemFree(newMesh));

  newMesh->vertCount = _initInfo.vertexCount;
  newMesh->indexCount = _initInfo.indexCount;

  *_mesh = newMesh;
  return Opal_Success;
}

void OpalMeshShutdown(OpalMesh* _mesh)
{
  OpalBufferShutdown(&(*_mesh)->vertBuffer);
  OpalBufferShutdown(&(*_mesh)->indexBuffer);

  LapisMemFree(*_mesh);
  *_mesh = OPAL_NULL_HANDLE;
}
