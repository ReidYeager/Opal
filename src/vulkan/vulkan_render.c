
#include "src/vulkan/vulkan_common.h"

// Declarations
// ============================================================

// Begin/End ==========
// OpalResult OpalVulkanRenderBegin          ()
// OpalResult OpalVulkanRenderEnd            ()
// OpalResult OpalVulkanRenderToWindowBegin  (OpalWindow* pWindow)
// OpalResult OpalVulkanRenderToWindowEnd    (OpalWindow* pWindow)

// Object commands ==========
// void       OpalVulkanRenderRenderpassBegin(const OpalRenderpass* pRenderpass, const OpalFramebuffer* pFramebuffer)
// void       OpalVulkanRenderRenderpassEnd  (const OpalRenderpass* pRenderpass)

// Begin/End
// ============================================================

OpalResult OpalVulkanRenderBegin()
{
  VkCommandBufferBeginInfo beginInfo;
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  beginInfo.pNext = NULL;
  beginInfo.pInheritanceInfo = NULL;

  OPAL_ATTEMPT_VK(vkBeginCommandBuffer(g_ovkState->renderCurrentCmd, &beginInfo));

  return Opal_Success;
}

OpalResult OpalVulkanRenderEnd()
{
  OPAL_ATTEMPT_VK(vkEndCommandBuffer(g_ovkState->renderCurrentCmd));

  VkSubmitInfo submitInfo;
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext = NULL;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &g_ovkState->renderCurrentCmd;
  submitInfo.pWaitDstStageMask = NULL;
  submitInfo.waitSemaphoreCount = 0;
  submitInfo.pWaitSemaphores = NULL;
  submitInfo.signalSemaphoreCount = 0;
  submitInfo.pSignalSemaphores = NULL;

  OPAL_ATTEMPT_VK(vkQueueSubmit(g_ovkState->queueGraphics, 1, &submitInfo, VK_NULL_HANDLE));

  return Opal_Success;
}

OpalResult OpalVulkanRenderToWindowBegin(OpalWindow* pWindow)
{
  OpalVulkanWindow* window = &pWindow->api.vk;

  OPAL_ATTEMPT_VK(
    vkWaitForFences(g_ovkState->device, 1, &window->pCmdAvailableFences[window->syncIndex], VK_TRUE, UINT64_MAX));

  OPAL_ATTEMPT_VK(vkAcquireNextImageKHR(
    g_ovkState->device,
    window->swapchain,
    UINT64_MAX,
    window->pImageAvailableSems[window->syncIndex],
    VK_NULL_HANDLE,
    &window->imageIndex));

  VkCommandBufferBeginInfo beginInfo;
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  beginInfo.pNext = NULL;
  beginInfo.pInheritanceInfo = NULL;

  g_ovkState->renderCurrentCmd = pWindow->api.vk.pCommandBuffers[window->syncIndex];
  OPAL_ATTEMPT_VK(vkBeginCommandBuffer(pWindow->api.vk.pCommandBuffers[window->syncIndex], &beginInfo));

  return Opal_Success;
}

OpalResult OpalVulkanRenderToWindowEnd(OpalWindow* pWindow)
{
  OpalVulkanWindow* window = &pWindow->api.vk;

  OPAL_ATTEMPT_VK(vkEndCommandBuffer(pWindow->api.vk.pCommandBuffers[window->syncIndex]));

  // Render ==========
  VkSubmitInfo submitInfo;
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext = NULL;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &g_ovkState->renderCurrentCmd;
  submitInfo.pWaitDstStageMask = NULL;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &window->pImageAvailableSems[window->syncIndex];
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &window->pRenderCompleteSems[window->syncIndex];

  OPAL_ATTEMPT_VK(
    vkResetFences(g_ovkState->device, 1, &window->pCmdAvailableFences[window->syncIndex]));
  OPAL_ATTEMPT_VK(
    vkQueueSubmit(g_ovkState->queueGraphics, 1, &submitInfo, window->pCmdAvailableFences[window->syncIndex]));

  // Present ==========
  VkResult presentResult;
  VkPresentInfoKHR presentInfo;
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = NULL;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &window->pRenderCompleteSems[window->syncIndex];
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &window->swapchain;
  presentInfo.pImageIndices = &window->imageIndex;
  presentInfo.pResults = &presentResult;

  OPAL_ATTEMPT_VK(vkQueuePresentKHR(g_ovkState->queuePresent, &presentInfo));

  window->syncIndex = (window->syncIndex + 1) % window->imageCount;

  return Opal_Success;
}

// Object commands
// ============================================================

void OpalVulkanRenderRenderpassBegin(const OpalRenderpass* pRenderpass, const OpalFramebuffer* pFramebuffer)
{
  VkRenderPassBeginInfo beginInfo;
  beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  beginInfo.pNext = NULL;

  beginInfo.renderPass = pRenderpass->api.vk.renderpass;
  beginInfo.clearValueCount = pRenderpass->attachmentCount;
  beginInfo.pClearValues = pRenderpass->api.vk.pClearValues;

  beginInfo.framebuffer = pFramebuffer->api.vk.framebuffer;
  beginInfo.renderArea.extent = (VkExtent2D){ pFramebuffer->width, pFramebuffer->height };
  beginInfo.renderArea.offset = (VkOffset2D){ 0, 0 };

  vkCmdBeginRenderPass(g_ovkState->renderCurrentCmd, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void OpalVulkanRenderRenderpassEnd(const OpalRenderpass* pRenderpass)
{
  vkCmdEndRenderPass(g_ovkState->renderCurrentCmd);
}
