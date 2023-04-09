
#include "src/defines.h"
#include "src/vulkan/vulkan.h"

#include <vulkan/vulkan.h>

OpalResult RecordCommandBuffer(OvkState_T* _state, OvkFrameSlot_T* _frame)
{
  uint32_t index = _frame->swapchainImageIndex;
  VkCommandBuffer cmd = _frame->cmd;

  vkResetCommandBuffer(cmd, 0);

  VkClearValue clearValues[1] = { 0 };
  clearValues[0].color = (VkClearColorValue){ 0.4f, 0.2f, 0.6f, 0.0f };

  VkRenderPassBeginInfo rpBeginInfo = { 0 };
  rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  rpBeginInfo.pNext = NULL;
  rpBeginInfo.clearValueCount = 1;
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

  vkCmdBeginRenderPass(cmd, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

  vkCmdEndRenderPass(cmd);

  OVK_ATTEMPT(
    vkEndCommandBuffer(cmd),
    {
      OPAL_LOG_VK_ERROR("Failed to end command buffer : %u\n", index);
      return Opal_Failure_Vk_Render;
    });

  return Opal_Success;
}

OpalResult OpalRenderFrame(OpalState _oState)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;
  uint32_t slotIndex = state->currentFrameSlotIndex;
  OvkFrameSlot_T* frameSlot = &state->frameSlots[slotIndex];

  // Setup =====

  OVK_ATTEMPT(
    // 3 sec timeout
    vkWaitForFences(state->device, 1, &frameSlot->fenceFrameAvailable, VK_TRUE, 3000000000),
    return Opal_Failure_Vk_Render);

  OVK_ATTEMPT(
    vkAcquireNextImageKHR(
      state->device,
      state->swapchain.swapchain,
      UINT64_MAX,
      frameSlot->semImageAvailable,
      VK_NULL_HANDLE,
      &frameSlot->swapchainImageIndex),
    return Opal_Failure_Vk_Render);

  // Record =====

  OPAL_ATTEMPT(RecordCommandBuffer(state, frameSlot), return Opal_Failure_Vk_Render);

  // Render =====

  VkPipelineStageFlags waitStages[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  VkSubmitInfo submitInfo = { 0 };
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext = NULL;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &frameSlot->semImageAvailable;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &frameSlot->cmd;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &frameSlot->semRenderComplete;

  OVK_ATTEMPT(
    vkResetFences(state->device, 1, &frameSlot->fenceFrameAvailable),
    return Opal_Failure_Vk_Render);

  OVK_ATTEMPT(
    vkQueueSubmit(state->queueGraphics, 1, &submitInfo, frameSlot->fenceFrameAvailable),
    return Opal_Failure_Vk_Render);

  // Present =====

  VkPresentInfoKHR presentInfo = { 0 };
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = NULL;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &frameSlot->semRenderComplete;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &state->swapchain.swapchain;
  presentInfo.pImageIndices = &frameSlot->swapchainImageIndex;

  OVK_ATTEMPT(vkQueuePresentKHR(state->queuePresent, &presentInfo), return Opal_Failure_Vk_Render);

  state->currentFrameSlotIndex = (state->currentFrameSlotIndex + 1) % maxFlightSlotCount;

  return Opal_Success;
}
