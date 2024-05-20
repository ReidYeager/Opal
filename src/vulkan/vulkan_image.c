
#include "vulkan/vulkan_common.h"
#include "opal.h" // Used for OpalVulkanImageFill

#include <math.h>
#include <stdint.h>

// Declarations
// ============================================================

// Core ==========
//OpalResult        OpalVulkanImageInit         (OpalImage* pImage, OpalImageInitInfo initInfo)
//void              OpalVulkanImageShutdown     (OpalImage* pImage)
OpalResult          InitImage_Ovk               (OpalImage* pImage, OpalImageInitInfo initInfo);
OpalResult          InitImageMemory_Ovk         (OpalImage* pImage);
OpalResult          InitView_Ovk                (OpalImage* pImage, OpalImageUsageFlags usage, uint32_t mipBase, uint32_t mipCount);
OpalResult          InitSampler_Ovk             (OpalImage* pImage, OpalImageFilterType filter, OpalImageSampleMode sample, uint32_t minLod, uint32_t   maxLod);
//OpalResult        OpalVulkanImagePushData     (OpalImage* pImage, const void* data)
OpalResult          FillMipmaps_Ovk             (OpalImage* pImage);

// Tools ==========
//OpalResult        ImageTransitionLayout_Ovk   (OpalImage* pImage, VkImageLayout newLayout)
//VkImageUsageFlags OpalImageUsageToVkFlags_Ovk (OpalImageUsageFlags opalFlags, OpalFormat format);
uint32_t            GetMemoryTypeIndex_Ovk      (uint32_t _mask, VkMemoryPropertyFlags _flags);
OpalResult          CopyBufferToImage_Ovk       (OpalBuffer* pBuffer, OpalImage* pImage);
//OpalResult        OpalVulkanImageGetMipAsImage(OpalImage* pImage, OpalImage* pMipImage, uint32_t mipLevel);

// Core
// ============================================================

OpalResult OpalVulkanImageInit(OpalImage* pImage, OpalImageInitInfo initInfo)
{
  OPAL_ATTEMPT(InitImage_Ovk(pImage, initInfo));
  OPAL_ATTEMPT(InitImageMemory_Ovk(pImage));
  OPAL_ATTEMPT(InitView_Ovk(pImage, initInfo.usage, 0, initInfo.mipCount));

  pImage->isMipReference = false;
  pImage->usage = initInfo.usage;
  pImage->filter = initInfo.filter;
  pImage->sampleMode = initInfo.sampleMode;
  pImage->api.vk.layout = VK_IMAGE_LAYOUT_UNDEFINED;
  pImage->api.vk.sampler = VK_NULL_HANDLE;

  if (initInfo.usage & Opal_Image_Usage_Storage)
  {
    OPAL_ATTEMPT(ImageTransitionLayout_Ovk(pImage, VK_IMAGE_LAYOUT_GENERAL));
  }

  if (initInfo.usage & Opal_Image_Usage_Uniform)
  {
    if (!(initInfo.usage & Opal_Image_Usage_Storage))
    {
      OPAL_ATTEMPT(ImageTransitionLayout_Ovk(pImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
    }

    OPAL_ATTEMPT(InitSampler_Ovk(pImage, initInfo.filter, initInfo.sampleMode, 0, initInfo.mipCount - 1));
  }
  else if (initInfo.usage & Opal_Image_Usage_Subpass_Product)
  {
    OPAL_ATTEMPT(ImageTransitionLayout_Ovk(pImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
  }

  return Opal_Success;
}

void OpalVulkanImageShutdown(OpalImage* pImage)
{
  if (!pImage->isMipReference)
  {
    vkFreeMemory(g_ovkState->device, pImage->api.vk.memory, NULL);
    vkDestroyImage(g_ovkState->device, pImage->api.vk.image, NULL);
  }
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
  createInfo.usage = OpalImageUsageToVkFlags_Ovk(initInfo.usage, initInfo.format);
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

OpalResult InitView_Ovk(OpalImage* pImage, OpalImageUsageFlags usage, uint32_t mipBase, uint32_t mipCount)
{
  VkImageViewCreateInfo vCreateInfo = { 0 };
  vCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  vCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  vCreateInfo.image = pImage->api.vk.image;
  vCreateInfo.format = pImage->api.vk.format;
  vCreateInfo.subresourceRange.aspectMask = 0;
  if (usage & Opal_Image_Usage_Output | Opal_Image_Usage_Uniform)
  {
    if (pImage->format == Opal_Format_D24_S8 || pImage->format == Opal_Format_D32)
    {
      vCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT
        | ((pImage->format == Opal_Format_D24_S8) * VK_IMAGE_ASPECT_STENCIL_BIT);
    }
    else
    {
      vCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
    }
  }
  vCreateInfo.subresourceRange.levelCount = mipCount;
  vCreateInfo.subresourceRange.baseMipLevel = mipBase;
  vCreateInfo.subresourceRange.layerCount = 1;
  vCreateInfo.subresourceRange.baseArrayLayer = 0;

  OPAL_ATTEMPT_VK(vkCreateImageView(g_ovkState->device, &vCreateInfo, NULL, &pImage->api.vk.view));

  pImage->api.vk.aspectFlags = vCreateInfo.subresourceRange.aspectMask;

  return Opal_Success;
}

OpalResult InitSampler_Ovk(OpalImage* pImage, OpalImageFilterType filter, OpalImageSampleMode sample, uint32_t minLod, uint32_t maxLod)
{
  VkSamplerCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  switch (filter)
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
  switch (sample)
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
  createInfo.minLod = (float)minLod;
  createInfo.maxLod = (float)maxLod;
  createInfo.maxAnisotropy = 1.0f;

  OPAL_ATTEMPT_VK(vkCreateSampler(g_ovkState->device, &createInfo, NULL, &pImage->api.vk.sampler));

  return Opal_Success;
}

OpalResult OpalVulkanImagePushData(OpalImage* pImage, const void* data)
{
  if (!(pImage->usage & Opal_Image_Usage_Transfer_Dst))
  {
    OpalLogError("Can not push data to image without Transfer_Dst usage bit");
    return Opal_Failure_Invalid_Input;
  }

  OpalBufferInitInfo bufferInfo = { 0 };
  bufferInfo.size = OpalFormatToSize(pImage->format) * pImage->width * pImage->height;
  bufferInfo.usage = Opal_Buffer_Usage_Cpu_Read | Opal_Buffer_Usage_Transfer_Src;
  OpalBuffer buffer;
  OPAL_ATTEMPT(OpalBufferInit(&buffer, bufferInfo));
  OPAL_ATTEMPT(OpalBufferPushData(&buffer, data));
  OpalWaitIdle();

  VkImageLayout ogLayout = pImage->api.vk.layout;
  OPAL_ATTEMPT(ImageTransitionLayout_Ovk(pImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL));
  OPAL_ATTEMPT(CopyBufferToImage_Ovk(&buffer, pImage));
  OPAL_ATTEMPT(ImageTransitionLayout_Ovk(pImage, ogLayout));

  OpalWaitIdle();
  OpalBufferShutdown(&buffer);

  if (pImage->mipCount > 1)
  {
    OPAL_ATTEMPT(FillMipmaps_Ovk(pImage));
  }

  return Opal_Success;
}

OpalResult OpalVulkanImageCopyImage(OpalImage* pImage, OpalImage* pSourceImage, OpalImageFilterType filter)
{
  if (!(pImage->usage & Opal_Image_Usage_Transfer_Dst))
  {
    OpalLogError("Destination image not setup with Transfer_Dst usage");
    return Opal_Failure_Invalid_Input;
  }
  if (!(pSourceImage->usage & Opal_Image_Usage_Transfer_Src))
  {
    OpalLogError("Source image not setup with Transfer_Src usage");
    return Opal_Failure_Invalid_Input;
  }
  if (pSourceImage->format != pImage->format)
  {
    OpalLogError("Source and destination images must have the same format for copy");
    return Opal_Failure_Invalid_Input;
  }

  VkImageAspectFlags aspect = 0;
  if (pSourceImage->format == Opal_Format_D24_S8 || pSourceImage->format == Opal_Format_D32)
  {
    aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
  }
  else
  {
    aspect = VK_IMAGE_ASPECT_COLOR_BIT;
  }

  VkFilter filterVk = VK_FILTER_LINEAR;
  switch (filter)
  {
  case Opal_Image_Filter_Linear:
  {
    filterVk = VK_FILTER_LINEAR;
  } break;
  case Opal_Image_Filter_Point:
  default:
  {
    filterVk = VK_FILTER_NEAREST;
  } break;
  }

  uint32_t mipCount = (pImage->mipCount < pSourceImage->mipCount) ? pImage->mipCount : pSourceImage->mipCount;
  VkImageLayout srcLayout = pSourceImage->api.vk.layout;
  VkImageLayout dstLayout = pImage->api.vk.layout;

  OPAL_ATTEMPT(ImageTransitionLayout_Ovk(pSourceImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL));
  OPAL_ATTEMPT(ImageTransitionLayout_Ovk(pImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL));

  VkCommandBuffer cmd;
  OPAL_ATTEMPT(SingleUseCmdBeginGraphics_Ovk(&cmd));

  int32_t srcMipWidth = (int32_t)pSourceImage->width;
  int32_t srcMipHeight = (int32_t)pSourceImage->height;
  int32_t dstMipWidth = (int32_t)pImage->width;
  int32_t dstMipHeight = (int32_t)pImage->height;
  for (int i = 0; i < mipCount; i++)
  {
    VkImageBlit blit = { 0 };
    blit.srcOffsets[0] = (VkOffset3D){ 0, 0, 0 };
    blit.srcOffsets[1] = (VkOffset3D){ srcMipWidth, srcMipHeight, 1 };
    blit.srcSubresource.aspectMask = aspect;
    blit.srcSubresource.mipLevel = i;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.srcSubresource.layerCount = 1;
    blit.dstOffsets[0] = (VkOffset3D){ 0, 0, 0 };
    blit.dstOffsets[1] = (VkOffset3D){ dstMipWidth, dstMipHeight, 1 };
    blit.dstSubresource.aspectMask = aspect;
    blit.dstSubresource.mipLevel = i;
    blit.dstSubresource.baseArrayLayer = 0;
    blit.dstSubresource.layerCount = 1;
    vkCmdBlitImage(
      cmd,
      pSourceImage->api.vk.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      pImage->api.vk.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1, &blit, filterVk);

    if (srcMipWidth > 1)
      srcMipWidth /= 2;
    if (srcMipHeight > 1)
      srcMipHeight /= 2;

    if (dstMipWidth > 1)
      dstMipWidth /= 2;
    if (dstMipHeight > 1)
      dstMipHeight /= 2;
  }

  OPAL_ATTEMPT(SingleUseCmdEndGraphics_Ovk(cmd));
  OpalWaitIdle();

  OPAL_ATTEMPT(ImageTransitionLayout_Ovk(pSourceImage, srcLayout));
  OPAL_ATTEMPT(ImageTransitionLayout_Ovk(pImage, dstLayout));

  return Opal_Success;
}

OpalResult FillMipmaps_Ovk(OpalImage* pImage)
{
  OPAL_ATTEMPT(ImageTransitionLayout_Ovk(pImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL));

  VkCommandBuffer cmd;
  OPAL_ATTEMPT(SingleUseCmdBeginGraphics_Ovk(&cmd));

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

  OPAL_ATTEMPT(SingleUseCmdEndGraphics_Ovk(cmd));
  OpalWaitIdle();

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
  memBarrier.subresourceRange.aspectMask = pImage->api.vk.aspectFlags;
  memBarrier.subresourceRange.levelCount = pImage->mipCount;
  memBarrier.subresourceRange.baseMipLevel = 0;
  memBarrier.subresourceRange.layerCount = 1;
  memBarrier.subresourceRange.baseArrayLayer = 0;
  memBarrier.srcAccessMask = 0;
  memBarrier.dstAccessMask = 0;

  memBarrier.oldLayout = pImage->api.vk.layout;
  memBarrier.newLayout = newLayout;

  VkPipelineStageFlagBits srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  VkPipelineStageFlagBits dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

  VkCommandBuffer cmd;
  OPAL_ATTEMPT(SingleUseCmdBeginGraphics_Ovk(&cmd));
  vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, NULL, 0, NULL, 1, &memBarrier);
  OPAL_ATTEMPT(SingleUseCmdEndGraphics_Ovk(cmd));
  OpalWaitIdle();

  pImage->api.vk.layout = newLayout;

  return Opal_Success;
}

VkImageUsageFlags OpalImageUsageToVkFlags_Ovk(OpalImageUsageFlags opalFlags, OpalFormat format)
{
  #define uses(o, vk) ((opalFlags & o) != 0) * vk
  VkImageUsageFlags vkUsage = 0;
  if (opalFlags & Opal_Image_Usage_Output)
  {
    if (format == Opal_Format_D32 || format == Opal_Format_D24_S8)
    {
      vkUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    else
    {
      vkUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
  }
  vkUsage |= uses(Opal_Image_Usage_Transfer_Src   , VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
  vkUsage |= uses(Opal_Image_Usage_Transfer_Dst   , VK_IMAGE_USAGE_TRANSFER_DST_BIT);
  vkUsage |= uses(Opal_Image_Usage_Uniform        , VK_IMAGE_USAGE_SAMPLED_BIT);
  vkUsage |= uses(Opal_Image_Usage_Subpass_Product, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
  vkUsage |= uses(Opal_Image_Usage_Storage        , VK_IMAGE_USAGE_STORAGE_BIT);
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
  OPAL_ATTEMPT(SingleUseCmdBeginGraphics_Ovk(&cmd));

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

  OPAL_ATTEMPT(SingleUseCmdEndGraphics_Ovk(cmd));
  OpalWaitIdle();

  return Opal_Success;
}

OpalResult OpalVulkanImageGetMipAsImage(OpalImage* pImage, OpalImage* pMipImage, uint32_t mipLevel)
{
  if (pImage->isMipReference)
  {
    OpalLogError("Can not create a mip reference of another mip reference");
    return Opal_Failure_Invalid_Input;
  }

  *pMipImage = *pImage;

  pMipImage->mipCount   = 1;
  pMipImage->height     = pImage->height * pow(0.5, mipLevel);
  pMipImage->width      = pImage->width  * pow(0.5, mipLevel);

  OPAL_ATTEMPT(InitView_Ovk(pMipImage, pMipImage->usage, mipLevel, 1));
  OPAL_ATTEMPT(InitSampler_Ovk(pMipImage, pMipImage->filter, pMipImage->sampleMode, mipLevel, mipLevel));

  pMipImage->isMipReference = true;

  return Opal_Success;
}
