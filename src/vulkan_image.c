
#include "src/common.h"

VkImageUsageFlags OpalUsageToVkUsage_Ovk(OpalImageUsage _usage)
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
  createInfo.mipLevels = 1;
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

  OpalLog("Vulkan image failed to find a suitable memory type index\n");
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
  vCreateInfo.subresourceRange.levelCount = 1;
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
  switch (_initInfo.sampleType)
  {
  case Opal_Sample_Bilinear:
  {
    createInfo.magFilter = VK_FILTER_LINEAR;
    createInfo.minFilter = VK_FILTER_LINEAR;
  } break;
  case Opal_Sample_Point:
  default:
  {
    createInfo.magFilter = VK_FILTER_NEAREST;
    createInfo.minFilter = VK_FILTER_NEAREST;
  } break;
  }
  createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT; // VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT; // VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
  createInfo.unnormalizedCoordinates = VK_FALSE;
  createInfo.compareEnable = VK_FALSE;
  createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  createInfo.mipLodBias = 0.0f;
  createInfo.minLod = 0.0f;
  createInfo.maxLod = 1.0f;
  createInfo.maxAnisotropy = 1.0f;

  OVK_ATTEMPT(vkCreateSampler(oState.vk.device, &createInfo, oState.vk.allocator, &_image->vk.sampler));

  return Opal_Success;
}

OpalResult OvkImageInit(OpalImage_T* _image, OpalImageInitInfo _initInfo)
{
  _image->extents = _initInfo.extent;
  _image->format = _initInfo.format;
  _image->usage = _initInfo.usage;

  OPAL_ATTEMPT(CreateImage_Ovk(_image, _initInfo));
  OPAL_ATTEMPT(AllocAndBindMemory_Ovk(_image));
  OPAL_ATTEMPT(CreateView_Ovk(_image));

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

  OPAL_ATTEMPT(OvkTransitionImageLayout(_image->vk.image, VK_IMAGE_LAYOUT_UNDEFINED, newlayout));
  _image->vk.layout = newlayout;

  return Opal_Success;
}

void OvkImageShutdown(OpalImage_T* _image)
{
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

  OPAL_ATTEMPT(OvkImageInit(_image, newInitInfo));

  return Opal_Success;
}

OpalResult CopyBufferToImage_Ovk(OpalImage_T* _image, OpalBuffer _buffer)
{
  VkCommandBuffer cmd;
  OPAL_ATTEMPT(OvkBeginSingleUseCommand(oState.vk.graphicsCommandPool, &cmd));

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

  OPAL_ATTEMPT(OvkEndSingleUseCommand(oState.vk.graphicsCommandPool, oState.vk.queueGraphics, cmd));

  return Opal_Success;
}

OpalResult OvkImageFill(OpalImage_T* _image, void* _data)
{
  OpalBufferInitInfo bufferInfo = { 0 };
  bufferInfo.size = OpalFormatToSize_Ovk(_image->format) * _image->extents.width * _image->extents.height * _image->extents.depth;
  bufferInfo.usage = Opal_Buffer_Usage_Cpu_Read | Opal_Buffer_Usage_Transfer_Src;
  OpalBuffer buffer;
  OPAL_ATTEMPT(OpalBufferInit(&buffer, bufferInfo));
  OPAL_ATTEMPT(OpalBufferPushData(buffer, _data));

  VkImageLayout ogLayout = _image->vk.layout;
  OPAL_ATTEMPT(OvkTransitionImageLayout(_image->vk.image, _image->vk.layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL));
  OPAL_ATTEMPT(CopyBufferToImage_Ovk(_image, buffer));
  OPAL_ATTEMPT(OvkTransitionImageLayout(_image->vk.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, ogLayout));

  OpalBufferShutdown(&buffer);

  return Opal_Success;
}

// TODO : Change this to (OpalImage_T*, vkImageLayout) when swapchain buffer image is transitioned to OpalImage
OpalResult OvkTransitionImageLayout(VkImage _image, VkImageLayout _layout, VkImageLayout _newLayout)
{
  VkImageMemoryBarrier memBarrier = { 0 };
  memBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  memBarrier.image = _image;
  memBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  memBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  memBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  memBarrier.subresourceRange.levelCount = 1;
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
      OpalLogError("Unknown new image layout %d\n", _newLayout);
      return Opal_Failure;
    }
  }

  VkCommandBuffer cmd;
  OPAL_ATTEMPT(OvkBeginSingleUseCommand(oState.vk.graphicsCommandPool, &cmd));
  vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, NULL, 0, NULL, 1, &memBarrier);
  OPAL_ATTEMPT(OvkEndSingleUseCommand(oState.vk.graphicsCommandPool, oState.vk.queueGraphics, cmd));

  return Opal_Success;
}
