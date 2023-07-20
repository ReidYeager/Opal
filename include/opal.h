
#ifndef GEM_OPAL_H
#define GEM_OPAL_H 1

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "include/opal_defines.h"

OpalResult OpalInit(OpalInitInfo _createInfo);
void OpalShutdown();

typedef struct OpalWindow_T* OpalWindow;
void OpalGetDefaultWindow(OpalWindow* _window);
typedef struct OpalRenderpass_T* OpalRenderpass;
OpalResult OpalRenderpassInit(OpalRenderpass* _renderpass);
void OpalWindowGetImage(OpalWindow _window, OpalImage* _outImage);

OpalResult OpalImageInit(OpalImage* _image, OpalImageInitInfo const* _initInfo);
void OpalImageShutdown(OpalImage* _image);
OpalResult OpalImageResize(OpalImage _image, OpalExtent _extents);

void OpalRenderpassShutdown(OpalRenderpass* _renderpass);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !GEM_OPAL_H
