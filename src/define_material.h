
#ifndef GEM_OPAL_DEFINE_MATERIAL_H_
#define GEM_OPAL_DEFINE_MATERIAL_H_

#include "include/opal.h"

#include <vulkan/vulkan.h>

typedef struct OvkShader_T
{
  VkShaderModule module;
  VkShaderStageFlagBits stage;
} OvkShader_T;

typedef struct OpalShader_T
{
  OpalShaderType type;
  OvkShader_T vk;
} OpalShader_T;

typedef struct OvkInputSet_T
{
  VkDescriptorSetLayout descriptorLayout;
  VkDescriptorSet descriptorSet;
} OvkInputSet_T;

typedef struct OvkMaterial_T
{
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;

  uint32_t inputSetCount;
  OpalInputSet* pInputSets;
} OvkMaterial_T;

typedef struct OpalInputSet_T
{
  OvkInputSet_T vk;
} OpalInputSet_T;

typedef struct OpalMaterial_T
{
  uint32_t shaderCount;
  OpalShader* pShaders;

  OpalRenderpass ownerRenderpass;
  uint32_t subpassIndex;

  //uint32_t inputCount;
  //OpalInputInfo* pInputs;

  OvkMaterial_T vk;
} OpalMaterial_T;

#endif // !GEM_OPAL_DEFINE_MATERIAL_H_
