
#include "src/common.h"

OpalResult OpalCreateMesh(OpalState _state, OpalCreateMeshInfo _createInfo, OpalMesh* _outMesh)
{
  OpalMesh_T* newMesh = (OpalMesh_T*)LapisMemAllocZero(sizeof(OpalMesh_T));

  // Vertex =====
  OpalCreateBufferInfo vertBufferInfo = { 0 };
  vertBufferInfo.size = _state->vertexLayout.structSize * _createInfo.vertexCount;
  vertBufferInfo.usage = Opal_Buffer_Usage_Vertex;

  OPAL_ATTEMPT(OpalCreateBuffer(_state, vertBufferInfo, &newMesh->vertexBuffer),
  {
    LapisMemFree(newMesh);
    return Opal_Failure;
  });

  OPAL_ATTEMPT(OpalBufferPushData(_state, newMesh->vertexBuffer, _createInfo.pVertices),
  {
    LapisMemFree(newMesh);
    return Opal_Failure;
  });

  newMesh->vertexCount = _createInfo.vertexCount;

  // Index =====
  OpalCreateBufferInfo indexBufferInfo = { 0 };
  indexBufferInfo.size = sizeof(uint32_t) * _createInfo.indexCount;
  indexBufferInfo.usage = Opal_Buffer_Usage_Index;

  OPAL_ATTEMPT(OpalCreateBuffer(_state, indexBufferInfo, &newMesh->indexBuffer),
  {
    LapisMemFree(newMesh);
    return Opal_Failure;
  });

  OPAL_ATTEMPT(OpalBufferPushData(_state, newMesh->indexBuffer, _createInfo.pIndices),
  {
    LapisMemFree(newMesh);
    return Opal_Failure;
  });

  newMesh->indexCount = _createInfo.indexCount;

  *_outMesh = newMesh;
  return Opal_Success;
}

void OpalDestroyMesh(OpalState _state, OpalMesh* _mesh)
{
  OpalMesh_T* mesh = *_mesh;
  OpalDestroyBuffer(_state, &mesh->vertexBuffer);
  OpalDestroyBuffer(_state, &mesh->indexBuffer);
  LapisMemFree(mesh);
  *_mesh = NULL;
}

void DefineVertexLayout_Opal(OpalState _state, OpalCreateStateInfo _createInfo)
{
  OpalVertexLayoutInfo* stateLayout = &_state->vertexLayout;
  OpalVertexLayoutInfo* inLayout = _createInfo.pCustomVertexLayout;

  if (inLayout == NULL)
  {
    stateLayout->elementCount = 3;
    stateLayout->pElementFormats = (OpalFormat*)LapisMemAlloc(
      sizeof(OpalFormat) * stateLayout->elementCount);

    stateLayout->pElementFormats[0] = Opal_Format_Float32_3; // Vec3 Position
    stateLayout->pElementFormats[1] = Opal_Format_Float32_3; // Vec3 Normal
    stateLayout->pElementFormats[2] = Opal_Format_Float32_2; // Vec2 Uv

    stateLayout->structSize = (4 * 3) * 2 + (4 * 2);

    return;
  }

  stateLayout->structSize = inLayout->structSize;
  if (stateLayout->structSize == 0)
  {
    for (uint32_t i = 0; i < inLayout->elementCount; i++)
    {
      stateLayout->structSize += OpalFormatToSize(inLayout->pElementFormats[i]);
    }
  }

  stateLayout->elementCount = inLayout->elementCount;
  stateLayout->pElementFormats = LapisMemAllocArray(OpalFormat, stateLayout->elementCount);
  LapisMemCopy(inLayout->pElementFormats, stateLayout->pElementFormats, sizeof(OpalFormat) * stateLayout->elementCount);
}
