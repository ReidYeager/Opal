
#include "src/vulkan/vulkan_common.h"

// Declarations
// ============================================================

// Shader ==========
//OpalResult OpalVulkanShaderInit               (OpalShader* pShader, OpalShaderInitInfo initInfo)
//void       OpalVulkanShaderShutdown           (OpalShader* pShader)

// ShaderInput ==========
//OpalResult OpalVulkanShaderInputLayoutInit    (OpalShaderInputLayout* pLayout, OpalShaderInputLayoutInitInfo initInfo)
//void       OpalVulkanShaderInputLayoutShutdown(OpalShaderInputLayout* pLayout)
//OpalResult OpalVulkanShaderInputInit          (OpalShaderInput* pShaderInput, OpalShaderInputInitInfo initInfo)
//void       OpalVulkanShaderInputShutdown      (OpalShaderInput* pShaderInput)
OpalResult   UpdateDescriptorSet_Ovk            (OpalShaderInput* pInput, OpalShaderInputValue* pValues);

// ShaderGroup ==========
//OpalResult OpalVulkanShaderGroupInit          (OpalShaderGroup* pShaderGroup, OpalShaderGroupInitInfo initInfo)
//void       OpalVulkanShaderGroupShutdown      (OpalShaderGroup* pShaderGroup)
OpalResult   InitPipelineLayout_Ovk             (OpalShaderGroup* pShaderGroup, OpalShaderGroupInitInfo initInfo);
OpalResult   InitPipeline_Ovk                   (OpalShaderGroup* pShaderGroup, OpalShaderGroupInitInfo initInfo);


// Shader
// ============================================================

OpalResult OpalVulkanShaderInit(OpalShader* pShader, OpalShaderInitInfo initInfo)
{
  VkShaderModuleCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.codeSize = initInfo.sourceSize;
  createInfo.pCode = (uint32_t*)initInfo.pSource;

  OPAL_ATTEMPT_VK(vkCreateShaderModule(g_ovkState->device, &createInfo, NULL, &pShader->api.vk.module));

  switch (initInfo.type)
  {
  case Opal_Shader_Vertex: pShader->api.vk.stage = VK_SHADER_STAGE_VERTEX_BIT; break;
  case Opal_Shader_Fragment: pShader->api.vk.stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
  //case Opal_Shader_Geometry: pShader->api.vk.stage = VK_SHADER_STAGE_GEOMETRY_BIT; break;
  //case Opal_Shader_Compute: pShader->api.vk.stage = VK_SHADER_STAGE_COMPUTE_BIT; break;
  //case Opal_Shader_Mesh: pShader->api.vk.stage = VK_SHADER_STAGE_MESH_BIT_EXT; break;
  default:
  {
    vkDestroyShaderModule(g_ovkState->device, pShader->api.vk.module, NULL);
    return Opal_Failure_Invalid_Input;
  }
  }

  return Opal_Success;
}

void OpalVulkanShaderShutdown(OpalShader* pShader)
{
  vkDestroyShaderModule(g_ovkState->device, pShader->api.vk.module, NULL);
}

// ShaderInput
// ============================================================

OpalResult OpalVulkanShaderInputLayoutInit(OpalShaderInputLayout* pLayout, OpalShaderInputLayoutInitInfo initInfo)
{
  VkDescriptorSetLayoutBinding* bindings = OpalMemAllocArray(VkDescriptorSetLayoutBinding, initInfo.count);

  for (int i = 0; i < initInfo.count; i++)
  {
    bindings[i].binding = i;
    bindings[i].descriptorCount = 1;
    bindings[i].pImmutableSamplers = NULL;
    bindings[i].stageFlags = OpalStagesToVkStages_Ovk(initInfo.pStages[i]);

    switch (initInfo.pTypes[i])
    {
    case Opal_Shader_Input_Buffer:
    {
      bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    } break;
    case Opal_Shader_Input_Image:
    {
      bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    } break;
    case Opal_Shader_Input_Subpass_Product:
    {
      bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    } break;
    default:
    {
      OpalMemFree(bindings);
      OpalLogError("Invalid shader input type : %d", initInfo.pTypes[i]);
      return Opal_Failure_Invalid_Input;
    }
    }
  }

  VkDescriptorSetLayoutCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.bindingCount = initInfo.count;
  createInfo.pBindings = bindings;

  OPAL_ATTEMPT_VK(
    vkCreateDescriptorSetLayout(g_ovkState->device, &createInfo, NULL, &pLayout->api.vk.layout),
    OpalMemFree(bindings));

  pLayout->elementCount = initInfo.count;
  pLayout->pTypes = OpalMemAllocArray(OpalShaderInputType, initInfo.count);
  OpalMemCopy(initInfo.pTypes, pLayout->pTypes, sizeof(OpalShaderInputType) * initInfo.count);

  OpalMemFree(bindings);
  return Opal_Success;
}

void OpalVulkanShaderInputLayoutShutdown(OpalShaderInputLayout* pLayout)
{
  OpalMemFree(pLayout->pTypes);
  vkDestroyDescriptorSetLayout(g_ovkState->device, pLayout->api.vk.layout, NULL);
}

OpalResult OpalVulkanShaderInputInit(OpalShaderInput* pShaderInput, OpalShaderInputInitInfo initInfo)
{
  VkDescriptorSetAllocateInfo allocInfo = { 0 };
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.pNext = NULL;
  allocInfo.descriptorPool = g_ovkState->descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &initInfo.layout.api.vk.layout;

  OPAL_ATTEMPT_VK(vkAllocateDescriptorSets(g_ovkState->device, &allocInfo, &pShaderInput->api.vk.set));

  pShaderInput->layout = initInfo.layout;

  UpdateDescriptorSet_Ovk(pShaderInput, initInfo.pValues);

  return Opal_Success;
}

void OpalVulkanShaderInputShutdown(OpalShaderInput* pShaderInput)
{
  vkFreeDescriptorSets(g_ovkState->device, g_ovkState->descriptorPool, 1, &pShaderInput->api.vk.set);
}

OpalResult UpdateDescriptorSet_Ovk(OpalShaderInput* pInput, OpalShaderInputValue* pValues)
{
  if (pInput->layout.elementCount == 0)
  {
    return Opal_Failure_Invalid_Input;
  }

  uint32_t bufferInfoCount = 0;
  uint32_t imageInfoCount = 0;
  VkDescriptorBufferInfo* pBufferInfos = OpalMemAllocArrayZeroed(VkDescriptorBufferInfo, pInput->layout.elementCount);
  VkDescriptorImageInfo* pImageInfos = OpalMemAllocArrayZeroed(VkDescriptorImageInfo, pInput->layout.elementCount);
  VkWriteDescriptorSet* pWrites = OpalMemAllocArrayZeroed(VkWriteDescriptorSet, pInput->layout.elementCount);

  for (int i = 0; i < pInput->layout.elementCount; i++)
  {
    pWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    pWrites[i].descriptorCount = 1;
    pWrites[i].dstArrayElement = 0;
    pWrites[i].dstSet = pInput->api.vk.set;
    pWrites[i].dstBinding = i;

    switch (pInput->layout.pTypes[i])
    {
    case Opal_Shader_Input_Buffer:
    {
      pBufferInfos[bufferInfoCount].buffer = pValues[i].buffer->api.vk.buffer;
      pBufferInfos[bufferInfoCount].offset = 0;
      pBufferInfos[bufferInfoCount].range  = VK_WHOLE_SIZE;

      pWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      pWrites[i].pBufferInfo    = &pBufferInfos[bufferInfoCount];
      pWrites[i].pImageInfo     = NULL;

      bufferInfoCount++;
    } break;
    case Opal_Shader_Input_Image:
    {
      if ((pValues[i].image->usage & Opal_Image_Usage_Uniform) == 0)
      {
        OpalMemFree(pBufferInfos);
        OpalMemFree(pImageInfos);
        OpalMemFree(pWrites);

        OpalLogError("Image %d is not marked for uniform usage", i);
        return Opal_Failure_Invalid_Input;
      }

      pImageInfos[imageInfoCount].imageLayout = pValues[i].image->api.vk.layout;
      pImageInfos[imageInfoCount].imageView   = pValues[i].image->api.vk.view;
      pImageInfos[imageInfoCount].sampler     = pValues[i].image->api.vk.sampler;

      pWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      pWrites[i].pBufferInfo = NULL;
      pWrites[i].pImageInfo = &pImageInfos[imageInfoCount];

      imageInfoCount++;
    } break;
    case Opal_Shader_Input_Subpass_Product:
    {
      pImageInfos[imageInfoCount].imageLayout = pValues[i].image->api.vk.layout;
      pImageInfos[imageInfoCount].imageView   = pValues[i].image->api.vk.view;
      pImageInfos[imageInfoCount].sampler     = VK_NULL_HANDLE;

      pWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
      pWrites[i].pBufferInfo = NULL;
      pWrites[i].pImageInfo = &pImageInfos[imageInfoCount];

      imageInfoCount++;
    } break;
    default:
    {
      OpalMemFree(pBufferInfos);
      OpalMemFree(pImageInfos);
      OpalMemFree(pWrites);
      OpalLogError("Unknown material input type %d", pInput->layout.pTypes[i]);
      return Opal_Failure_Invalid_Input;
    }
    }
  }

  vkUpdateDescriptorSets(g_ovkState->device, pInput->layout.elementCount, pWrites, 0, NULL);

  OpalMemFree(pBufferInfos);
  OpalMemFree(pImageInfos);
  OpalMemFree(pWrites);

  return Opal_Success;
}

// ShaderGroup
// ============================================================

OpalResult OpalVulkanShaderGroupInit(OpalShaderGroup* pShaderGroup, OpalShaderGroupInitInfo initInfo)
{
  OPAL_ATTEMPT(InitPipelineLayout_Ovk(pShaderGroup, initInfo));
  OPAL_ATTEMPT(InitPipeline_Ovk(pShaderGroup, initInfo));

  pShaderGroup->pushConstSize = initInfo.pushConstantSize;

  return Opal_Success;
}

void OpalVulkanShaderGroupShutdown(OpalShaderGroup* pShaderGroup)
{
  vkDestroyPipeline(g_ovkState->device, pShaderGroup->api.vk.pipeline, NULL);
  vkDestroyPipelineLayout(g_ovkState->device, pShaderGroup->api.vk.pipelineLayout, NULL);
}

OpalResult InitPipelineLayout_Ovk(OpalShaderGroup* pShaderGroup, OpalShaderGroupInitInfo initInfo)
{
  VkDescriptorSetLayout* pLayouts = NULL;
  if (initInfo.shaderInputLayoutCount > 0)
  {
    pLayouts = OpalMemAllocArray(VkDescriptorSetLayout, initInfo.shaderInputLayoutCount);
    for (uint32_t i = 0; i < initInfo.shaderInputLayoutCount; i++)
    {
      pLayouts[i] = initInfo.pShaderInputLayouts[i].api.vk.layout;
    }
  }

  VkPushConstantRange pushRange;
  pushRange.offset = 0;
  pushRange.size = initInfo.pushConstantSize;
  pushRange.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

  VkPipelineLayoutCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.pushConstantRangeCount = (initInfo.pushConstantSize > 0);
  createInfo.pPushConstantRanges = (initInfo.pushConstantSize) ? &pushRange : NULL;
  createInfo.setLayoutCount = initInfo.shaderInputLayoutCount;
  createInfo.pSetLayouts = pLayouts;

  OPAL_ATTEMPT_VK(
    vkCreatePipelineLayout(g_ovkState->device, &createInfo, NULL, &pShaderGroup->api.vk.pipelineLayout),
    OpalMemFree(pLayouts));

  OpalMemFree(pLayouts);
  return Opal_Success;
}

OpalResult InitPipeline_Ovk(OpalShaderGroup* pShaderGroup, OpalShaderGroupInitInfo initInfo)
{
  // Shader stages =====
  VkPipelineShaderStageCreateInfo* pShaderStages = NULL;

  if (initInfo.shaderCount > 0)
  {
    pShaderStages = OpalMemAllocArrayZeroed(VkPipelineShaderStageCreateInfo, initInfo.shaderCount);
  }

  for (uint32_t i = 0; i < initInfo.shaderCount; i++)
  {
    pShaderStages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pShaderStages[i].pNext = NULL;
    pShaderStages[i].flags = 0;
    pShaderStages[i].module = initInfo.pShaders[i].api.vk.module;
    pShaderStages[i].stage = initInfo.pShaders[i].api.vk.stage;
    pShaderStages[i].pName = "main";
  }

  // Vertex input stage =====
  VkPipelineVertexInputStateCreateInfo vertexInputStateInfo = { 0 };
  vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputStateInfo.pNext = NULL;
  vertexInputStateInfo.flags = 0;
  vertexInputStateInfo.vertexBindingDescriptionCount = g_ovkState->vertex.attribCount > 0;
  vertexInputStateInfo.pVertexBindingDescriptions = &g_ovkState->vertex.bindingDescription;
  vertexInputStateInfo.vertexAttributeDescriptionCount = g_ovkState->vertex.attribCount;
  vertexInputStateInfo.pVertexAttributeDescriptions = g_ovkState->vertex.pAttribDescriptions;

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

  switch (initInfo.flags & Opal_Pipeline_Cull_BITS)
  {
  case Opal_Pipeline_Cull_None: rasterStateInfo.cullMode = VK_CULL_MODE_NONE; break;
  case Opal_Pipeline_Cull_Front: rasterStateInfo.cullMode = VK_CULL_MODE_FRONT_BIT; break;
  case Opal_Pipeline_Cull_Back: rasterStateInfo.cullMode = VK_CULL_MODE_BACK_BIT; break;
  case Opal_Pipeline_Cull_Both: rasterStateInfo.cullMode = VK_CULL_MODE_FRONT_AND_BACK; break;
  }
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
  multisampleStateInfo.alphaToCoverageEnable = VK_FALSE;
  multisampleStateInfo.alphaToOneEnable = VK_FALSE;

  // Depth stencil state =====
  VkPipelineDepthStencilStateCreateInfo depthStateInfo = { 0 };
  depthStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStateInfo.pNext = NULL;
  depthStateInfo.flags = 0;
  depthStateInfo.depthTestEnable = VK_TRUE;
  depthStateInfo.depthWriteEnable = VK_TRUE;
  switch (initInfo.flags & Opal_Pipeline_Depth_Compare_BITS)
  {
  case Opal_Pipeline_Depth_Compare_Less: depthStateInfo.depthCompareOp = VK_COMPARE_OP_LESS; break;
  case Opal_Pipeline_Depth_Compare_LessEqual: depthStateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL; break;
  }
  depthStateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
  depthStateInfo.depthBoundsTestEnable = VK_FALSE;

  // Color blend state =====
  VkPipelineColorBlendAttachmentState blendAttachmentState[4];
  blendAttachmentState[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT
    | VK_COLOR_COMPONENT_B_BIT
    | VK_COLOR_COMPONENT_G_BIT
    | VK_COLOR_COMPONENT_A_BIT;
  blendAttachmentState[0].blendEnable = VK_FALSE;
  blendAttachmentState[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  blendAttachmentState[0].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  blendAttachmentState[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  blendAttachmentState[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  blendAttachmentState[0].alphaBlendOp = VK_BLEND_OP_ADD;
  blendAttachmentState[0].colorBlendOp = VK_BLEND_OP_ADD;
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

  createInfo.layout = pShaderGroup->api.vk.pipelineLayout;
  createInfo.renderPass = initInfo.renderpass.api.vk.renderpass;
  createInfo.subpass = initInfo.subpassIndex;

  OPAL_ATTEMPT_VK(
    vkCreateGraphicsPipelines(
      g_ovkState->device,
      VK_NULL_HANDLE,
      1,
      &createInfo,
      NULL,
      &pShaderGroup->api.vk.pipeline),
    {
      if (pShaderStages != NULL)
      {
        OpalMemFree(pShaderStages);
      }
    });

  if (pShaderStages != NULL)
  {
    OpalMemFree(pShaderStages);
  }

  return Opal_Success;
}