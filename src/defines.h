
#ifndef GEM_OPAL_DEFINES_H_PRIVATE
#define GEM_OPAL_DEFINES_H_PRIVATE

#include "include/opal.h"
#include "src/vulkan/vulkan_defines.h"

#include <lapis.h>

#include <stdint.h>

#define OPAL_ATTEMPT(fn, failureAction) \
{                                       \
  OpalResult attemptResult = (fn);      \
  if (attemptResult != Opal_Success)    \
  {                                     \
    failureAction;                      \
  }                                     \
}

typedef struct OpalState_T
{
  struct
  {
    void* state;
  } backend;


} OpalState_T;

#endif // !GEM_OPAL_DEFINES_H_PRIVATE
