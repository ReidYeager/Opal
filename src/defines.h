
#ifndef GEM_OPAL_DEFINES_H_PRIVATE
#define GEM_OPAL_DEFINES_H_PRIVATE

#include "include/opal.h"
#include "src/vulkan/vulkan_defines.h"

#include <lapis.h>

#include <stdint.h>

// =====
// Opal Generic
// =====

#define OPAL_ATTEMPT(fn, failureAction) \
{                                       \
  OpalResult attemptResult = (fn);      \
  if (attemptResult != Opal_Success)    \
  {                                     \
    failureAction;                      \
  }                                     \
}

// =====
// Console
// =====

// TODO : Define a way to change print settings when using Opal binaries (Enabled, output location)
#define OPAL_LOG_VK(type, msg, ...) \
  LapisConsolePrintMessage(         \
    type,                           \
    "Opal :: Vk :: " msg,           \
    __VA_ARGS__)

#define OPAL_LOG_VK_ERROR(msg, ...) \
  LapisConsolePrintMessage(         \
    Lapis_Console_Error,            \
    "Opal :: Vk :: " msg,           \
    __VA_ARGS__)

// =====
// State
// =====

typedef struct OpalState_T
{
  OpalApi api;

  struct
  {
    void* state;
    void(*ShutdownState)(OpalState _state);
  } backend;
} OpalState_T;

#endif // !GEM_OPAL_DEFINES_H_PRIVATE
