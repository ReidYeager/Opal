
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
    OPAL_LOG_ERROR("Attempt failed : Result %d : \"%s\"\n\t\"%s\" : %u\n", attemptResult, #fn, __FILE__, __LINE__); \
    __debugbreak();                     \
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
    "Opal :: "##msg,             \
    __VA_ARGS__)

#define OPAL_LOG_ERROR(msg, ...) \
  LapisConsolePrintMessage(      \
    Lapis_Console_Error,         \
    "Opal :: "##msg,              \
    __VA_ARGS__)

#define OPAL_LOG_VK(type, msg, ...) \
  LapisConsolePrintMessage(         \
    type,                           \
    "Opal :: Vk :: "##msg,           \
    __VA_ARGS__)

#define OPAL_LOG_VK_ERROR(msg, ...) \
  LapisConsolePrintMessage(         \
    Lapis_Console_Error,            \
    "Opal :: Vk :: "##msg,           \
    __VA_ARGS__)

// =====
// Buffer
// =====

typedef struct OpalBuffer_T
{
  uint64_t size;
  uint64_t paddedSize;

  struct {
    OvkBuffer_T vulkan;
  } backend;
} OpalBuffer_T;

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
// Mesh
// =====

typedef struct OpalMesh_T
{
  uint32_t vertexCount;
  OpalBuffer vertexBuffer;
  uint32_t indexCount;
  OpalBuffer indexBuffer;
} OpalMesh_T;

// =====
// State
// =====

typedef struct OpalState_T
{
  OpalApi api;
  OpalVertexLayoutInfo vertexLayout;

  struct {
    void* state;
    void(*ShutdownState)(OpalState _state);
    OpalResult(*RenderFrame)(OpalState _state, const OpalFrameData* _frameData);
    // Buffer =====
    OpalResult(*CreateBuffer)(
      OpalState _state,
      OpalCreateBufferInfo _createInfo,
      OpalBuffer _buffer );
    void(*DestroyBuffer)( OpalState _state, OpalBuffer _buffer);
    OpalResult(*BufferPushData)(OpalState _state, OpalBuffer _buffer, void* _data);
    // Material =====
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
    // Mesh =====
    OpalResult(*CreateMesh)(OpalState _state, OpalCreateMeshInfo _createInfo, OpalMesh _outMesh);
    void(*DestroyMesh)(OpalState _state, OpalMesh _mesh);
  } backend;
} OpalState_T;

#endif // !GEM_OPAL_DEFINES_H_PRIVATE
