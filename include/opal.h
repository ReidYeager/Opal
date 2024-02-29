#ifndef OPAL_H
#define OPAL_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "./defines.h"

// Core
// ============================================================

OpalResult OpalInit(OpalInitInfo initInfo);
OpalResult OpalWindowInit(OpalWindowInitInfo initInfo, OpalWindow* pWindow);

#ifdef __cplusplus
}
#endif

#endif // !OPAL_H
