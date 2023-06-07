
#include "src/common.h"

OpalResult CreateSingleShader(OpalState _state, OpalCreateShaderInfo _createInfo, OpalShader* _outShader)
{
  if (_createInfo.sourceCode == NULL || _createInfo.sourceSize == 0)
  {
    OPAL_LOG_ERROR("Invalid shader sourceCode or sourceSize\n");
    return Opal_Failure;
  }

  OpalShader_T* newShader = (OpalShader_T*)LapisMemAllocZero(sizeof(OpalShader_T));

  newShader->type = _createInfo.type;

  OPAL_ATTEMPT(_state->backend.CreateShader(_state, _createInfo, newShader),
  {
    LapisMemFree(newShader);
    return Opal_Failure_Backend;
  });

  *_outShader = newShader;
  return Opal_Success;
}

OpalResult OpalCreateShaders(
  OpalState _state,
  uint32_t _createCount,
  OpalCreateShaderInfo* _pCreateInfos,
  OpalShader* _pOutShaders)
{
  for (uint32_t i = 0; i < _createCount; i++)
  {
    OPAL_ATTEMPT(CreateSingleShader(_state, _pCreateInfos[i], &_pOutShaders[i]),
      return attemptResult);
  }

  return Opal_Success;
}

void OpalDestroyShader(OpalState _state, OpalShader* _shader)
{
  OpalShader_T* shader = *_shader;

  _state->backend.DestroyShader(_state, shader);

  LapisMemFree(shader);
  *_shader = NULL;
}

OpalResult OpalCreateMaterial(OpalState _state, OpalCreateMaterialInfo _createInfo, OpalMaterial* _outMaterial)
{
  OpalMaterial_T* newMaterial = (OpalMaterial_T*)LapisMemAllocZero(sizeof(OpalMaterial_T));

  OPAL_ATTEMPT(_state->backend.CreateMaterial(_state, _createInfo, newMaterial),
  {
    LapisMemFree(newMaterial);
    return Opal_Failure_Backend;
  });

  *_outMaterial = newMaterial;
  return Opal_Success;
}

void OpalDestroyMaterial(OpalState _state, OpalMaterial* _material)
{
  OpalMaterial_T* material = *_material;

  _state->backend.DestroyMaterial(_state, material);

  LapisMemFree(material);
  *_material = NULL;
}