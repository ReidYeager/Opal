
#include "src/defines.h"
#include "src/vulkan/vulkan.h"

#include <vulkan/vulkan.h>

OpalResult RecordCommandBuffer(OvkState_T* _state)
{
  uint32_t index = _state->sync.currentSwapchainImageIndex;
  VkCommandBuffer cmd = _state->graphicsCommandBuffers[index];

  VkClearValue clearValues[1] = { 0 };
  clearValues[0].color = (VkClearColorValue){ 0.4f, 0.6f, 0.2f, 0.0f };

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
      return Opal_Failure_Vk_Record;
    });

  vkCmdBeginRenderPass(cmd, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

  vkCmdEndRenderPass(cmd);

  OVK_ATTEMPT(
    vkEndCommandBuffer(cmd),
    {
      OPAL_LOG_VK_ERROR("Failed to end command buffer : %u\n", index);
      return Opal_Failure_Vk_Record;
    });

  return Opal_Success;
}

VkResult renderResult;
OpalResult OpalRenderFrame(OpalState _state)
{
  OvkState_T* vkState = (OvkState_T*)_state->backend.state;
  uint32_t flightSlotIndex = vkState->sync.currentFlightIndex;

  OVK_ATTEMPT(
    vkWaitForFences(
      vkState->device,
      1,
      &vkState->sync.fenceFlightSlotAvailable[flightSlotIndex],
      VK_TRUE,
      3000000000),
    {
      OPAL_LOG_VK_ERROR("Wait for flight slot fence failed : %u\n", flightSlotIndex);
      return Opal_Failure_Vk_Render;
    });

  renderResult =
    vkAcquireNextImageKHR(
      vkState->device,
      vkState->swapchain.swapchain,
      UINT64_MAX,
      vkState->sync.semaphoreImageAvailable[flightSlotIndex],
      VK_NULL_HANDLE,
      &vkState->sync.currentSwapchainImageIndex);

  if (renderResult == VK_ERROR_OUT_OF_DATE_KHR)
  {
    OPAL_LOG_VK_ERROR("Need to update components for resize\n");
    return Opal_Failure_Vk_Render;
  }
  else if (renderResult != VK_SUCCESS)
  {
    return Opal_Failure_Vk_Render;
  }

  RecordCommandBuffer(vkState);

  VkPipelineStageFlags waitStages[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  VkSubmitInfo submitInfo = { 0 };
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext = NULL;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &vkState->sync.semaphoreImageAvailable[flightSlotIndex];
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &vkState->graphicsCommandBuffers[vkState->sync.currentSwapchainImageIndex];
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &vkState->sync.semaphoreRenderingComplete[flightSlotIndex];

  OVK_ATTEMPT(
    vkResetFences(vkState->device, 1, &vkState->sync.fenceFlightSlotAvailable[flightSlotIndex]),
    {
      OPAL_LOG_VK_ERROR("Failed to reset flight slot fence : %u\n", flightSlotIndex);
      return Opal_Failure_Vk_Render;
    });

  OVK_ATTEMPT(
    vkQueueSubmit(vkState->queueGraphics, 1, &submitInfo, vkState->sync.fenceFlightSlotAvailable[flightSlotIndex]),
    {
      OPAL_LOG_VK_ERROR("Failed to submit queue\n");
      return Opal_Failure_Vk_Render;
    });

  VkPresentInfoKHR presentInfo = { 0 };
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = NULL;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &vkState->swapchain.swapchain;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &vkState->sync.semaphoreRenderingComplete[flightSlotIndex];
  presentInfo.pImageIndices = &vkState->sync.currentSwapchainImageIndex;

  renderResult = vkQueuePresentKHR(vkState->queuePresent, &presentInfo);
  if (renderResult == VK_ERROR_OUT_OF_DATE_KHR)
  {
    OPAL_LOG_VK_ERROR("Need to update components for resize\n");
    return Opal_Failure_Vk_Render;
  }
  else if (renderResult != VK_SUCCESS)
  {
    return Opal_Failure_Vk_Render;
  }

  vkState->sync.currentFlightIndex = (vkState->sync.currentFlightIndex + 1) % maxFlightSlotCount;

  return Opal_Success;
}
