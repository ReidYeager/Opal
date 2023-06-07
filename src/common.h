
#ifndef GEM_OPAL_COMMON_H_
#define GEM_OPAL_COMMON_H_

#include "src/defines.h"

// TODO : Define a way to change print settings when using Opal binaries (Enabled, output location)
#define OPAL_LOG(type, msg, ...) \
  LapisConsolePrintMessage(      \
    type,                        \
    "Opal :: "##msg,             \
    __VA_ARGS__)

#define OPAL_LOG_ERROR(msg, ...) \
  LapisConsolePrintMessage(      \
    Lapis_Console_Error,         \
    "*** Opal :: "##msg,         \
    __VA_ARGS__)

#if defined(_DEBUG)
#define OPAL_ATTEMPT(fn, failureAction) \
{                                       \
  OpalResult attemptResult = (fn);      \
  if (attemptResult != Opal_Success)    \
  {                                     \
    OPAL_LOG_ERROR("Attempt failed : Result %d : \"%s\"\n\t\"%s\" : %u\n", attemptResult, #fn, __FILE__, __LINE__); \
    __debugbreak();                     \
    failureAction;                      \
  }                                     \
}
#else
#define OPAL_ATTEMPT(fn, failureAction) \
{                                       \
  OpalResult attemptResult = (fn);      \
  if (attemptResult != Opal_Success)    \
  {                                     \
    OPAL_LOG_ERROR("Attempt failed : Result %d : \"%s\"\n\t\"%s\" : %u\n", attemptResult, #fn, __FILE__, __LINE__); \
    failureAction;                      \
  }                                     \
}
#endif // defined(_DEBUG)

#endif // !GEM_OPAL_COMMON_H_