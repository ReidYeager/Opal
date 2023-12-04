
#include "src/common.h"

OpalResult OvkShaderInit(OpalShader_T* _shader, OpalShaderInitInfo _initInfo)
{
  VkShaderModuleCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = _initInfo.size;
  createInfo.pCode = (uint32_t*)_initInfo.pSource;

  OVK_ATTEMPT(vkCreateShaderModule(oState.vk.device, &createInfo, oState.vk.allocator, &_shader->vk.module));

  switch (_initInfo.type)
  {
    case Opal_Shader_Vertex: _shader->vk.stage = VK_SHADER_STAGE_VERTEX_BIT; break;
    case Opal_Shader_Fragment: _shader->vk.stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
    //case Opal_Shader_Compute: _shader->vk.stage = VK_SHADER_STAGE_COMPUTE_BIT; break;
    //case Opal_Shader_Geometry: _shader->vk.stage = VK_SHADER_STAGE_GEOMETRY_BIT; break;
    default: return Opal_Failure;
  }

  return Opal_Success;
}

void OvkShaderShutdown(OpalShader_T* _shader)
{
  vkDestroyShaderModule(oState.vk.device, _shader->vk.module, oState.vk.allocator);
}

OpalResult CreateDescriptorSetLayout_Ovk(OpalInputLayout_T* _layout, OpalInputLayoutInitInfo _initInfo)
{
  VkDescriptorSetLayoutBinding* pBindings = LapisMemAllocZeroArray(VkDescriptorSetLayoutBinding, _initInfo.count);

  for (uint32_t i = 0; i < _initInfo.count; i++)
  {
    pBindings[i].binding = i;
    pBindings[i].descriptorCount = 1;
    pBindings[i].pImmutableSamplers = NULL;

    switch (_initInfo.pTypes[i])
    {
    case Opal_Input_Type_Uniform_Buffer:
    {
      pBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      pBindings[i].stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    } break;
    case Opal_Input_Type_Samped_Image:
    {
      pBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      pBindings[i].stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    } break;
    case Opal_Input_Type_Subpass_Input:
    {
      pBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
      pBindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    } break;
    default:
    {
      LapisMemFree(pBindings);
      return Opal_Failure;
    }
    }
  }

  VkDescriptorSetLayoutCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  createInfo.bindingCount = _initInfo.count;
  createInfo.pBindings = pBindings;

  OVK_ATTEMPT(
    vkCreateDescriptorSetLayout(oState.vk.device, &createInfo, oState.vk.allocator, &_layout->vk.descriptorLayout),
    LapisMemFree(pBindings));

  LapisMemFree(pBindings);

  return Opal_Success;
}

OpalResult CreateDescriptorSet_Ovk(OpalInputSet_T* _set, OpalInputSetInitInfo _initInfo)
{
  VkDescriptorSetAllocateInfo allocInfo = { 0 };
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = oState.vk.descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts =  &_initInfo.layout->vk.descriptorLayout;

  OVK_ATTEMPT(vkAllocateDescriptorSets(oState.vk.device, &allocInfo, &_set->vk.descriptorSet));

  return Opal_Success;
}

OpalResult UpdateDescriptorSet_Ovk(OpalInputSet_T* _set, uint32_t _count, OpalInputInfo* _pInputs)
{
  uint32_t bufferInfoCount = 0;
  uint32_t imageInfoCount = 0;
  VkDescriptorBufferInfo* pBufferInfos = LapisMemAllocZeroArray(VkDescriptorBufferInfo, _count);
  VkDescriptorImageInfo* pImageInfos = LapisMemAllocZeroArray(VkDescriptorImageInfo, _count);
  VkWriteDescriptorSet* pWrites = LapisMemAllocZeroArray(VkWriteDescriptorSet, _count);

  for (uint32_t i = 0; i < _count; i++)
  {
    pWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    pWrites[i].descriptorCount = 1;
    pWrites[i].dstArrayElement = 0;
    pWrites[i].dstSet = _set->vk.descriptorSet;
    pWrites[i].dstBinding = i;

    switch (_pInputs[i].type)
    {
    case Opal_Input_Type_Uniform_Buffer:
    {
      pBufferInfos[bufferInfoCount].buffer = _pInputs[i].value.buffer->vk.buffer;
      pBufferInfos[bufferInfoCount].offset = 0;
      pBufferInfos[bufferInfoCount].range = VK_WHOLE_SIZE;

      pWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      pWrites[i].pBufferInfo = &pBufferInfos[bufferInfoCount];
      pWrites[i].pImageInfo = NULL;

      bufferInfoCount++;
    } break;
    case Opal_Input_Type_Samped_Image:
    {
      pImageInfos[imageInfoCount].imageLayout = _pInputs[i].value.image->vk.layout;
      pImageInfos[imageInfoCount].imageView = _pInputs[i].value.image->vk.view;
      pImageInfos[imageInfoCount].sampler = _pInputs[i].value.image->vk.sampler;

      pWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      pWrites[i].pBufferInfo = NULL;
      pWrites[i].pImageInfo = &pImageInfos[imageInfoCount];

      imageInfoCount++;
    } break;
    case Opal_Input_Type_Subpass_Input:
    {
      pImageInfos[imageInfoCount].imageLayout = _pInputs[i].value.image->vk.layout;
      pImageInfos[imageInfoCount].imageView = _pInputs[i].value.image->vk.view;
      pImageInfos[imageInfoCount].sampler = VK_NULL_HANDLE;

      pWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
      pWrites[i].pBufferInfo = NULL;
      pWrites[i].pImageInfo = &pImageInfos[imageInfoCount];

      imageInfoCount++;
    } break;
    default:
    {
      LapisMemFree(pBufferInfos);
      LapisMemFree(pImageInfos);
      LapisMemFree(pWrites);
      OpalLogError("Unknown material input type %d", _pInputs[i].type);
      return Opal_Failure;
    } break;
    }
  }

  vkUpdateDescriptorSets(oState.vk.device, _count, pWrites, 0, NULL);

  LapisMemFree(pBufferInfos);
  LapisMemFree(pImageInfos);
  LapisMemFree(pWrites);

  return Opal_Success;
}

OpalResult OvkInputLayoutInit(OpalInputLayout_T* _layout, OpalInputLayoutInitInfo _initInfo)
{
  OPAL_ATTEMPT(CreateDescriptorSetLayout_Ovk(_layout, _initInfo));
  return Opal_Success;
}

OpalResult OvkInputSetInit(OpalInputSet_T* _set, OpalInputSetInitInfo _initInfo)
{
  OPAL_ATTEMPT(CreateDescriptorSet_Ovk(_set, _initInfo));

  OpalInputInfo* pInputs = LapisMemAllocZeroArray(OpalInputInfo, _initInfo.layout->count);
  for (uint32_t i = 0; i < _initInfo.layout->count; i++)
  {
    pInputs[i].index = i;
    pInputs[i].type = _initInfo.layout->pTypes[i];
    pInputs[i].value = _initInfo.pInputValues[i];
  }

  OPAL_ATTEMPT(UpdateDescriptorSet_Ovk(_set, _initInfo.layout->count, pInputs), LapisMemFree(pInputs));

  LapisMemFree(pInputs);
  return Opal_Success;
}

void OvkInputLayoutShutdown(OpalInputLayout_T* _layout)
{
  vkDestroyDescriptorSetLayout(oState.vk.device, _layout->vk.descriptorLayout, oState.vk.allocator);
}

void OvkInputSetShutdown(OpalInputSet_T* _set)
{
  vkFreeDescriptorSets(oState.vk.device, oState.vk.descriptorPool, 1, &_set->vk.descriptorSet);
}

OpalResult OvkInputSetUpdate(OpalInputSet _set, uint32_t _count, OpalInputInfo* _pInputs)
{
  OPAL_ATTEMPT(UpdateDescriptorSet_Ovk(_set, _count, _pInputs));
  return Opal_Success;
}

OpalResult CreatePipelineLayout_Ovk(OpalMaterial_T* _material, OpalMaterialInitInfo _initInfo)
{
  VkDescriptorSetLayout* pLayouts = LapisMemAllocArray(VkDescriptorSetLayout, _initInfo.inputLayoutCount);
  for (uint32_t i = 0; i < _initInfo.inputLayoutCount; i++)
  {
    pLayouts[i] = _initInfo.pInputLayouts[i]->vk.descriptorLayout;
  }

  VkPushConstantRange pushRange = { 0 };
  pushRange.offset = 0;
  pushRange.size = _initInfo.pushConstantSize;
  pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

  VkPipelineLayoutCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  createInfo.pushConstantRangeCount = (_initInfo.pushConstantSize > 0);
  createInfo.pPushConstantRanges = (_initInfo.pushConstantSize) ? &pushRange : NULL;
  createInfo.setLayoutCount = _initInfo.inputLayoutCount;
  createInfo.pSetLayouts = pLayouts;

  OVK_ATTEMPT(
    vkCreatePipelineLayout(oState.vk.device, &createInfo, oState.vk.allocator, &_material->vk.pipelineLayout),
    LapisMemFree(pLayouts));

  LapisMemFree(pLayouts);
  return Opal_Success;
}

OpalResult CreatePipeline_Ovk(OpalMaterial_T* _material, OpalMaterialInitInfo _initInfo)
{
    // Shader stages =====
  VkPipelineShaderStageCreateInfo* pShaderStages = NULL;

  if (_initInfo.shaderCount > 0)
  {
    pShaderStages = LapisMemAllocZeroArray(VkPipelineShaderStageCreateInfo, _initInfo.shaderCount);
  }

  for (uint32_t i = 0; i < _initInfo.shaderCount; i++)
  {
    pShaderStages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pShaderStages[i].module = _initInfo.pShaders[i]->vk.module;
    pShaderStages[i].stage = _initInfo.pShaders[i]->vk.stage;
    pShaderStages[i].pName = "main";
  }

  // Vertex input stage =====
  VkPipelineVertexInputStateCreateInfo vertexInputStateInfo = { 0 };
  vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputStateInfo.pNext = NULL;
  vertexInputStateInfo.flags = 0;
  vertexInputStateInfo.vertexAttributeDescriptionCount = oState.vertexFormat.attribCount;
  vertexInputStateInfo.pVertexAttributeDescriptions = oState.vertexFormat.vk.pAttribDescriptions;
  vertexInputStateInfo.vertexBindingDescriptionCount = 1;
  vertexInputStateInfo.pVertexBindingDescriptions = &oState.vertexFormat.vk.bindingDescription;

  // Input assembly state =====
  VkPipelineInputAssemblyStateCreateInfo inputAssembyStateInfo = { 0 };
  inputAssembyStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembyStateInfo.pNext = NULL;
  inputAssembyStateInfo.flags = 0;
  inputAssembyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembyStateInfo.primitiveRestartEnable = VK_FALSE;

  // Viewport state =====
  VkPipelineViewportStateCreateInfo viewportStateInfo = { 0 };
  viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportStateInfo.pNext = NULL;
  viewportStateInfo.flags = 0;
  viewportStateInfo.viewportCount = 1;
  viewportStateInfo.pViewports = NULL;
  viewportStateInfo.scissorCount = 1;
  viewportStateInfo.pScissors = NULL;

  // Rasterization state =====
  VkPipelineRasterizationStateCreateInfo rasterStateInfo = { 0 };
  rasterStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterStateInfo.pNext = NULL;
  rasterStateInfo.flags = 0;

  rasterStateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
  //rasterStateInfo.cullMode = VK_CULL_MODE_NONE;
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
  blendAttachmentState[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT
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
    //_oMaterial->renderpass->pSubpasses[_oMaterial->subpassIndex].colorAttachmentCount;
  blendStateInfo.pAttachments = blendAttachmentState;

  // Dynamic states =====
  uint32_t dynStateCount = 2;
  VkDynamicState dynStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

  VkPipelineDynamicStateCreateInfo dynamicStateInfo = { 0 };
  dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicStateInfo.pNext = NULL;
  dynamicStateInfo.flags = 0;
  dynamicStateInfo.dynamicStateCount = dynStateCount;
  dynamicStateInfo.pDynamicStates = dynStates;

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

  createInfo.stageCount = 2;
  createInfo.pStages = pShaderStages;

  createInfo.layout = _material->vk.pipelineLayout;
  createInfo.renderPass = _initInfo.renderpass->vk.renderpass;
  createInfo.subpass = _initInfo.subpassIndex;

  OVK_ATTEMPT(vkCreateGraphicsPipelines(oState.vk.device, VK_NULL_HANDLE, 1, &createInfo, oState.vk.allocator, &_material->vk.pipeline));

  return Opal_Success;
}

OpalResult OvkMaterialInit(OpalMaterial_T* _material, OpalMaterialInitInfo _initInfo)
{
  OPAL_ATTEMPT(CreatePipelineLayout_Ovk(_material, _initInfo));
  OPAL_ATTEMPT(CreatePipeline_Ovk(_material, _initInfo));

  return Opal_Success;
}

void OvkMaterialShutdown(OpalMaterial_T* _material)
{
  vkDestroyPipeline(oState.vk.device, _material->vk.pipeline, oState.vk.allocator);
  vkDestroyPipelineLayout(oState.vk.device, _material->vk.pipelineLayout, oState.vk.allocator);
}

OpalResult OvkMaterialReinit(OpalMaterial_T* _material)
{
  OpalMaterialInitInfo newInit = { 0 };
  newInit.shaderCount = _material->shaderCount;
  newInit.pShaders = _material->pShaders;
  newInit.renderpass = _material->ownerRenderpass;
  newInit.subpassIndex = _material->subpassIndex;

  vkDestroyPipeline(oState.vk.device, _material->vk.pipeline, oState.vk.allocator);
  OPAL_ATTEMPT(CreatePipeline_Ovk(_material, newInit))

  return Opal_Success;
}
