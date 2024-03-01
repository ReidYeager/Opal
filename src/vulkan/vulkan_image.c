
#include "src/vulkan/vulkan_common.h"

// Declarations
// ============================================================

// Core ==========
// OpalResult OpalVulkanImageInit      (OpalImage* pImage, OpalImageInitInfo initInfo)
// void       OpalVulkanImageShutdown  (OpalImage* pImage)

// Tools ==========
// OpalResult ImageTransitionLayout_Ovk(OpalImage* pImage, VkImageLayout newLayout)

// Core
// ============================================================

OpalResult OpalVulkanImageInit(OpalImage* pImage, OpalImageInitInfo initInfo)
{
  
  return Opal_Success;
}

void OpalVulkanImageShutdown(OpalImage* pImage)
{
  
}

// Tools
// ============================================================

OpalResult ImageTransitionLayout_Ovk(OpalImage* pImage, VkImageLayout newLayout)
{
  VkImageMemoryBarrier memBarrier = { 0 };
  memBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  memBarrier.image = pImage->api.vk.image;
  memBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  memBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  memBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  memBarrier.subresourceRange.levelCount = pImage->mipCount;
  memBarrier.subresourceRange.baseMipLevel = 0;
  memBarrier.subresourceRange.layerCount = 1;
  memBarrier.subresourceRange.baseArrayLayer = 0;

  memBarrier.oldLayout = pImage->api.vk.layout;
  memBarrier.newLayout = newLayout;

  VkPipelineStageFlagBits srcStage, dstStage;

  switch (newLayout)
  {
  case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
  {
    srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    memBarrier.srcAccessMask = 0;

    dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    memBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  } break;
  case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
  {
    srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    memBarrier.srcAccessMask = 0;

    dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    memBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  } break;
  case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
  {
    srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    memBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    dstStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    memBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
  } break;
  case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
  {
    srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    memBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    memBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  } break;
  case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
  {
    srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    memBarrier.srcAccessMask = 0;

    dstStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    memBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
  } break;
  case 0:
  {
    return Opal_Success;
  }
  default:
  {
    OpalLogError("Unknown new image layout %d", newLayout);
    return Opal_Failure_Invalid_Input;
  }
  }

  VkCommandBuffer cmd;
  OPAL_ATTEMPT(BeginSingleUseCommandBuffer_Ovk(&cmd, g_ovkState->graphicsCommandPool));
  vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, NULL, 0, NULL, 1, &memBarrier);
  OPAL_ATTEMPT(EndSingleUseCommandBuffer_Ovk(cmd, g_ovkState->graphicsCommandPool, g_ovkState->queueGraphics));

  pImage->api.vk.layout = newLayout;

  return Opal_Success;
}
