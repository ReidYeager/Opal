
#include "src/vulkan/vulkan_common.h"

// Declarations
// ============================================================

// Synchronization ==========
//OpalResult  OpalVulkanFenceInit                 (OpalFence* pFence, bool startSignaled)
//void        OpalVulkanFenceShutdown             (OpalFence* pFence)
//OpalResult  OpalVulkanSemaphoreInit             (OpalSemaphore* pSemaphore)
//void        OpalVulkanSemaphoreShutdown         (OpalSemaphore* pSemaphore)

// Begin/End ==========
//OpalResult OpalVulkanRenderBegin                ()
//OpalResult OpalVulkanRenderEnd                  (OpalSyncPack syncInfo)
//OpalResult OpalVulkanRenderToWindowBegin        (OpalWindow* pWindow)
//OpalResult OpalVulkanRenderToWindowEnd          (OpalWindow* pWindow)

// Objects ==========
//void       OpalVulkanRenderRenderpassBegin      (const OpalRenderpass* pRenderpass, const OpalFramebuffer* pFramebuffer)
//void       OpalVulkanRenderRenderpassEnd        (const OpalRenderpass* pRenderpass)
//void       OpalVulkanRenderBindShaderGroup      (const OpalShaderGroup* pGroup)
//void       OpalVulkanRenderSetViewportDimensions(uint32_t width, uint32_t height)
//void       OpalVulkanRenderSetPushConstant      (const void* data)
//void       OpalVulkanRenderBindShaderInput      (const OpalShaderInput* pInput)
//void       OpalVulkanRenderMesh                 (const OpalMesh* pMesh)

// Synchronization
// ============================================================

OpalResult OpalVulkanFenceInit(OpalFence* pFence, bool startSignaled)
{
  VkFenceCreateInfo createInfo;
  createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = startSignaled * VK_FENCE_CREATE_SIGNALED_BIT;

  OPAL_ATTEMPT_VK(vkCreateFence(g_ovkState->device, &createInfo, NULL, &pFence->api.vk.fence));

  return Opal_Success;
}

void OpalVulkanFenceShutdown(OpalFence* pFence)
{
  vkDestroyFence(g_ovkState->device, pFence->api.vk.fence, NULL);
}

OpalResult OpalVulkanSemaphoreInit(OpalSemaphore* pSemaphore)
{
  VkSemaphoreCreateInfo createInfo;
  createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;

  OPAL_ATTEMPT_VK(vkCreateSemaphore(g_ovkState->device, &createInfo, NULL, &pSemaphore->api.vk.semaphore));

  return Opal_Success;
}

void OpalVulkanSemaphoreShutdown(OpalSemaphore* pSemaphore)
{
  vkDestroySemaphore(g_ovkState->device, pSemaphore->api.vk.semaphore, NULL);
}

// Begin/End
// ============================================================

OpalResult OpalVulkanRenderBegin()
{
  g_ovkState->renderState.curIndex = (g_ovkState->renderState.curIndex + 1) % g_ovkState->renderState.cmdCount;
  g_ovkState->renderState.curCmd = g_ovkState->renderState.pCmdBuffers[g_ovkState->renderState.curIndex];
  g_ovkState->renderState.curFence = g_ovkState->renderState.pCmdAvailableFence[g_ovkState->renderState.curIndex];

  vkWaitForFences(g_ovkState->device, 1, &g_ovkState->renderState.curFence, VK_TRUE, UINT64_MAX);
  vkResetFences(g_ovkState->device, 1, &g_ovkState->renderState.curFence);

  VkCommandBufferBeginInfo beginInfo = { 0 };
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.pNext = NULL;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  beginInfo.pInheritanceInfo = NULL;

  OPAL_ATTEMPT_VK(vkBeginCommandBuffer(g_ovkState->renderState.curCmd, &beginInfo));

  return Opal_Success;
}

OpalResult OpalVulkanRenderEnd(OpalSyncPack syncInfo)
{
  OPAL_ATTEMPT_VK(vkEndCommandBuffer(g_ovkState->renderState.curCmd));

  VkPipelineStageFlags stages[4] =
  {
    VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
    VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
    VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
    VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT
  };

  VkSubmitInfo submitInfo = { 0 };
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext = NULL;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &g_ovkState->renderState.curCmd;
  submitInfo.waitSemaphoreCount = syncInfo.waitCount;
  submitInfo.pWaitSemaphores = (VkSemaphore*)syncInfo.pWaitSemaphores;
  submitInfo.pWaitDstStageMask = stages;
  submitInfo.signalSemaphoreCount = syncInfo.signalCount;
  submitInfo.pSignalSemaphores = (VkSemaphore*)syncInfo.pSignalSemaphores;

  OPAL_ATTEMPT_VK(vkQueueSubmit(g_ovkState->queueGraphics, 1, &submitInfo, g_ovkState->renderState.curFence));

  return Opal_Success;
}

OpalResult OpalVulkanRenderToWindowBegin(OpalWindow* pWindow)
{
  OpalVulkanWindow* window = &pWindow->api.vk;

  OPAL_ATTEMPT_VK(vkAcquireNextImageKHR(
    g_ovkState->device,
    window->swapchain,
    UINT64_MAX,
    window->pImageAvailableSems[window->syncIndex],
    VK_NULL_HANDLE,
    &window->imageIndex));

  OPAL_ATTEMPT(OpalVulkanRenderBegin());

  return Opal_Success;
}

OpalResult OpalVulkanRenderToWindowEnd(OpalWindow* pWindow)
{
  OpalVulkanWindow* window = &pWindow->api.vk;

  OpalSyncPack pack;
  pack.waitCount = 1;
  pack.pWaitSemaphores = (OpalSemaphore*)&window->pImageAvailableSems[window->syncIndex];
  pack.signalCount = 1;
  pack.pSignalSemaphores = (OpalSemaphore*)&window->pRenderCompleteSems[window->syncIndex];

  OPAL_ATTEMPT(OpalVulkanRenderEnd(pack));

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

// Objects
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

  vkCmdBeginRenderPass(g_ovkState->renderState.curCmd, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

  for (int i = 0; i < pRenderpass->attachmentCount; i++)
  {
    pFramebuffer->api.vk.pImagePointers[i]->api.vk.layout = pRenderpass->api.vk.pFinalLayouts[i];
  }

  OpalVulkanRenderSetViewportDimensions(pFramebuffer->width, pFramebuffer->height);
}

void OpalVulkanRenderRenderpassEnd(const OpalRenderpass* pRenderpass)
{
  vkCmdEndRenderPass(g_ovkState->renderState.curCmd);
}

void OpalVulkanRenderRenderpassNext(const OpalRenderpass* pRenderpass)
{
  vkCmdNextSubpass(g_ovkState->renderState.curCmd, VK_SUBPASS_CONTENTS_INLINE);
}

void OpalVulkanRenderBindShaderGroup(const OpalShaderGroup* pGroup)
{
  g_ovkState->renderState.bindPoint = pGroup->type == Opal_Group_Graphics ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE;
  vkCmdBindPipeline(g_ovkState->renderState.curCmd, g_ovkState->renderState.bindPoint, pGroup->api.vk.pipeline);

  g_ovkState->renderState.layout = pGroup->api.vk.pipelineLayout;
  g_ovkState->renderState.pushConstSize = pGroup->pushConstSize;
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

  vkCmdSetViewport(g_ovkState->renderState.curCmd, 0, 1, &viewport);
  vkCmdSetScissor(g_ovkState->renderState.curCmd, 0, 1, &scissor);
}

void OpalVulkanRenderSetPushConstant(const void* data)
{
  vkCmdPushConstants(
    g_ovkState->renderState.curCmd,
    g_ovkState->renderState.layout,
    VK_SHADER_STAGE_ALL,
    0,
    g_ovkState->renderState.pushConstSize,
    data);
}

void OpalVulkanRenderBindShaderInput(const OpalShaderInput* pInput, uint32_t setIndex)
{
  vkCmdBindDescriptorSets(
    g_ovkState->renderState.curCmd,
    g_ovkState->renderState.bindPoint,
    g_ovkState->renderState.layout,
    setIndex, 1, &pInput->api.vk.set,
    0, NULL);
}

void OpalVulkanRenderMesh(const OpalMesh* pMesh)
{
  VkDeviceSize offset = 0;
  vkCmdBindIndexBuffer(g_ovkState->renderState.curCmd, pMesh->indexBuffer.api.vk.buffer, 0, VK_INDEX_TYPE_UINT32);
  vkCmdBindVertexBuffers(g_ovkState->renderState.curCmd, 0, 1, &pMesh->vertexBuffer.api.vk.buffer, &offset);
  vkCmdDrawIndexed(g_ovkState->renderState.curCmd, pMesh->indexCount, 1, 0, 0, 0);
}

void OpalVulkanRenderComputeDispatch(uint32_t x, uint32_t y, uint32_t z)
{
  vkCmdDispatch(g_ovkState->renderState.curCmd, x, y, z);
}
