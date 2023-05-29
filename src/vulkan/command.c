
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

OpalResult OvkRecordCommandBuffer(
  OvkState_T* _state,
  OvkFrame_T* _frame,
  const OpalFrameData* _data)
{
  uint32_t index = _frame->swapchainImageIndex;
  VkCommandBuffer cmd = _frame->cmd;

  vkResetCommandBuffer(cmd, 0);

  const VkDeviceSize zeroDeviceSize = 0;

  const uint32_t clearCount = 2;
  VkClearValue clearValues[2] = { 0 };
  clearValues[0].depthStencil = (VkClearDepthStencilValue){ 1, 0 };
  clearValues[1].color = (VkClearColorValue){ 0.4f, 0.2f, 0.6f, 0.0f };

  VkRenderPassBeginInfo rpBeginInfo = { 0 };
  rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  rpBeginInfo.pNext = NULL;
  rpBeginInfo.clearValueCount = clearCount;
  rpBeginInfo.pClearValues = clearValues;
  rpBeginInfo.renderArea.extent = _state->swapchain.extents;
  rpBeginInfo.renderArea.offset = (VkOffset2D){ 0, 0 };
  rpBeginInfo.renderPass = _state->renderpass;
  rpBeginInfo.framebuffer = _state->framebuffers[index];

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

  // Commands start =====

  vkCmdBeginRenderPass(cmd, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

  for (uint32_t renderIndex = 0; renderIndex < _data->renderableCount; renderIndex++)
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

  vkCmdEndRenderPass(cmd);

  // Commands end =====

  OVK_ATTEMPT(
    vkEndCommandBuffer(cmd),
    {
      OPAL_LOG_VK_ERROR("Failed to end command buffer : %u\n", index);
      return Opal_Failure_Vk_Render;
    });

  return Opal_Success;
}
