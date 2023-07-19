
#ifndef GEM_OPAL_H
#define GEM_OPAL_H 1

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "include/opal_defines.h"

OpalResult OpalInit(OpalInitInfo _createInfo);
void OpalShutdown();

typedef struct OpalRenderpass_T* OpalRenderpass;
OpalResult OpalRenderpassInit(OpalRenderpass* _renderpass);
void OpalRenderpassShutdown(OpalRenderpass* _renderpass);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !GEM_OPAL_H
