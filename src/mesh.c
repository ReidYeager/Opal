
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
