
#include "src/defines.h"
#include "src/vulkan/vulkan.h"

#include <vulkan/vulkan.h>

OpalResult OvkBeginSingleUseCommand(OvkState_T* _state, VkCommandPool _pool, VkCommandBuffer* _cmd)
{
  VkCommandBufferAllocateInfo allocInfo = { 0 };
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = _pool;
  allocInfo.commandBufferCount = 1;

  OVK_ATTEMPT(
    vkAllocateCommandBuffers(_state->device, &allocInfo, _cmd),
    return Opal_Failure_Vk_Misc);

  VkCommandBufferBeginInfo beginInfo = { 0 };
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  beginInfo.pInheritanceInfo = NULL;

  OVK_ATTEMPT(
    vkBeginCommandBuffer(*_cmd, &beginInfo),
    return Opal_Failure_Vk_Misc);

  return Opal_Success;
}

OpalResult OvkEndSingleUseCommand(
  OvkState_T* _state,
  VkCommandPool _pool,
  VkQueue _queue,
  VkCommandBuffer _cmd)
{
  OVK_ATTEMPT(vkEndCommandBuffer(_cmd), return Opal_Failure_Vk_Misc);

  VkSubmitInfo submitInfo = { 0 };
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &_cmd;

  OVK_ATTEMPT(
    vkQueueSubmit(_queue, 1, &submitInfo, VK_NULL_HANDLE),
    return Opal_Failure_Vk_Misc);

  vkQueueWaitIdle(_queue);
  vkFreeCommandBuffers(_state->device, _pool, 1, &_cmd);

  return Opal_Success;
}

void tmprenderpassrender(
  OvkState_T* _state,
  OvkFrame_T* _frame,
  const OpalFrameData* _data,
  VkCommandBuffer cmd)
{
  const VkDeviceSize zeroDeviceSize = 0;

  for (uint32_t renderIndex = 0; renderIndex < _data->renderableCount - 1; renderIndex++)
  {
    vkCmdBindPipeline(
      cmd,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      _data->renderables[renderIndex]->material->backend.vulkan.pipeline);

    vkCmdBindDescriptorSets(
      cmd,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      _data->renderables[renderIndex]->material->backend.vulkan.pipelineLayout,
      0,
      1,
      &_data->renderables[renderIndex]->material->backend.vulkan.descriptorSet,
      0,
      NULL);

    vkCmdBindDescriptorSets(
      cmd,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      _data->renderables[renderIndex]->material->backend.vulkan.pipelineLayout,
      1,
      1,
      &_data->renderables[renderIndex]->backend.vulkan.descSet,
      0,
      NULL);

    vkCmdBindVertexBuffers(
      cmd,
      0,
      1,
      &_data->renderables[renderIndex]->mesh->vertexBuffer->backend.vulkan.buffer,
      &zeroDeviceSize);
    vkCmdBindIndexBuffer(
      cmd,
      _data->renderables[renderIndex]->mesh->indexBuffer->backend.vulkan.buffer,
      zeroDeviceSize,
      VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(cmd, _data->renderables[renderIndex]->mesh->indexCount, 1, 0, 0, 0);
  }

  vkCmdNextSubpass(cmd, VK_SUBPASS_CONTENTS_INLINE);

  uint32_t renderIndex = _data->renderableCount - 1;

  vkCmdBindPipeline(
    cmd,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    _data->renderables[renderIndex]->material->backend.vulkan.pipeline);

  vkCmdBindDescriptorSets(
    cmd,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    _data->renderables[renderIndex]->material->backend.vulkan.pipelineLayout,
    0,
    1,
    &_data->renderables[renderIndex]->material->backend.vulkan.descriptorSet,
    0,
    NULL);

  vkCmdBindDescriptorSets(
    cmd,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    _data->renderables[renderIndex]->material->backend.vulkan.pipelineLayout,
    1,
    1,
    &_data->renderables[renderIndex]->backend.vulkan.descSet,
    0,
    NULL);

  vkCmdBindVertexBuffers(
    cmd,
    0,
    1,
    &_data->renderables[renderIndex]->mesh->vertexBuffer->backend.vulkan.buffer,
    &zeroDeviceSize);
  vkCmdBindIndexBuffer(
    cmd,
    _data->renderables[renderIndex]->mesh->indexBuffer->backend.vulkan.buffer,
    zeroDeviceSize,
    VK_INDEX_TYPE_UINT32);
  vkCmdDrawIndexed(cmd, _data->renderables[renderIndex]->mesh->indexCount, 1, 0, 0, 0);

}

OpalResult OvkRecordCommandBuffer(
  OvkState_T* _state,
  OvkFrame_T* _frame,
  const OpalFrameData* _data)
{
  uint32_t index = _frame->swapchainImageIndex;
  VkCommandBuffer cmd = _frame->cmd;

  vkResetCommandBuffer(cmd, 0);

  VkCommandBufferBeginInfo cmdBeginInfo = { 0 };
  cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmdBeginInfo.pNext = NULL;
  cmdBeginInfo.flags = 0;

  OVK_ATTEMPT(
    vkBeginCommandBuffer(cmd, &cmdBeginInfo),
    {
      OPAL_LOG_VK_ERROR("Failed to begin command buffer : %u\n", index);
      return Opal_Failure_Vk_Render;
    });

  for (uint32_t i = 0; i < _data->renderpassCount; i++)
  {
    OpalRenderpass_T orp = *_data->renderpasses[i];
    OvkRenderpass_T vrp = _data->renderpasses[i]->backend.vulkan;

    VkClearValue* clearValues = (VkClearValue*)orp.clearValues;

    VkRenderPassBeginInfo rpBeginInfo = { 0 };
    rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBeginInfo.pNext = NULL;
    rpBeginInfo.clearValueCount = orp.attachmentCount;
    rpBeginInfo.pClearValues = clearValues;
    rpBeginInfo.renderArea.extent = (VkExtent2D){orp.extents.width, orp.extents.height};
    rpBeginInfo.renderArea.offset = (VkOffset2D){ 0, 0 };
    rpBeginInfo.renderPass = vrp.renderpass;
    rpBeginInfo.framebuffer = vrp.framebuffers[index % vrp.framebufferCount];

    vkCmdBeginRenderPass(cmd, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    //OPAL_ATTEMPT(orp.Render(), return Opal_Failure);
    tmprenderpassrender(_state, _frame, _data, cmd);

    vkCmdEndRenderPass(cmd);
  }

  OVK_ATTEMPT(
    vkEndCommandBuffer(cmd),
    {
      OPAL_LOG_VK_ERROR("Failed to end command buffer : %u\n", index);
      return Opal_Failure_Vk_Render;
    });

  return Opal_Success;
}
