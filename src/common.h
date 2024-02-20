#ifndef OPAL_COMMON_H
#define OPAL_COMMON_H 1

#include "include/defines.h"

// ============================================================
// ============================================================
//
// Global variables ==========
// g_OpalState
//
// Preprocessor ==========
// OPAL_PLATFORM_*
// OpalMem*
// OpalLog
// OpalLogError
// OPAL_ATTEMPT
// OPAL_ATTEMPT_FAIL_LOG
//
// Logging ==========
// OpalOutputMessage()
//
// Core ==========
// OpalVulkanInit()
//
// ============================================================
// ============================================================


// Global variables
// ===========================================================

extern OpalState g_OpalState;

// Preprocessor
// ============================================================

// Platform

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define OPAL_PLATFORM_WIN32 1
#ifndef _WIN64
#error "Must have 64-bit windows"
#endif
#else
#error "Unsupported platform"
#endif // OPAL_PLATFORM_*

// Memory

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define OpalMemAlloc(size)                           malloc(size)
#define OpalMemAllocZeroed(size)                     memset(malloc(size), 0, size)
#define OpalMemRealloc(data, size)                   realloc(data, size)
#define OpalMemSet(data, value, size)                memset(data, value, size)
#define OpalMemCopy(src, dst, size)                  memcpy(dst, src, size)
#define OpalMemFree(data)                            free((void*)data)
#define OpalMemAllocSingle(type)                    (type*)OpalMemAlloc(sizeof(type))
#define OpalMemAllocSingleZeroed(type)              (type*)OpalMemAllocZeroed(sizeof(type))
#define OpalMemReallocSingle(source, type)          (type*)OpalMemRealloc(source, sizeof(type))
#define OpalMemAllocArray(type, count)              (type*)OpalMemAlloc(sizeof(type) * count)
#define OpalMemAllocArrayZeroed(type, count)        (type*)OpalMemAllocZeroed(sizeof(type) * count)
#define OpalMemReallocArray(source, type, newCount) (type*)OpalMemRealloc(source, sizeof(type) * newCount)

// Logging

#define OpalLog(...) OpalOutputMessage(Opal_Message_Info, __VA_ARGS__)
#define OpalLogError(...) OpalOutputMessage(Opal_Message_Error, __VA_ARGS__)

#define OPAL_ATTEMPT(fn, ...)                                        \
{                                                                    \
  OpalResult oResult = (fn);                                         \
  if (oResult)                                                       \
  {                                                                  \
    OpalLogError("Function \""#fn"\" failed. Result = %d", oResult); \
    OpalLogError("    %s:%d\n", __FILE__, __LINE__);                 \
    { __VA_ARGS__; }                                                 \
    return Opal_Failure_Unknown;                                     \
  }                                                                  \
  else {}                                                            \
}

#define OPAL_ATTEMPT_FAIL_LOG(message, ...)        \
{                                                  \
  OpalLogError(message, __VA_ARGS__);              \
  OpalLogError("    %s:%d\n", __FILE__, __LINE__); \
}

// Logging
// ============================================================

void OpalOutputMessage(OpalMessageType type, const char* message, ...);

// Core
// ============================================================

OpalResult OpalVulkanInit(OpalInitInfo initInfo); // Defined in vulkan_core.c

#endif // !OPAL_COMMON_H

