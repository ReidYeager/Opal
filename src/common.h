
#ifndef GEM_OPAL_LOCAL_COMMON_H
#define GEM_OPAL_LOCAL_COMMON_H 1

#include "src/defines.h"

OpalResult OvkInit();
void OvkShutdown();

OpalResult OvkWindowInit(OpalWindow_T* _window);
OpalResult OvkWindowReinit(OpalWindow_T* _window);
OpalResult OvkWindowShutdown(OpalWindow_T* _window);


#endif // !GEM_OPAL_LOCAL_COMMON_H
