
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

OpalResult OpalImageInit(OpalImage* _image, OpalImageInitInfo const* _initInfo);
void OpalImageShutdown(OpalImage* _image);
OpalResult OpalImageResize(OpalImage _image, OpalExtent _extents);

OpalResult OpalRenderpassInit(OpalRenderpass* _renderpass);
void OpalRenderpassShutdown(OpalRenderpass* _renderpass);

OpalResult OpalFramebufferInit(OpalFramebuffer* _framebuffer, OpalFramebufferInitInfo const* _initInfo);
void OpalFramebufferShutdown(OpalFramebuffer* _framebuffer);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !GEM_OPAL_H
