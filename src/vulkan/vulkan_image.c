
#include "src/vulkan/vulkan_common.h"
#include "include/opal.h" // Used for OpalVulkanImageFill

// Declarations
// ============================================================

// Core ==========
//OpalResult      OpalVulkanImageInit        (OpalImage* pImage, OpalImageInitInfo initInfo)
//void            OpalVulkanImageShutdown    (OpalImage* pImage)
OpalResult        InitImage_Ovk              (OpalImage* pImage, OpalImageInitInfo initInfo);
OpalResult        InitImageMemory_Ovk        (OpalImage* pImage);
OpalResult        InitView_Ovk               (OpalImage* pImage, OpalImageInitInfo initInfo);
OpalResult        InitSampler_Ovk            (OpalImage* pImage, OpalImageInitInfo initInfo);
//OpalResult      OpalVulkanImagePushData    (OpalImage* pImage, const void* data)
OpalResult        FillMipmaps_Ovk            (OpalImage* pImage);

// Tools ==========
//OpalResult      ImageTransitionLayout_Ovk  (OpalImage* pImage, VkImageLayout newLayout)
VkImageUsageFlags OpalImageUsageToVkFlags_Ovk(OpalImageUsageFlags opalFlags);
uint32_t          GetMemoryTypeIndex_Ovk     (uint32_t _mask, VkMemoryPropertyFlags _flags);
OpalResult        CopyBufferToImage_Ovk      (OpalBuffer* pBuffer, OpalImage* pImage);

// Core
// ============================================================

OpalResult OpalVulkanImageInit(OpalImage* pImage, OpalImageInitInfo initInfo)
{
  OPAL_ATTEMPT(InitImage_Ovk(pImage, initInfo));
  OPAL_ATTEMPT(InitImageMemory_Ovk(pImage));
  OPAL_ATTEMPT(InitView_Ovk(pImage, initInfo));

  pImage->usage = initInfo.usage;
  pImage->api.vk.layout = VK_IMAGE_LAYOUT_UNDEFINED;

  if (initInfo.usage & Opal_Image_Usage_Uniform)
  {
    OPAL_ATTEMPT(InitSampler_Ovk(pImage, initInfo));
  }
  else
  {
    pImage->api.vk.sampler = VK_NULL_HANDLE;
  }

  VkImageLayout newlayout;
  if (initInfo.usage & Opal_Image_Usage_Color)
    newlayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  else if (initInfo.usage & Opal_Image_Usage_Uniform)
    newlayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  else
    newlayout = VK_IMAGE_LAYOUT_UNDEFINED;

  OPAL_ATTEMPT(ImageTransitionLayout_Ovk(pImage, newlayout));

  return Opal_Success;
}

void OpalVulkanImageShutdown(OpalImage* pImage)
{
  vkFreeMemory(g_ovkState->device, pImage->api.vk.memory, NULL);
  vkDestroyImage(g_ovkState->device, pImage->api.vk.image, NULL);
  vkDestroyImageView(g_ovkState->device, pImage->api.vk.view, NULL);
  vkDestroySampler(g_ovkState->device, pImage->api.vk.sampler, NULL);
}

OpalResult InitImage_Ovk(OpalImage* pImage, OpalImageInitInfo initInfo)
{
  VkImageCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  createInfo.extent.width = initInfo.width;
  createInfo.extent.height = initInfo.height;
  createInfo.extent.depth = 1;
  createInfo.format = OpalFormatToVkFormat_Ovk(initInfo.format);
  createInfo.usage = OpalImageUsageToVkFlags_Ovk(initInfo.usage);
  createInfo.mipLevels = initInfo.mipCount;
  createInfo.arrayLayers = 1;
  createInfo.imageType = VK_IMAGE_TYPE_2D;
  createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.samples = VK_SAMPLE_COUNT_1_BIT;

  OPAL_ATTEMPT_VK(vkCreateImage(g_ovkState->device, &createInfo, NULL, &pImage->api.vk.image));

  pImage->api.vk.format = createInfo.format;

  return Opal_Success;
}

OpalResult InitImageMemory_Ovk(OpalImage* pImage)
{
  VkMemoryRequirements memReq = { 0 };
  vkGetImageMemoryRequirements(g_ovkState->device, pImage->api.vk.image, &memReq);

  VkMemoryAllocateInfo memAllocInfo = { 0 };
  memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memAllocInfo.allocationSize = memReq.size;
  memAllocInfo.memoryTypeIndex = GetMemoryTypeIndex_Ovk(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  OPAL_ATTEMPT_VK(vkAllocateMemory(g_ovkState->device, &memAllocInfo, NULL, &pImage->api.vk.memory));
  OPAL_ATTEMPT_VK(vkBindImageMemory(g_ovkState->device, pImage->api.vk.image, pImage->api.vk.memory, 0));

  return Opal_Success;
}

OpalResult InitView_Ovk(OpalImage* pImage, OpalImageInitInfo initInfo)
{
  VkImageViewCreateInfo vCreateInfo = { 0 };
  vCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  vCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  vCreateInfo.image = pImage->api.vk.image;
  vCreateInfo.format = pImage->api.vk.format;
  vCreateInfo.subresourceRange.aspectMask = 0;
  if (initInfo.usage & Opal_Image_Usage_Color)
    vCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
  if (initInfo.usage & Opal_Image_Usage_Depth)
    vCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
  if (initInfo.usage & Opal_Image_Usage_Uniform)
    vCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
  vCreateInfo.subresourceRange.levelCount = initInfo.mipCount;
  vCreateInfo.subresourceRange.baseMipLevel = 0;
  vCreateInfo.subresourceRange.layerCount = 1;
  vCreateInfo.subresourceRange.baseArrayLayer = 0;

  OPAL_ATTEMPT_VK(vkCreateImageView(g_ovkState->device, &vCreateInfo, NULL, &pImage->api.vk.view));

  return Opal_Success;
}

OpalResult InitSampler_Ovk(OpalImage* pImage, OpalImageInitInfo initInfo)
{
  VkSamplerCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  switch (initInfo.filter)
  {
  case Opal_Image_Filter_Linear:
  {
    createInfo.magFilter = VK_FILTER_LINEAR;
    createInfo.minFilter = VK_FILTER_LINEAR;
  } break;
  case Opal_Image_Filter_Point:
  default:
  {
    createInfo.magFilter = VK_FILTER_NEAREST;
    createInfo.minFilter = VK_FILTER_NEAREST;
  } break;
  }

  VkSamplerAddressMode sampleMode;
  switch (initInfo.sampleMode)
  {
  case Opal_Image_Sample_Reflect:
  {
    sampleMode = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
  } break;
  case Opal_Image_Sample_Clamp:
  {
    sampleMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  } break;
  case Opal_Image_Sample_Wrap:
  default:
  {
    sampleMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  } break;
  }

  createInfo.addressModeU = createInfo.addressModeV = createInfo.addressModeW = sampleMode;
  createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
  createInfo.unnormalizedCoordinates = VK_FALSE;
  createInfo.compareEnable = VK_FALSE;
  createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  createInfo.mipLodBias = 0.0f;
  createInfo.minLod = 0.0f;
  createInfo.maxLod = (float)initInfo.mipCount - 1.0f;
  createInfo.maxAnisotropy = 1.0f;

  OPAL_ATTEMPT_VK(vkCreateSampler(g_ovkState->device, &createInfo, NULL, &pImage->api.vk.sampler));

  return Opal_Success;
}

OpalResult OpalVulkanImagePushData(OpalImage* pImage, const void* data)
{
  OpalBufferInitInfo bufferInfo = { 0 };
  bufferInfo.size = OpalFormatToSize(pImage->format) * pImage->width * pImage->height;
  bufferInfo.usage = Opal_Buffer_Usage_Cpu_Read | Opal_Buffer_Usage_Transfer_Src;
  OpalBuffer buffer;
  OPAL_ATTEMPT(OpalBufferInit(&buffer, bufferInfo));
  OPAL_ATTEMPT(OpalBufferPushData(&buffer, data));

  VkImageLayout ogLayout = pImage->api.vk.layout;
  OPAL_ATTEMPT(ImageTransitionLayout_Ovk(pImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL));
  OPAL_ATTEMPT(CopyBufferToImage_Ovk(&buffer, pImage));
  OPAL_ATTEMPT(ImageTransitionLayout_Ovk(pImage, ogLayout));

  OpalBufferShutdown(&buffer);

  if (pImage->mipCount > 1)
  {
    OPAL_ATTEMPT(FillMipmaps_Ovk(pImage));
  }

  return Opal_Success;
}

OpalResult FillMipmaps_Ovk(OpalImage* pImage)
{
  OPAL_ATTEMPT(ImageTransitionLayout_Ovk(pImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL));

  VkCommandBuffer cmd;
  OPAL_ATTEMPT(BeginSingleUseCommandBuffer_Ovk(&cmd, g_ovkState->graphicsCommandPool));

  VkImageMemoryBarrier barrier = { 0 };
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.image = pImage->api.vk.image;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.subresourceRange.levelCount = 1;

  int32_t mipWidth = (int32_t)pImage->width;
  int32_t mipHeight = (int32_t)pImage->height;
  for (uint32_t i = 1; i < pImage->mipCount; i++)
  {
    barrier.subresourceRange.baseMipLevel = i - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    vkCmdPipelineBarrier(
      cmd,
      VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
      0, NULL,
      0, NULL,
      1, &barrier);

    VkImageBlit blit = { 0 };
    blit.srcOffsets[0] = (VkOffset3D){ 0, 0, 0 };
    blit.srcOffsets[1] = (VkOffset3D){ mipWidth, mipHeight, 1 };
    blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.srcSubresource.mipLevel = i - 1;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.srcSubresource.layerCount = 1;
    blit.dstOffsets[0] = (VkOffset3D){ 0, 0, 0 };
    blit.dstOffsets[1] = (VkOffset3D){ (mipWidth > 1) ? (mipWidth / 2) : 1, (mipHeight > 1) ? (mipHeight / 2) : 1, 1 };
    blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.dstSubresource.mipLevel = i;
    blit.dstSubresource.baseArrayLayer = 0;
    blit.dstSubresource.layerCount = 1;
    vkCmdBlitImage(
      cmd,
      pImage->api.vk.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      pImage->api.vk.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1, &blit, VK_FILTER_LINEAR);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(
      cmd,
      VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
      0, NULL,
      0, NULL,
      1, &barrier);

    if (mipWidth > 1)
      mipWidth /= 2;
    if (mipHeight > 1)
      mipHeight /= 2;
  }

  barrier.subresourceRange.baseMipLevel = pImage->mipCount - 1;
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  vkCmdPipelineBarrier(
    cmd,
    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
    0, NULL,
    0, NULL,
    1, &barrier);

  pImage->api.vk.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  OPAL_ATTEMPT(EndSingleUseCommandBuffer_Ovk(cmd, g_ovkState->graphicsCommandPool, g_ovkState->queueGraphics));
  return Opal_Success;
}

// Tools
// ============================================================

OpalResult ImageTransitionLayout_Ovk(OpalImage* pImage, VkImageLayout newLayout)
{
  VkImageMemoryBarrier memBarrier = { 0 };
  memBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  memBarrier.pNext = NULL;
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

VkImageUsageFlags OpalImageUsageToVkFlags_Ovk(OpalImageUsageFlags opalFlags)
{
  #define uses(o, vk) ((opalFlags & o) != 0) * vk
  VkImageUsageFlags vkUsage = 0;
  vkUsage |= uses(Opal_Image_Usage_Color, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
  vkUsage |= uses(Opal_Image_Usage_Depth, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
  vkUsage |= uses(Opal_Image_Usage_Copy_Src, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
  vkUsage |= uses(Opal_Image_Usage_Copy_Dst, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
  vkUsage |= uses(Opal_Image_Usage_Uniform, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
  return vkUsage;
  #undef uses
}

uint32_t GetMemoryTypeIndex_Ovk(uint32_t _mask, VkMemoryPropertyFlags _flags)
{
  const VkPhysicalDeviceMemoryProperties* memProps = &g_ovkState->gpu.memoryProperties;
  for (uint32_t i = 0; i < memProps->memoryTypeCount; i++)
  {
    if (_mask & (1 << i) && (memProps->memoryTypes[i].propertyFlags & _flags) == _flags)
    {
      return i;
    }
  }

  OpalLog("Vulkan image failed to find a suitable memory type index");
  return ~0u;
}

OpalResult CopyBufferToImage_Ovk(OpalBuffer* pBuffer, OpalImage* pImage)
{
  VkCommandBuffer cmd;
  OPAL_ATTEMPT(BeginSingleUseCommandBuffer_Ovk(&cmd, g_ovkState->graphicsCommandPool));

  VkBufferImageCopy copyRegion = { 0 };
  copyRegion.bufferOffset = 0;
  copyRegion.bufferRowLength = pImage->width;
  copyRegion.bufferImageHeight = pImage->height;
  copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  copyRegion.imageSubresource.mipLevel = 0;
  copyRegion.imageSubresource.layerCount = 1;
  copyRegion.imageSubresource.baseArrayLayer = 0;
  copyRegion.imageOffset = (VkOffset3D){ 0, 0, 0 };
  copyRegion.imageExtent = (VkExtent3D){ pImage->width, pImage->height, 1 };

  vkCmdCopyBufferToImage(cmd, pBuffer->api.vk.buffer, pImage->api.vk.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

  OPAL_ATTEMPT(EndSingleUseCommandBuffer_Ovk(cmd, g_ovkState->graphicsCommandPool, g_ovkState->queueGraphics));

  return Opal_Success;
}
