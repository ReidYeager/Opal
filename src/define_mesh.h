
#ifndef GEM_OPAL_DEFINE_MESH_H_
#define GEM_OPAL_DEFINE_MESH_H_

#include "include/opal.h"

#include <vulkan/vulkan.h>

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

#endif // !GEM_OPAL_DEFINE_MESH_H_
