
#include "src/vulkan/vulkan_common.h"

// Declarations
// ============================================================

// Begin/End ==========
//OpalResult OpalVulkanRenderBegin          ()
//OpalResult OpalVulkanRenderEnd            ()
//OpalResult OpalVulkanRenderToWindowBegin  (OpalWindow* pWindow)
//OpalResult OpalVulkanRenderToWindowEnd    (OpalWindow* pWindow)

// Object commands ==========
//void       OpalVulkanRenderRenderpassBegin(const OpalRenderpass* pRenderpass, const OpalFramebuffer* pFramebuffer)
//void       OpalVulkanRenderRenderpassEnd  (const OpalRenderpass* pRenderpass)
//void       OpalVulkanRenderBindShaderGroup(const OpalShaderGroup* pGroup)
//void       OpalVulkanRenderBindShaderInput(const OpalShaderInput* pInput)
//void       OpalVulkanRenderMesh           (const OpalMesh* pMesh)


// Begin/End
// ============================================================

OpalResult OpalVulkanRenderBegin()
{
  VkCommandBufferBeginInfo beginInfo = { 0 };
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.pNext = NULL;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  beginInfo.pInheritanceInfo = NULL;

  OPAL_ATTEMPT_VK(vkBeginCommandBuffer(g_ovkState->renderState.cmd, &beginInfo));

  return Opal_Success;
}

OpalResult OpalVulkanRenderEnd()
{
  OPAL_ATTEMPT_VK(vkEndCommandBuffer(g_ovkState->renderState.cmd));

  VkSubmitInfo submitInfo = { 0 };
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext = NULL;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &g_ovkState->renderState.cmd;
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

  VkCommandBufferBeginInfo beginInfo = { 0 };
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.pNext = NULL;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  beginInfo.pInheritanceInfo = NULL;

  g_ovkState->renderState.cmd = pWindow->api.vk.pCommandBuffers[window->syncIndex];
  OPAL_ATTEMPT_VK(vkBeginCommandBuffer(pWindow->api.vk.pCommandBuffers[window->syncIndex], &beginInfo));

  return Opal_Success;
}

OpalResult OpalVulkanRenderToWindowEnd(OpalWindow* pWindow)
{
  OpalVulkanWindow* window = &pWindow->api.vk;

  OPAL_ATTEMPT_VK(vkEndCommandBuffer(pWindow->api.vk.pCommandBuffers[window->syncIndex]));

  // Render ==========
  VkSubmitInfo submitInfo = { 0 };
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext = NULL;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &g_ovkState->renderState.cmd;
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
  VkPresentInfoKHR presentInfo = { 0 };
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

void OpalVulkanRenderSetViewportDimensions(uint32_t width, uint32_t height)
{
  VkViewport viewport = { 0, 0, 1, 1, 0, 1 };
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = width;
  viewport.height = height;
  viewport.minDepth = 0;
  viewport.maxDepth = 1;

  VkRect2D scissor = { 0 };
  scissor.extent = (VkExtent2D){ width, height };
  scissor.offset = (VkOffset2D){ 0, 0 };

  vkCmdSetViewport(g_ovkState->renderState.cmd, 0, 1, &viewport);
  vkCmdSetScissor(g_ovkState->renderState.cmd, 0, 1, &scissor);
}

// Object commands
// ============================================================

void OpalVulkanRenderRenderpassBegin(const OpalRenderpass* pRenderpass, const OpalFramebuffer* pFramebuffer)
{
  VkRenderPassBeginInfo beginInfo = { 0 };
  beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  beginInfo.pNext = NULL;

  beginInfo.renderPass = pRenderpass->api.vk.renderpass;
  beginInfo.clearValueCount = pRenderpass->attachmentCount;
  beginInfo.pClearValues = pRenderpass->api.vk.pClearValues;

  beginInfo.framebuffer = pFramebuffer->api.vk.framebuffer;
  beginInfo.renderArea.extent = (VkExtent2D){ pFramebuffer->width, pFramebuffer->height };
  beginInfo.renderArea.offset = (VkOffset2D){ 0, 0 };

  vkCmdBeginRenderPass(g_ovkState->renderState.cmd, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void OpalVulkanRenderRenderpassEnd(const OpalRenderpass* pRenderpass)
{
  vkCmdEndRenderPass(g_ovkState->renderState.cmd);
}

void OpalVulkanRenderBindShaderGroup(const OpalShaderGroup* pGroup)
{
  vkCmdBindPipeline(g_ovkState->renderState.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pGroup->api.vk.pipeline);

  g_ovkState->renderState.layout = pGroup->api.vk.pipelineLayout;
  g_ovkState->renderState.pushConstSize = pGroup->pushConstSize;
}

void OpalVulkanRenderBindShaderInput(const OpalShaderInput* pInput)
{
  vkCmdBindDescriptorSets(
    g_ovkState->renderState.cmd,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    g_ovkState->renderState.layout,
    0, 1, &pInput->api.vk.set,
    0, NULL);
}

void OpalVulkanRenderSetPushConstant(const void* data)
{
  vkCmdPushConstants(
    g_ovkState->renderState.cmd,
    g_ovkState->renderState.layout,
    VK_SHADER_STAGE_ALL_GRAPHICS,
    0,
    g_ovkState->renderState.pushConstSize,
    data);
}

void OpalVulkanRenderMesh(const OpalMesh* pMesh)
{
  VkDeviceSize offset = 0;
  vkCmdBindIndexBuffer(g_ovkState->renderState.cmd, pMesh->indexBuffer.api.vk.buffer, 0, VK_INDEX_TYPE_UINT32);
  vkCmdBindVertexBuffers(g_ovkState->renderState.cmd, 0, 1, &pMesh->vertexBuffer.api.vk.buffer, &offset);
  vkCmdDrawIndexed(g_ovkState->renderState.cmd, pMesh->indexCount, 1, 0, 0, 0);
}
