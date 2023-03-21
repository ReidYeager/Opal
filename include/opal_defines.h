
#ifndef GEM_OPAL_DEFINES_H
#define GEM_OPAL_DEFINES_H

#include <lapis.h>
#include <stdint.h>

// =====
// Opal generic
// =====

typedef enum OpalResult
{
  Opal_Success,
  Opal_Backend_Failure,
  Opal_Vk_Init_Failure,
  Opal_Unknown
} OpalResult;

typedef enum OpalApi
{
  Opal_Api_Vulkan,
  Opal_Api_Unknown
} OpalApi;

// =====
// State
// =====

typedef struct OpalState_T* OpalState;

typedef struct OpalCreateStateInfo
{
  OpalApi api;
  LapisWindow window; // TODO : Allow headless when nullptr
} OpalCreateStateInfo;

#endif // !GEM_OPAL_DEFINES_H
