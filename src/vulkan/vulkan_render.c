
#include "src/vulkan/vulkan_common.h"

OpalResult OpalVulkanRenderBegin()
{
  VkCommandBufferBeginInfo beginInfo;
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  beginInfo.pNext = NULL;
  beginInfo.pInheritanceInfo = NULL;

  OPAL_ATTEMPT_VK(vkBeginCommandBuffer(g_OpalState.api.vk.currentRenderInfo.cmd, &beginInfo));

  return Opal_Success;
}

OpalResult OpalVulkanRenderEnd()
{
  OPAL_ATTEMPT_VK(vkEndCommandBuffer(g_OpalState.api.vk.currentRenderInfo.cmd));

  VkSubmitInfo submitInfo;
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext = NULL;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &g_OpalState.api.vk.currentRenderInfo.cmd;
  submitInfo.pWaitDstStageMask = NULL;
  submitInfo.waitSemaphoreCount = 0;
  submitInfo.pWaitSemaphores = NULL;
  submitInfo.signalSemaphoreCount = 0;
  submitInfo.pSignalSemaphores = NULL;

  OPAL_ATTEMPT_VK(vkQueueSubmit(g_OpalState.api.vk.queueGraphics, 1, &submitInfo, VK_NULL_HANDLE));

  return Opal_Success;
}

static int curIndex = 0;

OpalResult OpalVulkanRenderToWindowBegin(OpalWindow* pWindow)
{
  OpalVulkanWindow* window = &pWindow->api.vk;

  OPAL_ATTEMPT_VK(vkWaitForFences(g_OpalState.api.vk.device, 1, &window->pFenceFrameAvailable[window->currentSyncIndex], VK_TRUE, UINT64_MAX));

  OPAL_ATTEMPT_VK(vkAcquireNextImageKHR(
    g_OpalState.api.vk.device,
    window->swapchain,
    UINT64_MAX,
    window->pSemaphoresImageAvailable[window->currentSyncIndex],
    VK_NULL_HANDLE, //window->fenceNextImageRetrieved,
    &window->currentImageIndex));

  //OPAL_ATTEMPT_VK(vkWaitForFences(g_OpalState.api.vk.device, 1, &window->fenceNextImageRetrieved, VK_TRUE, UINT64_MAX));
  //OPAL_ATTEMPT_VK(vkResetFences(g_OpalState.api.vk.device, 1, &window->fenceNextImageRetrieved));

  curIndex = window->currentImageIndex;

  VkCommandBufferBeginInfo beginInfo;
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  beginInfo.pNext = NULL;
  beginInfo.pInheritanceInfo = NULL;

  g_OpalState.api.vk.currentRenderInfo.cmd = pWindow->api.vk.pCommandBuffers[window->currentSyncIndex];
  OPAL_ATTEMPT_VK(vkBeginCommandBuffer(pWindow->api.vk.pCommandBuffers[window->currentSyncIndex], &beginInfo));

  return Opal_Success;
}

OpalResult OpalVulkanRenderToWindowEnd(OpalWindow* pWindow)
{
  OpalVulkanWindow* window = &pWindow->api.vk;

  OPAL_ATTEMPT_VK(vkEndCommandBuffer(pWindow->api.vk.pCommandBuffers[window->currentSyncIndex]));

  // Render ==========
  VkSubmitInfo submitInfo;
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext = NULL;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &g_OpalState.api.vk.currentRenderInfo.cmd;
  submitInfo.pWaitDstStageMask = NULL;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &window->pSemaphoresImageAvailable[window->currentImageIndex];
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &window->pSemaphoresRenderComplete[window->currentSyncIndex];

  OPAL_ATTEMPT_VK(vkResetFences(g_OpalState.api.vk.device, 1, &window->pFenceFrameAvailable[window->currentSyncIndex]));
  OPAL_ATTEMPT_VK(vkQueueSubmit(g_OpalState.api.vk.queueGraphics, 1, &submitInfo, window->pFenceFrameAvailable[window->currentSyncIndex]));

  // Present ==========
  VkPresentInfoKHR presentInfo;
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = NULL;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &window->pSemaphoresRenderComplete[window->currentSyncIndex];
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &window->swapchain;
  presentInfo.pImageIndices = &window->currentImageIndex;

  OPAL_ATTEMPT_VK(vkQueuePresentKHR(g_OpalState.api.vk.queuePresent, &presentInfo));

  window->currentSyncIndex = (window->currentSyncIndex + 1) % window->imageCount;

  return Opal_Success;
}

void OpalVulkanRenderRenderpassBegin(const OpalRenderpass* pRenderpass, const OpalFramebuffer* pFramebuffer)
{
  static VkClearValue clearValues[3] = {
    { 1.0f, 0.0f, 0.0f, 1.0f},
    { 0.0f, 1.0f, 0.0f, 1.0f},
    { 0.0f, 0.0f, 1.0f, 1.0f},
  };

  VkRenderPassBeginInfo beginInfo;
  beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  beginInfo.pNext = NULL;

  beginInfo.renderPass = pRenderpass->api.vk.renderpass;
  beginInfo.clearValueCount = pRenderpass->attachmentCount;
  beginInfo.pClearValues = &clearValues[curIndex]; //pRenderpass->api.vk.pClearValues;

  beginInfo.framebuffer = pFramebuffer->api.vk.framebuffer;
  beginInfo.renderArea.extent = (VkExtent2D){ pFramebuffer->width, pFramebuffer->height };
  beginInfo.renderArea.offset = (VkOffset2D){ 0, 0 };

  vkCmdBeginRenderPass(g_OpalState.api.vk.currentRenderInfo.cmd, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void OpalVulkanRenderRenderpassEnd(const OpalRenderpass* pRenderpass)
{
  vkCmdEndRenderPass(g_OpalState.api.vk.currentRenderInfo.cmd);
}
