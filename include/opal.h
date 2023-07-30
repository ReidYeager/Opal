
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
OpalResult OpalWindowResize(OpalWindow _window, uint32_t _width, uint32_t _height);

OpalResult OpalImageInit(OpalImage* _image, OpalImageInitInfo const* _initInfo);
void OpalImageShutdown(OpalImage* _image);
OpalResult OpalImageResize(OpalImage _image, OpalExtent _extents);

OpalResult OpalRenderpassInit(OpalRenderpass* _renderpass, OpalRenderpassInitInfo _initInfo);
void OpalRenderpassShutdown(OpalRenderpass* _renderpass);

OpalResult OpalFramebufferInit(OpalFramebuffer* _framebuffer, OpalFramebufferInitInfo const* _initInfo);
void OpalFramebufferShutdown(OpalFramebuffer* _framebuffer);

OpalResult OpalShaderInit(OpalShader* _shader, OpalShaderInitInfo _initInfo);
void OpalShaderShutdown(OpalShader* _shader);

OpalResult OpalMaterialInit(OpalMaterial* _material, OpalMaterialInitInfo _initInfo);
void OpalMaterialShutdown(OpalMaterial* _material);

OpalResult OpalRenderBegin();
OpalResult OpalRenderEnd();
void OpalRenderBeginRenderpass(OpalRenderpass _renderpass, OpalFramebuffer _framebuffer);
void OpalRenderEndRenderpass(OpalRenderpass _renderpass);
void OpalRenderBindMaterial(OpalMaterial _material);
void OpalRenderVertices(uint32_t _count); // Tmp. Replace with mesh render

//void OpalRenderBindShaderArguments(); // For descriptor set input

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !GEM_OPAL_H
