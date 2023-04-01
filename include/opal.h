
#ifndef GEM_OPAL_H
#define GEM_OPAL_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#include "include/opal_defines.h"

OpalResult OpalCreateState(OpalCreateStateInfo _info, OpalState* _outState);
void OpalDestroyState(OpalState* _state);

// TODO : Opal create/destroy window (If not headless)


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !define GEM_OPAL_H
