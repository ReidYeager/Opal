
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

OpalResult TransitionRenderBufferImage_Ovk(VkImage _image, VkImageLayout _layout, bool _toWritable)
{
  VkCommandBuffer cmd;
  OPAL_ATTEMPT(OvkBeginSingleUseCommand(oState.vk.graphicsCommandPool, &cmd));

  VkImageMemoryBarrier memBarrier = { 0 };
  memBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  memBarrier.pNext = NULL;
  memBarrier.oldLayout = _layout;
  memBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  memBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  memBarrier.image = _image;
  memBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  memBarrier.subresourceRange.levelCount = 1;
  memBarrier.subresourceRange.baseMipLevel = 0;
  memBarrier.subresourceRange.layerCount = 1;
  memBarrier.subresourceRange.baseArrayLayer = 0;

  VkPipelineStageFlagBits srcStage, dstStage;

  if (_toWritable)
  {
    memBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    memBarrier.srcAccessMask = 0;
    memBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else
  {
    memBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    memBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    memBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dstStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
  }

  vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, NULL, 0, NULL, 1, &memBarrier);
  OPAL_ATTEMPT(OvkEndSingleUseCommand(oState.vk.graphicsCommandPool, oState.vk.queueGraphics, cmd));

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

  TransitionRenderBufferImage_Ovk(oState.window.renderBufferImage->vk.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, true);
  OvkTransitionImageLayout(oState.window.vk.swapchain.pImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, true);

  VkCommandBuffer tmpcmd;
  OvkBeginSingleUseCommand(oState.vk.transientCommandPool, &tmpcmd);
  vkCmdCopyImage(
    tmpcmd,
    oState.window.renderBufferImage->vk.image,
    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,// VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    oState.window.vk.swapchain.pImages[swapchainImageIndex],
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    1,
    &imageCopy);
  OvkEndSingleUseCommand(oState.vk.transientCommandPool, oState.vk.queueTransfer, tmpcmd);

  OvkTransitionImageLayout(oState.window.vk.swapchain.pImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, false);
  TransitionRenderBufferImage_Ovk(oState.window.renderBufferImage->vk.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, false);

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

void OvkRenderBindMaterial(OpalMaterial _material)
{
  vkCmdBindPipeline(currentRenderCmd_Ovk, VK_PIPELINE_BIND_POINT_GRAPHICS, _material->vk.pipeline);

  currentPipelineLayout_Ovk = _material->vk.pipelineLayout;

  vkCmdBindDescriptorSets(
    currentRenderCmd_Ovk,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    currentPipelineLayout_Ovk,
    0,
    1,
    &_material->vk.descriptorSet,
    0,
    NULL);
}

void OvkRenderVertices(uint32_t _count)
{
  vkCmdDraw(currentRenderCmd_Ovk, _count, 1, 0, 0);
}
