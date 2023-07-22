
#ifndef GEM_OPAL_DEFINES_H
#define GEM_OPAL_DEFINES_H 1

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "include/opal_define_image.h"
#include <lapis.h>

typedef enum OpalResult
{
  Opal_Success = 0,
  Opal_Failure
} OpalResult;

#define DEFINE_OPAL_HANDLE(name) typedef struct name##_T* name

DEFINE_OPAL_HANDLE(OpalWindow);
DEFINE_OPAL_HANDLE(OpalRenderpass);
DEFINE_OPAL_HANDLE(OpalFramebuffer);

typedef struct OpalFramebufferInitInfo
{
  OpalRenderpass renderpass;
} OpalFramebufferInitInfo;


typedef struct OpalInitInfo
{
  LapisWindow* window;
} OpalInitInfo;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !GEM_OPAL_DEFINES_H
