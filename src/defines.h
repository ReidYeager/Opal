
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
#define OPAL_LOG(type, msg, ...) \
  LapisConsolePrintMessage(      \
    type,                        \
    "Opal :: " msg,              \
    __VA_ARGS__)

#define OPAL_LOG_ERROR(msg, ...) \
  LapisConsolePrintMessage(      \
    Lapis_Console_Error,         \
    "Opal :: " msg,              \
    __VA_ARGS__)

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
// Material
// =====

typedef struct OpalShader_T
{
  OpalShaderTypes type;

  struct {
    OvkShader_T vulkan;
  } backend;
} OpalShader_T;

typedef struct OpalMaterial_T
{
  struct {
    OvkMaterial_T vulkan;
  } backend;
} OpalMaterial_T;

// =====
// State
// =====

typedef struct OpalState_T
{
  OpalApi api;

  struct {
    void* state;
    void(*ShutdownState)(OpalState _state);
    OpalResult(*CreateShader)(
      OpalState _state,
      OpalCreateShaderInfo _createInfo,
      OpalShader _shader);
    void(*DestroyShader)(OpalState _state, OpalShader _shader);
    OpalResult(*CreateMaterial)(
      OpalState _state,
      OpalCreateMaterialInfo _createInfo,
      OpalMaterial _Material);
    void(*DestroyMaterial)(OpalState _state, OpalMaterial _material);
  } backend;
} OpalState_T;

#endif // !GEM_OPAL_DEFINES_H_PRIVATE
