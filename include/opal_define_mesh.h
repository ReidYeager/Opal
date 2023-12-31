
#ifndef GEM_OPAL_DEFINE_MESH_H_
#define GEM_OPAL_DEFINE_MESH_H_

#include "include/opal_define_buffer.h"

#include <vulkan/vulkan.h>

typedef struct OpalMesh_T* OpalMesh;

typedef struct OvkMesh_T
{
  int x;
} OvkMesh_T;

typedef struct OpalMesh_T
{
  uint32_t vertCount;
  OpalBuffer vertBuffer;
  uint32_t indexCount;
  OpalBuffer indexBuffer;
} OpalMesh_T;

typedef struct OpalMeshInitInfo
{
  uint32_t vertexCount;
  const void* pVertices;
  uint32_t indexCount;
  const uint32_t* pIndices;
} OpalMeshInitInfo;

#endif // !GEM_OPAL_DEFINE_MESH_H_
