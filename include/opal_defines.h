
#ifndef GEM_OPAL_DEFINES_H
#define GEM_OPAL_DEFINES_H 1

#include <lapis.h>

typedef enum OpalResult
{
  Opal_Success = 0,
  Opal_Failure
} OpalResult;

#define OPAL_HANDLE(name) typedef struct name##_T* name

typedef struct OpalInitInfo
{
  LapisWindow* window;
} OpalInitInfo;

#endif // !GEM_OPAL_DEFINES_H
