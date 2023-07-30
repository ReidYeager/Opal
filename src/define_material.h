
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

typedef struct OvkMaterial_T
{
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;

  VkDescriptorSetLayout descriptorLayout;
  VkDescriptorSet descriptorSet;

  // Shaders
} OvkMaterial_T;

typedef struct OpalMaterial_T
{
  OvkMaterial_T vk;
} OpalMaterial_T;

#endif // !GEM_OPAL_DEFINE_MATERIAL_H_
