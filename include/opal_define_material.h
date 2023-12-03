
#ifndef GEM_OPAL_DEFINE_MATERIAL_H_
#define GEM_OPAL_DEFINE_MATERIAL_H_

#include "include/opal_define_renderpass.h"

#include <vulkan/vulkan.h>

typedef struct OpalShader_T*      OpalShader;
typedef struct OpalInputLayout_T* OpalInputLayout;
typedef struct OpalInputSet_T*    OpalInputSet;
typedef struct OpalMaterial_T*    OpalMaterial;

typedef enum OpalShaderType
{
  Opal_Shader_Vertex,
  Opal_Shader_Fragment,
  //Opal_Shader_Compute, // Not supported yet
  //Opal_Shader_Geometry // Not supported yet
  Opal_Shader_COUNT
} OpalShaderType;

typedef enum OpalInputType
{
  Opal_Input_Type_Uniform_Buffer,
  Opal_Input_Type_Samped_Image,
  Opal_Input_Type_Subpass_Input,
  Opal_Input_Type_COUNT
} OpalInputType;

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

typedef struct OvkInputLayout_T
{
  VkDescriptorSetLayout descriptorLayout;
} OvkInputLayout_T;

typedef struct OvkInputSet_T
{
  VkDescriptorSet descriptorSet;
} OvkInputSet_T;

typedef struct OvkMaterial_T
{
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;

  uint32_t inputSetCount;
  OpalInputSet* pInputSets;
} OvkMaterial_T;

typedef struct OpalInputLayout_T
{
  uint32_t count;
  OpalInputType* pTypes;
  OvkInputLayout_T vk;
} OpalInputLayout_T;

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

  uint32_t pushConstantSize;

  OvkMaterial_T vk;
} OpalMaterial_T;

typedef struct OpalShaderInitInfo
{
  char* pSource;
  uint32_t size;
  OpalShaderType type;
} OpalShaderInitInfo;

typedef union OpalMaterialInputValue
{
  OpalBuffer buffer;
  OpalImage image;
  struct {
    uint32_t attachmentIndex;
    OpalImage image;
  } inputAttachment;
} OpalMaterialInputValue;

typedef struct OpalInputLayoutInitInfo
{
  uint32_t count;
  OpalInputType* pTypes;
} OpalInputLayoutInitInfo;

typedef struct OpalInputSetInitInfo
{
  OpalInputLayout layout;
  OpalMaterialInputValue* pInputValues;
} OpalInputSetInitInfo;

typedef struct OpalInputInfo
{
  uint32_t index;
  OpalInputType type;
  OpalMaterialInputValue value;
} OpalInputInfo;

typedef struct OpalMaterialInitInfo
{
  OpalRenderpass renderpass;
  uint32_t subpassIndex;

  uint32_t shaderCount;
  OpalShader* pShaders;

  uint32_t inputLayoutCount;
  OpalInputLayout* pInputLayouts;

  uint32_t pushConstantSize;
} OpalMaterialInitInfo;

#endif // !GEM_OPAL_DEFINE_MATERIAL_H_
