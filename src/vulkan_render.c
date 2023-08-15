
#include "src/common.h"

VkCommandBuffer currentRenderCmd_Ovk;
VkPipelineLayout currentPipelineLayout_Ovk;
uint32_t syncIndex = 0;
uint32_t swapchainImageIndex = 0;
OvkSync_T* curSync = NULL;

OpalResult OvkRenderBegin()
{
  curSync = &oState.window.vk.pSync[syncIndex];

  OVK_ATTEMPT(vkWaitForFences(oState.vk.device, 1, &curSync->fenceFrameAvailable, VK_TRUE, UINT64_MAX));

  OVK_ATTEMPT(vkAcquireNextImageKHR(oState.vk.device, oState.window.vk.swapchain.swapchain, UINT64_MAX, curSync->semImageAvailable, VK_NULL_HANDLE, &swapchainImageIndex));

  VkCommandBufferBeginInfo beginInfo = { 0 };
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  currentRenderCmd_Ovk = curSync->cmdBuffer;

  OVK_ATTEMPT(vkBeginCommandBuffer(currentRenderCmd_Ovk, &beginInfo));

  return Opal_Success;
}

OpalResult CopyRenderBufferToSwapchain_Ovk()
{
  VkImageSubresourceLayers subres = { 0 };
  subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subres.baseArrayLayer = 0;
  subres.layerCount = 1;
  subres.mipLevel = 0;

  VkImageCopy imageCopy = { 0 };
  imageCopy.extent = (VkExtent3D){ oState.window.extents.width, oState.window.extents.height, oState.window.extents.depth };
  imageCopy.srcOffset = (VkOffset3D){ 0, 0, 0 };
  imageCopy.srcSubresource = subres;
  imageCopy.dstOffset = (VkOffset3D){ 0, 0, 0 };
  imageCopy.dstSubresource = subres;

  OPAL_ATTEMPT(OvkTransitionImageLayout(oState.window.vk.swapchain.pImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL));

  VkCommandBuffer tmpcmd;
  OPAL_ATTEMPT(OvkBeginSingleUseCommand(oState.vk.transientCommandPool, &tmpcmd));
  vkCmdCopyImage(
    tmpcmd,
    oState.window.renderBufferImage->vk.image,
    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    oState.window.vk.swapchain.pImages[swapchainImageIndex],
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    1,
    &imageCopy);
  OPAL_ATTEMPT(OvkEndSingleUseCommand(oState.vk.transientCommandPool, oState.vk.queueTransfer, tmpcmd));

  OPAL_ATTEMPT(OvkTransitionImageLayout(oState.window.vk.swapchain.pImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR));

  return Opal_Success;
}

OpalResult OvkRenderEnd()
{
  OVK_ATTEMPT(vkEndCommandBuffer(currentRenderCmd_Ovk));

  VkPipelineStageFlags waitStages[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  VkSubmitInfo submitInfo = { 0 };
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext = NULL;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &curSync->semImageAvailable;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &currentRenderCmd_Ovk;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &curSync->semRenderComplete;

  OVK_ATTEMPT(vkResetFences(oState.vk.device, 1, &curSync->fenceFrameAvailable));

  OVK_ATTEMPT(vkQueueSubmit(oState.vk.queueGraphics, 1, &submitInfo, curSync->fenceFrameAvailable));

  // Present =====

  OPAL_ATTEMPT(CopyRenderBufferToSwapchain_Ovk());

  VkPresentInfoKHR presentInfo = { 0 };
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = NULL;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &curSync->semRenderComplete;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &oState.window.vk.swapchain.swapchain;
  presentInfo.pImageIndices = &swapchainImageIndex;

  OVK_ATTEMPT(vkQueuePresentKHR(oState.vk.queuePresent, &presentInfo));

  syncIndex = (syncIndex + 1) % oState.window.imageCount;

  return Opal_Success;
}

void OvkRenderBeginRenderpass(OpalRenderpass _renderpass, OpalFramebuffer _framebuffer)
{
  VkRenderPassBeginInfo beginInfo = { 0 };
  beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

  beginInfo.renderPass = _renderpass->vk.renderpass;
  beginInfo.clearValueCount = _renderpass->imageCount;
  beginInfo.pClearValues = (VkClearValue*)_renderpass->pClearValues;

  beginInfo.framebuffer = _framebuffer->vk.framebuffer;
  beginInfo.renderArea.extent = (VkExtent2D){ _framebuffer->extent.width, _framebuffer->extent.height };
  beginInfo.renderArea.offset = (VkOffset2D){ 0, 0 };

  vkCmdBeginRenderPass(currentRenderCmd_Ovk, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void OvkRenderEndRenderpass(OpalRenderpass _renderpass)
{
  vkCmdEndRenderPass(currentRenderCmd_Ovk);
}

void OvkRenderBindInputSet(OpalInputSet _set, uint32_t _setIndex)
{
  vkCmdBindDescriptorSets(
    currentRenderCmd_Ovk,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    currentPipelineLayout_Ovk,
    _setIndex,
    1,
    &_set->vk.descriptorSet,
    0,
    NULL);
}

void OvkRenderBindMaterial(OpalMaterial _material)
{
  vkCmdBindPipeline(currentRenderCmd_Ovk, VK_PIPELINE_BIND_POINT_GRAPHICS, _material->vk.pipeline);

  currentPipelineLayout_Ovk = _material->vk.pipelineLayout;
}

void OvkRenderMesh(OpalMesh _mesh)
{
  VkDeviceSize offset = 0;
  vkCmdBindIndexBuffer(currentRenderCmd_Ovk, _mesh->indexBuffer->vk.buffer, 0, VK_INDEX_TYPE_UINT32);
  vkCmdBindVertexBuffers(currentRenderCmd_Ovk, 0, 1, &_mesh->vertBuffer->vk.buffer, &offset);
  vkCmdDrawIndexed(currentRenderCmd_Ovk, _mesh->indexCount, 1, 0, 0, 0);
}
