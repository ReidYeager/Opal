
#include "src/common.h"

VkImageUsageFlags OpalUsageToVkUsage_Ovk(OpalImageUsageFlags _usage)
{
  #define uses(o, vk) ((_usage & o) != 0) * vk
  VkImageUsageFlags vkUsage = 0;
  vkUsage |= uses(Opal_Image_Usage_Color, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
  vkUsage |= uses(Opal_Image_Usage_Depth, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
  vkUsage |= uses(Opal_Image_Usage_Copy_Src, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
  vkUsage |= uses(Opal_Image_Usage_Copy_Dst, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
  vkUsage |= uses(Opal_Image_Usage_Uniform, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
  return vkUsage;
  #undef uses
}


OpalResult CreateImage_Ovk(OpalImage_T* _image, OpalImageInitInfo _initInfo)
{
  VkImageCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  createInfo.extent.width = _initInfo.extent.width;
  createInfo.extent.height = _initInfo.extent.height;
  createInfo.extent.depth = _initInfo.extent.depth == 0 ? 1 : _initInfo.extent.depth;
  createInfo.format = OpalFormatToVkFormat_Ovk(_initInfo.format);
  createInfo.usage = OpalUsageToVkUsage_Ovk(_initInfo.usage);
  createInfo.mipLevels = _initInfo.mipLevels;
  createInfo.arrayLayers = 1;
  createInfo.imageType = _initInfo.extent.depth <= 1 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_3D;
  createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.samples = VK_SAMPLE_COUNT_1_BIT;

  OVK_ATTEMPT(vkCreateImage(oState.vk.device, &createInfo, oState.vk.allocator, &_image->vk.image));

  _image->vk.format = createInfo.format;

  return Opal_Success;
}

uint32_t GetMemoryTypeIndex_Ovk(uint32_t _mask, VkMemoryPropertyFlags _flags)
{
  const VkPhysicalDeviceMemoryProperties* memProps = &oState.vk.gpuInfo.memoryProperties;
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

OpalResult AllocAndBindMemory_Ovk(OpalImage_T* _image)
{
  VkMemoryRequirements memReq = { 0 };
  vkGetImageMemoryRequirements(oState.vk.device, _image->vk.image, &memReq);

  VkMemoryAllocateInfo memAllocInfo = { 0 };
  memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memAllocInfo.allocationSize = memReq.size;
  memAllocInfo.memoryTypeIndex = GetMemoryTypeIndex_Ovk(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  OVK_ATTEMPT(vkAllocateMemory(oState.vk.device, &memAllocInfo, oState.vk.allocator, &_image->vk.memory));
  OVK_ATTEMPT(vkBindImageMemory(oState.vk.device, _image->vk.image, _image->vk.memory, 0));

  return Opal_Success;
}

OpalResult CreateView_Ovk(OpalImage_T* _image)
{
  VkImageViewCreateInfo vCreateInfo = { 0 };
  vCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  vCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  vCreateInfo.image = _image->vk.image;
  vCreateInfo.format = _image->vk.format;
  vCreateInfo.subresourceRange.aspectMask = 0;
  if (_image->usage & Opal_Image_Usage_Color)
    vCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
  if (_image->usage & Opal_Image_Usage_Depth)
    vCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
  if (_image->usage & Opal_Image_Usage_Uniform)
    vCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
  vCreateInfo.subresourceRange.levelCount = _image->mipLevels;
  vCreateInfo.subresourceRange.baseMipLevel = 0;
  vCreateInfo.subresourceRange.layerCount = 1;
  vCreateInfo.subresourceRange.baseArrayLayer = 0;

  OVK_ATTEMPT(vkCreateImageView(oState.vk.device, &vCreateInfo, oState.vk.allocator, &_image->vk.view));

  return Opal_Success;
}

OpalResult CreateSampler_Ovk(OpalImage_T* _image, OpalImageInitInfo _initInfo)
{
  VkSamplerCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  switch (_initInfo.filterType)
  {
  case Opal_Image_Filter_Bilinear:
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
  switch (_initInfo.sampleMode)
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
  createInfo.maxLod = (float)_initInfo.mipLevels - 1.0f;
  createInfo.maxAnisotropy = 1.0f;

  OVK_ATTEMPT(vkCreateSampler(oState.vk.device, &createInfo, oState.vk.allocator, &_image->vk.sampler));

  return Opal_Success;
}

OpalResult FillMipmaps_Ovk(OpalImage_T* image)
{
  OPAL_ATTEMPT(OvkTransitionImageLayout(image->vk.image, image->vk.layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, image->mipLevels));

  VkCommandBuffer cmd;
  OPAL_ATTEMPT(OpalBeginSingleUseCommand(oState.vk.graphicsCommandPool, &cmd));

  VkImageMemoryBarrier barrier = {0};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.image = image->vk.image;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.subresourceRange.levelCount = 1;

  uint32_t mipWidth = image->extents.width;
  uint32_t mipHeight = image->extents.height;
  for (uint32_t i = 1; i < image->mipLevels; i++)
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

    VkImageBlit blit = {0};
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
      image->vk.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      image->vk.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
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

  barrier.subresourceRange.baseMipLevel = image->mipLevels - 1;
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

  image->vk.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  OPAL_ATTEMPT(OpalEndSingleUseCommand(oState.vk.graphicsCommandPool, oState.vk.queueGraphics, cmd));
  return Opal_Success;
}

OpalResult OvkImageInit(OpalImage_T* _image, OpalImageInitInfo _initInfo)
{
  OPAL_ATTEMPT(CreateImage_Ovk(_image, _initInfo));
  OPAL_ATTEMPT(AllocAndBindMemory_Ovk(_image));
  OPAL_ATTEMPT(CreateView_Ovk(_image));

  _image->vk.layout = VK_IMAGE_LAYOUT_UNDEFINED;

  if (_image->usage & Opal_Image_Usage_Uniform)
  {
    OPAL_ATTEMPT(CreateSampler_Ovk(_image, _initInfo));
  }
  else
  {
    _image->vk.sampler = VK_NULL_HANDLE;
  }

  VkImageLayout newlayout;
  if (_initInfo.usage & Opal_Image_Usage_Color)
    newlayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  else if (_initInfo.usage & Opal_Image_Usage_Uniform)
    newlayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  else
    newlayout = VK_IMAGE_LAYOUT_UNDEFINED;

  OPAL_ATTEMPT(OvkTransitionImageLayout(_image->vk.image, _image->vk.layout, newlayout, _image->mipLevels));
  _image->vk.layout = newlayout;

  return Opal_Success;
}

void OvkImageShutdown(OpalImage_T* _image)
{
  vkDeviceWaitIdle(oState.vk.device);
  if (_image->usage & Opal_Image_Usage_Uniform)
  {
    vkDestroySampler(oState.vk.device, _image->vk.sampler, oState.vk.allocator);
  }

  vkDestroyImageView(oState.vk.device, _image->vk.view, oState.vk.allocator);
  vkDestroyImage(oState.vk.device, _image->vk.image, oState.vk.allocator);
  vkFreeMemory(oState.vk.device, _image->vk.memory, oState.vk.allocator);
}

OpalResult OvkImageResize(OpalImage_T* _image, OpalExtent _extents)
{
  OvkImageShutdown(_image);

  OpalImageInitInfo newInitInfo = { 0 };
  newInitInfo.extent = _extents;
  newInitInfo.format = _image->format;
  newInitInfo.usage = _image->usage;
  newInitInfo.mipLevels = _image->mipLevels;

  OPAL_ATTEMPT(OvkImageInit(_image, newInitInfo));

  _image->extents = newInitInfo.extent;

  return Opal_Success;
}

OpalResult CopyBufferToImage_Ovk(OpalImage_T* _image, OpalBuffer _buffer)
{
  VkCommandBuffer cmd;
  OPAL_ATTEMPT(OpalBeginSingleUseCommand(oState.vk.graphicsCommandPool, &cmd));

  VkBufferImageCopy copyRegion = { 0 };
  copyRegion.bufferOffset = 0;
  copyRegion.bufferRowLength = _image->extents.width;
  copyRegion.bufferImageHeight = _image->extents.height;
  copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  copyRegion.imageSubresource.mipLevel = 0;
  copyRegion.imageSubresource.layerCount = 1;
  copyRegion.imageSubresource.baseArrayLayer = 0;
  copyRegion.imageOffset = (VkOffset3D){ 0, 0, 0 };
  copyRegion.imageExtent = (VkExtent3D){ _image->extents.width, _image->extents.height, _image->extents.depth };

  vkCmdCopyBufferToImage(cmd, _buffer->vk.buffer, _image->vk.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

  OPAL_ATTEMPT(OpalEndSingleUseCommand(oState.vk.graphicsCommandPool, oState.vk.queueGraphics, cmd));

  return Opal_Success;
}

OpalResult CopyImageToBuffer_Ovk(OpalImage image, OpalBuffer buffer)
{
  VkCommandBuffer cmd;
  OPAL_ATTEMPT(OpalBeginSingleUseCommand(oState.vk.graphicsCommandPool, &cmd));

  VkBufferImageCopy copyRegion = { 0 };
  copyRegion.bufferOffset = 0;
  copyRegion.bufferRowLength = image->extents.width;
  copyRegion.bufferImageHeight = image->extents.height;
  copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  copyRegion.imageSubresource.mipLevel = 0;
  copyRegion.imageSubresource.layerCount = 1;
  copyRegion.imageSubresource.baseArrayLayer = 0;
  copyRegion.imageOffset = (VkOffset3D){ 0, 0, 0 };
  copyRegion.imageExtent = (VkExtent3D){ image->extents.width, image->extents.height, image->extents.depth };

  vkCmdCopyImageToBuffer(cmd, image->vk.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer->vk.buffer, 1, &copyRegion);

  OPAL_ATTEMPT(OpalEndSingleUseCommand(oState.vk.graphicsCommandPool, oState.vk.queueGraphics, cmd));

  return Opal_Success;
}

OpalResult OvkImageFill(OpalImage_T* _image, void* _data)
{
  OpalBufferInitInfo bufferInfo = { 0 };
  bufferInfo.size = OpalFormatToSize(_image->format) * _image->extents.width * _image->extents.height * _image->extents.depth;
  bufferInfo.usage = Opal_Buffer_Usage_Cpu_Read | Opal_Buffer_Usage_Transfer_Src;
  OpalBuffer buffer;
  OPAL_ATTEMPT(OpalBufferInit(&buffer, bufferInfo));
  OPAL_ATTEMPT(OpalBufferPushData(buffer, _data));

  VkImageLayout ogLayout = _image->vk.layout;
  OPAL_ATTEMPT(OvkTransitionImageLayout(_image->vk.image, _image->vk.layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, _image->mipLevels));
  OPAL_ATTEMPT(CopyBufferToImage_Ovk(_image, buffer));
  OPAL_ATTEMPT(OvkTransitionImageLayout(_image->vk.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, ogLayout, _image->mipLevels));

  OpalBufferShutdown(&buffer);

  if (_image->mipLevels > 1)
  {
    OPAL_ATTEMPT(FillMipmaps_Ovk(_image));
  }

  return Opal_Success;
}

uint32_t OvkImageDumpData(OpalImage image, void** data)
{
  OpalBufferInitInfo bufferInfo = { 0 };
  bufferInfo.size = image->extents.width * image->extents.height * image->extents.depth * OpalFormatToSize(image->format);
  bufferInfo.usage = Opal_Buffer_Usage_Cpu_Read | Opal_Buffer_Usage_Transfer_Dst;
  OpalBuffer buffer;
  OPAL_ATTEMPT(OpalBufferInit(&buffer, bufferInfo));

  VkImageLayout ogLayout = image->vk.layout;
  OPAL_ATTEMPT(OvkTransitionImageLayout(image->vk.image, image->vk.layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image->mipLevels));
  OPAL_ATTEMPT(CopyImageToBuffer_Ovk(image, buffer));
  OPAL_ATTEMPT(OvkTransitionImageLayout(image->vk.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, ogLayout, image->mipLevels));

  uint32_t outSize = 0;
  outSize = OpalBufferDumpData(buffer, data);

  OpalBufferShutdown(&buffer);

  return outSize;
}

// TODO : Change this to (OpalImage_T*, vkImageLayout) when swapchain buffer image is transitioned to OpalImage
OpalResult OvkTransitionImageLayout(VkImage _image, VkImageLayout _layout, VkImageLayout _newLayout, uint32_t mipCount)
{
  VkImageMemoryBarrier memBarrier = { 0 };
  memBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  memBarrier.image = _image;
  memBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  memBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  memBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  memBarrier.subresourceRange.levelCount = mipCount;
  memBarrier.subresourceRange.baseMipLevel = 0;
  memBarrier.subresourceRange.layerCount = 1;
  memBarrier.subresourceRange.baseArrayLayer = 0;

  memBarrier.oldLayout = _layout;
  memBarrier.newLayout = _newLayout;

  VkPipelineStageFlagBits srcStage, dstStage;

  switch (_newLayout)
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
      OpalLogError("Unknown new image layout %d", _newLayout);
      return Opal_Failure;
    }
  }

  VkCommandBuffer cmd;
  OPAL_ATTEMPT(OpalBeginSingleUseCommand(oState.vk.graphicsCommandPool, &cmd));
  vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, NULL, 0, NULL, 1, &memBarrier);
  OPAL_ATTEMPT(OpalEndSingleUseCommand(oState.vk.graphicsCommandPool, oState.vk.queueGraphics, cmd));

  return Opal_Success;
}
