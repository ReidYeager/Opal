
#include "src/defines.h"
#include "src/vulkan/vulkan.h"

#include <vulkan/vulkan.h>
#include <lapis.h>

OpalResult OvkCreateShader(OpalState _oState, OpalCreateShaderInfo _createInfo, OpalShader _oShader)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;

  VkShaderModuleCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.codeSize = _createInfo.sourceSize;
  createInfo.pCode = (uint32_t*)_createInfo.sourceCode;

  OVK_ATTEMPT(
    vkCreateShaderModule(state->device, &createInfo, NULL, &_oShader->backend.vulkan.module),
    return Opal_Failure_Vk_Create);

  return Opal_Success;
}

void OvkDestroyShader(OpalState _oState, OpalShader _oShader)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;
  vkDestroyShaderModule(state->device, _oShader->backend.vulkan.module, NULL);
}

OpalResult CreateDescriptorSetLayout(
  OvkState_T* _state,
  OpalCreateMaterialInfo _createInfo,
  OvkMaterial_T* _material)
{
  VkDescriptorSetLayoutBinding* bindings =
    (VkDescriptorSetLayoutBinding*)LapisMemAllocZero(
      sizeof(VkDescriptorSetLayoutBinding) * _createInfo.shaderArgCount);
  VkDescriptorSetLayoutBinding newBinding = { 0 };
  newBinding.descriptorCount = 1;
  newBinding.pImmutableSamplers = NULL;
  newBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

  for (uint32_t i = 0; i < _createInfo.shaderArgCount; i++)
  {
    OpalShaderArg argI = _createInfo.pShaderArgs[i];
    newBinding.binding = argI.index;
    switch (argI.type)
    {
    case Opal_Shader_Arg_Uniform_Buffer:
    {
      newBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    } break;
    default: return Opal_Failure_Vk_Create;
    }

    bindings[i] = newBinding;
  }

  VkDescriptorSetLayoutCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.bindingCount = _createInfo.shaderArgCount;
  createInfo.pBindings = bindings;

  OVK_ATTEMPT(
    vkCreateDescriptorSetLayout(_state->device, &createInfo, NULL, &_material->descriptorSetLayout),
    return Opal_Failure_Vk_Create);

  return Opal_Success;
}

OpalResult CreateDescriptorSet(
  OvkState_T* _state,
  OpalCreateMaterialInfo _createInfo,
  OvkMaterial_T* _material)
{
  VkDescriptorSetAllocateInfo allocInfo = { 0 };
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.pNext = NULL;
  allocInfo.descriptorPool = _state->descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &_material->descriptorSetLayout;

  OVK_ATTEMPT(
    vkAllocateDescriptorSets(_state->device, &allocInfo, &_material->descriptorSet),
    return Opal_Failure_Vk_Create);

  return Opal_Success;
}

OpalResult CreatePipelineLayout(OvkState_T* _state, OvkMaterial_T* _material)
{
  const uint32_t layoutCount = 1;
  VkDescriptorSetLayout layouts[1] = {
    // Global descriptor set layout
    // Camera? descriptor set layout
    _material->descriptorSetLayout
    // Object descriptor set layout
  };

  VkPipelineLayoutCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.pushConstantRangeCount = 0;
  createInfo.pPushConstantRanges = NULL;
  createInfo.setLayoutCount = 1;
  createInfo.pSetLayouts = layouts;

  OVK_ATTEMPT(
    vkCreatePipelineLayout(_state->device, &createInfo, NULL, &_material->pipelineLayout),
    return Opal_Failure_Vk_Create);

  return Opal_Success;
}

OpalResult CreatePipeline(
  OvkState_T* _state,
  OpalCreateMaterialInfo _createInfo,
  OvkMaterial_T* _oMaterial)
{
  // Shader stages =====
  uint32_t shaderStageCount = _createInfo.shaderCount;
  VkPipelineShaderStageCreateInfo* shaderStages =
    (VkPipelineShaderStageCreateInfo*)LapisMemAllocZero(
      sizeof(VkPipelineShaderStageCreateInfo) * _createInfo.shaderCount);

  VkPipelineShaderStageCreateInfo newShaderStage = { 0 };
  newShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  newShaderStage.pNext = NULL;
  newShaderStage.flags = 0;
  newShaderStage.pName = "main";
  for (uint32_t i = 0; i < _createInfo.shaderCount; i++)
  {
    newShaderStage.module = _createInfo.pShaders[i]->backend.vulkan.module;
    switch (_createInfo.pShaders[i]->type)
    {
    case Opal_Shader_Vertex: newShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT; break;
    case Opal_Shader_Fragment: newShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
    default: return Opal_Failure_Vk_Create;
    }

    shaderStages[i] = newShaderStage;
  }

  // Vertex input stage =====
  // TODO : Load vertex attributes from somewhere
  uint32_t vertAttributeCount = 1;
  VkVertexInputAttributeDescription vertAttributes [1];
  // Position
  vertAttributes[0].binding = 0;
  vertAttributes[0].location = 0;
  vertAttributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  vertAttributes[0].offset = 0;
  // Normal
  // UV

  VkVertexInputBindingDescription vertBinding;
  vertBinding.stride = sizeof(uint32_t) * 3;
  vertBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  vertBinding.binding = 0;

  VkPipelineVertexInputStateCreateInfo vertexInputStateInfo = { 0 };
  vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputStateInfo.pNext = NULL;
  vertexInputStateInfo.flags = 0;
  vertexInputStateInfo.vertexAttributeDescriptionCount = 0;
  //vertexInputStateInfo.vertexAttributeDescriptionCount = 1;
  //vertexInputStateInfo.pVertexAttributeDescriptions = &vertAttributes;
  vertexInputStateInfo.vertexBindingDescriptionCount = 0;
  //vertexInputStateInfo.vertexBindingDescriptionCount = 1;
  //vertexInputStateInfo.pVertexBindingDescriptions = &vertBinding;

  // Input assembly state =====
  VkPipelineInputAssemblyStateCreateInfo inputAssembyStateInfo = { 0 };
  inputAssembyStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembyStateInfo.pNext = NULL;
  inputAssembyStateInfo.flags = 0;
  inputAssembyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembyStateInfo.primitiveRestartEnable = VK_FALSE;

  // Viewport state =====
  VkViewport viewport = { 0 };
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = (float)_state->swapchain.extents.width;
  viewport.height = (float)_state->swapchain.extents.height;
  viewport.minDepth = 0;
  viewport.maxDepth = 1;

  VkRect2D scissor = { 0 };
  scissor.extent = (VkExtent2D){
    _state->swapchain.extents.width,
    _state->swapchain.extents.height };
  scissor.offset = (VkOffset2D){ 0, 0 };

  VkPipelineViewportStateCreateInfo viewportStateInfo = { 0 };
  viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportStateInfo.pNext = NULL;
  viewportStateInfo.flags = 0;
  viewportStateInfo.viewportCount = 1;
  viewportStateInfo.pViewports = &viewport;
  viewportStateInfo.scissorCount = 1;
  viewportStateInfo.pScissors = &scissor;

  // Rasterization state =====
  VkPipelineRasterizationStateCreateInfo rasterStateInfo = { 0 };
  rasterStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterStateInfo.pNext = NULL;
  rasterStateInfo.flags = 0;

  rasterStateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterStateInfo.polygonMode = VK_POLYGON_MODE_FILL;

  rasterStateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterStateInfo.lineWidth = 1.0f;
  rasterStateInfo.rasterizerDiscardEnable = VK_TRUE;
  rasterStateInfo.depthBiasEnable = VK_FALSE;
  rasterStateInfo.depthClampEnable = VK_FALSE;
  rasterStateInfo.rasterizerDiscardEnable = VK_FALSE;

  // Multisample state =====
  VkPipelineMultisampleStateCreateInfo multisampleStateInfo = { 0 };
  multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampleStateInfo.pNext = NULL;
  multisampleStateInfo.flags = 0;
  multisampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampleStateInfo.sampleShadingEnable = VK_FALSE;

  // Depth stencil state =====
  VkPipelineDepthStencilStateCreateInfo depthStateInfo = { 0 };
  depthStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStateInfo.pNext = NULL;
  depthStateInfo.flags = 0;
  depthStateInfo.depthTestEnable = VK_TRUE;
  depthStateInfo.depthWriteEnable = VK_TRUE;
  depthStateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
  depthStateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
  depthStateInfo.depthBoundsTestEnable = VK_FALSE;

  // Color blend state =====
  VkPipelineColorBlendAttachmentState blendAttachmentState[4] = { 0 };
  blendAttachmentState[0].colorWriteMask =
    VK_COLOR_COMPONENT_R_BIT
    | VK_COLOR_COMPONENT_B_BIT
    | VK_COLOR_COMPONENT_G_BIT
    | VK_COLOR_COMPONENT_A_BIT;
  blendAttachmentState[0].blendEnable = VK_FALSE;
  blendAttachmentState[1] = blendAttachmentState[0];
  blendAttachmentState[2] = blendAttachmentState[0];
  blendAttachmentState[3] = blendAttachmentState[0];

  VkPipelineColorBlendStateCreateInfo blendStateInfo = { 0 };
  blendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  blendStateInfo.pNext = NULL;
  blendStateInfo.flags = 0;
  blendStateInfo.logicOpEnable = VK_FALSE;
  blendStateInfo.attachmentCount = 1;
  blendStateInfo.pAttachments = blendAttachmentState;

  // Dynamic states =====
  VkPipelineDynamicStateCreateInfo dynamicStateInfo = { 0 };
  dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicStateInfo.pNext = NULL;
  dynamicStateInfo.flags = 0;
  dynamicStateInfo.dynamicStateCount = 0;
  dynamicStateInfo.pDynamicStates = NULL;

  // Creation =====
  VkGraphicsPipelineCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.pViewportState = &viewportStateInfo;
  createInfo.pVertexInputState = &vertexInputStateInfo;
  createInfo.pInputAssemblyState = &inputAssembyStateInfo;
  createInfo.pRasterizationState = &rasterStateInfo;
  createInfo.pMultisampleState = &multisampleStateInfo;
  createInfo.pDepthStencilState = &depthStateInfo;
  createInfo.pColorBlendState = &blendStateInfo;
  createInfo.pDynamicState = &dynamicStateInfo;
  createInfo.stageCount = shaderStageCount;
  createInfo.pStages = shaderStages;

  createInfo.layout = _oMaterial->pipelineLayout;
  createInfo.renderPass = _state->renderpass;
  createInfo.subpass = 0;

  OVK_ATTEMPT(
    vkCreateGraphicsPipelines(_state->device, NULL, 1, &createInfo, NULL, &_oMaterial->pipeline),
    return Opal_Failure_Vk_Create);

  LapisMemFree(shaderStages);
  return Opal_Success;
}

OpalResult UpdateShaderArguments(
  OvkState_T* _state,
  OpalCreateMaterialInfo _createInfo,
  OvkMaterial_T* _material)
{
  uint32_t bufferInfoCount = 0;
  VkDescriptorBufferInfo* bufferInfos =
    (VkDescriptorBufferInfo*)LapisMemAllocZero(
      sizeof(VkDescriptorBufferInfo) * _createInfo.shaderArgCount);
  uint32_t imageInfoCount = 0;
  VkDescriptorImageInfo* imageInfos =
    (VkDescriptorImageInfo*)LapisMemAllocZero(
      sizeof(VkDescriptorImageInfo) * _createInfo.shaderArgCount);
  VkWriteDescriptorSet* writes =
    (VkWriteDescriptorSet*)LapisMemAllocZero(
      sizeof(VkWriteDescriptorSet) * _createInfo.shaderArgCount);

  VkDescriptorBufferInfo newBuffer = { 0 };
  VkDescriptorImageInfo newImage = { 0 };
  VkWriteDescriptorSet newWrite = { 0 };
  newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  newWrite.dstSet = _material->descriptorSet;
  newWrite.dstArrayElement = 0;
  newWrite.descriptorCount = 1;
  newWrite.pTexelBufferView = NULL;

  for (uint32_t i = 0; i < _createInfo.shaderArgCount; i++)
  {
    OpalShaderArg argI = _createInfo.pShaderArgs[i];
    newWrite.dstBinding = argI.index;

    switch (argI.type)
    {
    case Opal_Shader_Arg_Uniform_Buffer:
    {
      newBuffer.buffer = argI.bufferData.buffer->backend.vulkan.buffer;
      newBuffer.offset = 0;
      newBuffer.range = VK_WHOLE_SIZE;
      bufferInfos[bufferInfoCount] = newBuffer;

      newWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      newWrite.pBufferInfo = &bufferInfos[bufferInfoCount];
      newWrite.pImageInfo = NULL;

      bufferInfoCount++;
    } break;
    default:
    {
      LapisMemFree(writes);
      LapisMemFree(imageInfos);
      LapisMemFree(bufferInfos);
      return Opal_Failure_Vk_Create;
    }
    }

    writes[i] = newWrite;
  }

  vkUpdateDescriptorSets(_state->device, _createInfo.shaderArgCount, writes, 0, NULL),

  LapisMemFree(writes);
  LapisMemFree(imageInfos);
  LapisMemFree(bufferInfos);
  return Opal_Success;
}

OpalResult OvkCreateMaterial(
  OpalState _oState,
  OpalCreateMaterialInfo _createInfo,
  OpalMaterial _oMaterial)
{
  OvkState_T* state = _oState->backend.state;
  OvkMaterial_T* newMaterial = &_oMaterial->backend.vulkan;

  OPAL_ATTEMPT(
    CreateDescriptorSetLayout(state, _createInfo, newMaterial),
    {
      OPAL_LOG_VK_ERROR("Failed to create material : descriptor set layout\n");
      return Opal_Failure_Vk_Create;
    });

  // Descriptor set
  OPAL_ATTEMPT(
    CreateDescriptorSet(state, _createInfo, newMaterial),
    {
      OPAL_LOG_VK_ERROR("Failed to create material : descriptor set\n");
      return Opal_Failure_Vk_Create;
    });

  OPAL_ATTEMPT(
    CreatePipelineLayout(state, newMaterial),
    {
      OPAL_LOG_VK_ERROR("Failed to create material : pipeline layout\n");
      return Opal_Failure_Vk_Create;
    });

  OPAL_ATTEMPT(
    CreatePipeline(state, _createInfo, newMaterial),
    {
      OPAL_LOG_VK_ERROR("Failed to create material : pipeline\n");
      return Opal_Failure_Vk_Create;
    });

  OPAL_ATTEMPT(
    UpdateShaderArguments(state, _createInfo, newMaterial),
    {
      OPAL_LOG_VK_ERROR("Failed to bind one or more shader arguments\n");
      return Opal_Failure_Vk_Create;
    });

  return Opal_Success;
}

void OvkDestroyMaterial(OpalState _oState, OpalMaterial _oMaterial)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;
  OvkMaterial_T* material = &_oMaterial->backend.vulkan;

  vkDeviceWaitIdle(state->device);

  vkFreeDescriptorSets(state->device, state->descriptorPool, 1, &material->descriptorSet);
  vkDestroyDescriptorSetLayout(state->device, material->descriptorSetLayout, NULL);
  vkDestroyPipeline(state->device, material->pipeline, NULL);
  vkDestroyPipelineLayout(state->device, material->pipelineLayout, NULL);
}
