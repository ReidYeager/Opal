
#ifndef GEM_OPAL_DEFINES_H_PRIVATE
#define GEM_OPAL_DEFINES_H_PRIVATE

#include "include/opal.h"
#include "src/vulkan/vulkan_defines.h"

#include <lapis.h>

#include <stdint.h>

// =====
// Buffer
// =====

typedef struct OpalBuffer_T
{
  uint64_t size;
  uint64_t paddedSize;

  union
  {
    void* null;
    OvkBuffer_T vulkan;
  } backend;
} OpalBuffer_T;

// =====
// Image
// =====

typedef struct OpalImage_T
{
  OpalExtents2D extents;

  union
  {
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

  union
  {
    void* null;
    OvkShader_T vulkan;
  } backend;
} OpalShader_T;

typedef struct OpalMaterial_T
{
  union
  {
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

  union
  {
    OvkMesh_T vulkan;
  } backend;
} OpalMesh_T;

// =====
// Rendering
// =====

typedef struct OpalObject_T
{
  union
  {
    OvkRenderable_T vulkan;
  } backend;
} OpalObject_T;

typedef struct OpalSubpass_T
{
  bool usesDepth;
  uint32_t colorAttachmentCount;
} OpalSubpass_T;

typedef struct OpalRenderpass_T
{
  union
  {
    OvkRenderpass_T vulkan;
  } backend;

  OpalExtents2D extents;
  OpalExtents2D offset;

  uint32_t subpassCount;
  OpalSubpass_T* pSubpasses;

  uint32_t attachmentCount;
  OpalClearValues* clearValues;
  OpalResult(*Render)();
} OpalRenderpass_T;

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

    OpalExtents2D(*GetSwapchainExtents)(OpalState _oState);

    // Buffer =====
    OpalResult(*CreateBuffer)(OpalState _oState, OpalCreateBufferInfo _createInfo, OpalBuffer _oBuffer );
    void(*DestroyBuffer)(OpalState _oState, OpalBuffer _oBuffer );
    OpalResult(*BufferPushData)(OpalState _oState, OpalBuffer _oBuffer, void* _data);

    // Image =====
    OpalResult(*CreateImage)(OpalState _oState, OpalCreateImageInfo _createInfo, OpalImage _oImage);
    void(*DestroyImage)(OpalState _oState, OpalImage _oImage);

    // Material =====
    OpalResult(*CreateShader)(OpalState _oState, OpalCreateShaderInfo _createInfo, OpalShader _oShader);
    void(*DestroyShader)(OpalState _oState, OpalShader _oShader);

    OpalResult(*CreateMaterial)(OpalState _oState, OpalCreateMaterialInfo _createInfo, OpalMaterial _oMaterial);
    void(*DestroyMaterial)(OpalState _oState, OpalMaterial _oMaterial );

    // Mesh =====
    OpalResult(*CreateMesh)(OpalState _oState, OpalCreateMeshInfo _createInfo, OpalMesh _oMesh);
    void(*DestroyMesh)(OpalState _oState, OpalMesh _oMesh );

    // Rendering =====
    OpalResult(*CreateObject)(OpalState _oState, OpalShaderArg* _objectArguments, OpalObject _oRenderable);
    OpalResult(*CreateRenderpass)(OpalState _oState, OpalCreateRenderpassInfo _createInfo, OpalRenderpass _oRenderpass);

    void(*BindMaterial)(OpalState _oState, OpalMaterial _material);
    void(*BindObject)(OpalState _oState, OpalObject _renderable);
    void(*RenderMesh)(OpalMesh _mesh);
    void(*NextSubpass)();
  } backend;
} OpalState_T;

#endif // !GEM_OPAL_DEFINES_H_PRIVATE
