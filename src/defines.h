
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

  union {
    void* null;
    OvkBuffer_T vulkan;
  } backend;
} OpalBuffer_T;

// =====
// Image
// =====

typedef struct OpalImage_T
{
  uint32_t width;
  uint32_t height;

  union {
    void* null;
    OvkImage_T vulkan;
  } backend;
} OpalImage_T;

// =====
// Material
// =====

typedef struct OpalShader_T
{
  OpalShaderTypes type;

  union {
    void* null;
    OvkShader_T vulkan;
  } backend;
} OpalShader_T;

typedef struct OpalMaterial_T
{
  union {
    void* null;
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
// Rendering
// =====

typedef struct OpalRenderable_T
{
  OpalMaterial material;
  OpalMesh mesh;
  union
  {
    OvkRenderable_T vulkan;
  } backend;
} OpalRenderable_T;

// =====
// State
// =====

typedef struct OpalState_T
{
  OpalApi api;
  OpalVertexLayoutInfo vertexLayout;
  OpalObjectShaderArgumentsInfo objectShaderArgsInfo;

  struct {
    void* state;
    void(*ShutdownState)(OpalState _oState);
    OpalResult(*RenderFrame)(OpalState _oState, const OpalFrameData* _frameData);
    // Buffer =====
    OpalResult(*CreateBuffer)(
      OpalState _oState,
      OpalCreateBufferInfo _createInfo,
      OpalBuffer _oBuffer );
    void(*DestroyBuffer)(OpalState _oState, OpalBuffer _oBuffer );
    OpalResult(*BufferPushData)(OpalState _oState, OpalBuffer _oBuffer, void* _data);
    // Image =====
    OpalResult(*CreateImage)(
      OpalState _oState,
      OpalCreateImageInfo _createInfo,
      OpalImage _oImage);
    void(*DestroyImage)(OpalState _oState, OpalImage _oImage);
    // Material =====
    OpalResult(*CreateShader)(
      OpalState _oState,
      OpalCreateShaderInfo _createInfo,
      OpalShader _oShader);
    void(*DestroyShader)(OpalState _oState, OpalShader _oShader);
    OpalResult(*CreateMaterial)(
      OpalState _oState,
      OpalCreateMaterialInfo _createInfo,
      OpalMaterial _oMaterial);
    void(*DestroyMaterial)(OpalState _oState, OpalMaterial _oMaterial );
    // Mesh =====
    OpalResult(*CreateMesh)(OpalState _oState, OpalCreateMeshInfo _createInfo, OpalMesh _oMesh);
    void(*DestroyMesh)(OpalState _oState, OpalMesh _oMesh );
    // Renderable =====
    OpalResult(*CreateRenderable)(
      OpalState _oState,
      OpalShaderArg* _objectArguments,
      OpalRenderable _renderable);
  } backend;
} OpalState_T;

#endif // !GEM_OPAL_DEFINES_H_PRIVATE
