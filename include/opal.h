
#ifndef GEM_OPAL_H
#define GEM_OPAL_H 1

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "include/opal_defines.h"

OpalResult OpalInit(OpalInitInfo _createInfo);
void OpalShutdown();

void OpalGetDefaultWindow(OpalWindow* _window);
void OpalWindowGetImage(OpalWindow _window, OpalImage* _outImage);
OpalResult OpalWindowReinit(OpalWindow _window);

OpalResult OpalImageInit(OpalImage* _image, OpalImageInitInfo _initInfo);
void OpalImageShutdown(OpalImage* _image);
OpalResult OpalImageResize(OpalImage _image, OpalExtent _extents);
OpalResult OpalImageFill(OpalImage _image, void* _data);

OpalResult OpalRenderpassInit(OpalRenderpass* _renderpass, OpalRenderpassInitInfo _initInfo);
void OpalRenderpassShutdown(OpalRenderpass* _renderpass);

OpalResult OpalFramebufferInit(OpalFramebuffer* _framebuffer, OpalFramebufferInitInfo _initInfo);
void OpalFramebufferShutdown(OpalFramebuffer* _framebuffer);
OpalResult OpalFramebufferReinit(OpalFramebuffer _framebuffer);

OpalResult OpalShaderInit(OpalShader* _shader, OpalShaderInitInfo _initInfo);
void OpalShaderShutdown(OpalShader* _shader);

OpalResult OpalInputSetInit(OpalInputSet* _set, OpalInputSetInitInfo _initInfo);
void OpalInputSetShutdown(OpalInputSet* _set);

OpalResult OpalMaterialInit(OpalMaterial* _material, OpalMaterialInitInfo _initInfo);
void OpalMaterialShutdown(OpalMaterial* _material);
OpalResult OpalMaterialReinit(OpalMaterial _material);

OpalResult OpalRenderBegin();
OpalResult OpalRenderEnd();
void OpalRenderBeginRenderpass(OpalRenderpass _renderpass, OpalFramebuffer _framebuffer);
void OpalRenderEndRenderpass(OpalRenderpass _renderpass);
void OpalRenderBindInputSet(OpalInputSet _set, uint32_t _setIndex);
void OpalRenderBindMaterial(OpalMaterial _material);
void OpalRenderMesh(OpalMesh _mesh);

OpalResult OpalBufferInit(OpalBuffer* _buffer, OpalBufferInitInfo _initInfo);
void OpalBufferShutdown(OpalBuffer* _buffer);
OpalResult OpalBufferPushData(OpalBuffer _buffer, void* _data);

OpalResult OpalMeshInit(OpalMesh* _mesh, OpalMeshInitInfo _initInfo);
void OpalMeshShutdown(OpalMesh* _mesh);

//void OpalRenderBindShaderArguments(); // For descriptor set input

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !GEM_OPAL_H
