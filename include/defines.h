#ifndef OPAL_DEFINES_H
#define OPAL_DEFINES_H 1

#include "./vulkan_defines.h"

#include <stdint.h>
#include <stdbool.h>

// ============================================================
// ============================================================
//
// Primitives ==========
// Result
// Format
// MessageType
//
// Core ==========
// OpalGraphicsApi
// OpalPlatformWindowInfo
// OpalInitInfo
//
// State ==========
// OpalState
//
// ============================================================
// ============================================================


// Primitives
// ============================================================

typedef enum OpalResult
{
  Opal_Success,         // Normal execution
  Opal_Failure_Unknown, // Unknown issue
  Opal_Failure_Api      // Graphics Api function failure
} OpalResult;

typedef enum OpalFormat
{
  Opal_Format_Unknown,

  // Float R-G-B-A variants
  Opal_Format_R8 , Opal_Format_RG8 , Opal_Format_RGB8 , Opal_Format_RGBA8 ,
  Opal_Format_R16, Opal_Format_RG16, Opal_Format_RGB16, Opal_Format_RGBA16,
  Opal_Format_R32, Opal_Format_RG32, Opal_Format_RGB32, Opal_Format_RGBA32,
  Opal_Format_R64, Opal_Format_RG64, Opal_Format_RGB64, Opal_Format_RGBA64,

  // Int R-G-B-A variants
  Opal_Format_R8_I , Opal_Format_RG8_I , Opal_Format_RGB8_I , Opal_Format_RGBA8_I ,
  Opal_Format_R16_I, Opal_Format_RG16_I, Opal_Format_RGB16_I, Opal_Format_RGBA16_I,
  Opal_Format_R32_I, Opal_Format_RG32_I, Opal_Format_RGB32_I, Opal_Format_RGBA32_I,
  Opal_Format_R64_I, Opal_Format_RG64_I, Opal_Format_RGB64_I, Opal_Format_RGBA64_I,

  // Uint R-G-B-A variants
  Opal_Format_R8_U , Opal_Format_RG8_U , Opal_Format_RGB8_U , Opal_Format_RGBA8_U ,
  Opal_Format_R16_U, Opal_Format_RG16_U, Opal_Format_RGB16_U, Opal_Format_RGBA16_U,
  Opal_Format_R32_U, Opal_Format_RG32_U, Opal_Format_RGB32_U, Opal_Format_RGBA32_U,
  Opal_Format_R64_U, Opal_Format_RG64_U, Opal_Format_RGB64_U, Opal_Format_RGBA64_U,

  // Depth
  Opal_Format_D24_S8,
  Opal_Format_D32,

  // Other
  Opal_Format_Mat4x4
} OpalFormat;

typedef enum OpalMessageType
{
  Opal_Message_Info,
  Opal_Message_Error,
} OpalMessageType;

// Core
// ============================================================

typedef enum OpalGraphicsApi
{
  Opal_Api_Invalid,
  Opal_Api_Vulkan,
} OpalGraphicsApi;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
typedef struct OpalPlatformWindowInfo
{
  void* hwnd;
  void* hinstance;
} OpalPlatformWindowInfo;
#else
#error "Unsupported platform"
typedef struct OpalPlatformWindowInfo
{
  int unused;
} OpalPlatformWindowInfo;
#endif // OPAL_PLATFORM_*

typedef struct OpalInitInfo
{
  OpalGraphicsApi api;
  bool useDebug;

  OpalPlatformWindowInfo window;
} OpalInitInfo;

// State
// ============================================================

typedef struct OpalState
{
  OpalVulkanState vk;
} OpalState;

#endif // !OPAL_DEFINES_H
