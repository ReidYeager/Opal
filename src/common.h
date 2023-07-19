
#ifndef GEM_OPAL_LOCAL_COMMON_H_
#define GEM_OPAL_LOCAL_COMMON_H_ 1

#include "src/defines.h"

OpalResult OvkInit();
void OvkShutdown();

OpalResult OvkWindowInit(OpalWindow_T* _window);
OpalResult OvkWindowReinit(OpalWindow_T* _window);
OpalResult OvkWindowShutdown(OpalWindow_T* _window);

OpalResult OvkRenderpassInit(OpalRenderpass_T* _renderpass);
void OvkRenderpassShutdown(OpalRenderpass_T* _renderpass);

OpalResult OvkFramebufferInit();
OpalResult OvkFramebufferReinit();
void OvkFramebufferShutdown();



#endif // !GEM_OPAL_LOCAL_COMMON_H_
