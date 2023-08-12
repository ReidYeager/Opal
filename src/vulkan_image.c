
#include "src/common.h"

VkFormat OpalFormatToVkFormat_Ovk(OpalImageFormat _format)
{
  switch (_format)
  {
  case Opal_Image_Format_R8G8B8A8 : return VK_FORMAT_B8G8R8A8_SRGB;
  case Opal_Image_Format_R8G8B8 : return VK_FORMAT_B8G8R8_SRGB;
  case Opal_Image_Format_R32G32B32A32 : return VK_FORMAT_R32G32B32A32_SFLOAT;
  case Opal_Image_Format_R32G32B32 : return VK_FORMAT_R32G32B32_SFLOAT;
  case Opal_Image_Format_Depth : return VK_FORMAT_D24_UNORM_S8_UINT;
  default: OpalLog("Vulkan unkown opal format %d\n", _format); return VK_FORMAT_UNDEFINED;
  }
}

VkImageUsageFlags OpalUsageToVkUsage_Ovk(OpalImageUsage _usage)
{
  #define uses(o, vk) ((_usage & o) != 0) * vk
  VkImageUsageFlags vkUsage = 0;
  vkUsage |= uses(Opal_Image_Usage_Color, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
  vkUsage |= uses(Opal_Image_Usage_Depth, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
  vkUsage |= uses(Opal_Image_Usage_Copy_Src, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
  vkUsage |= uses(Opal_Image_Usage_Copy_Dst, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
  return vkUsage;
  #undef uses
}


OpalResult CreateImage_Ovk(OpalImage_T* _image, OpalImageInitInfo const* _initInfo)
{
  VkImageCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  createInfo.extent.width = _initInfo->extent.width;
  createInfo.extent.height = _initInfo->extent.height;
  createInfo.extent.depth = _initInfo->extent.depth == 0 ? 1 : _initInfo->extent.depth;
  createInfo.format = OpalFormatToVkFormat_Ovk(_initInfo->format);
  createInfo.usage = OpalUsageToVkUsage_Ovk(_initInfo->usage);
  createInfo.mipLevels = 1;
  createInfo.arrayLayers = 1;
  createInfo.imageType = _initInfo->extent.depth <= 1 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_3D;
  createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.samples = VK_SAMPLE_COUNT_1_BIT;

  OVK_ATTEMPT(vkCreateImage(oState.vk.device, &createInfo, oState.vk.allocator, &_image->vk.image));

  _image->vk.format = createInfo.format;
  _image->vk.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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
  vCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  vCreateInfo.subresourceRange.levelCount = 1;
  vCreateInfo.subresourceRange.baseMipLevel = 0;
  vCreateInfo.subresourceRange.layerCount = 1;
  vCreateInfo.subresourceRange.baseArrayLayer = 0;

  OVK_ATTEMPT(vkCreateImageView(oState.vk.device, &vCreateInfo, oState.vk.allocator, &_image->vk.view));

  return Opal_Success;
}

OpalResult OvkImageInit(OpalImage_T* _image, OpalImageInitInfo const* _initInfo)
OpalResult OvkImageInit(OpalImage_T* _image, OpalImageInitInfo _initInfo)
{
  _image->extents = _initInfo.extent;
  _image->format = _initInfo.format;
  _image->usage = _initInfo.usage;

  OPAL_ATTEMPT(CreateImage_Ovk(_image, _initInfo));
  OPAL_ATTEMPT(AllocAndBindMemory_Ovk(_image));
  OPAL_ATTEMPT(CreateView_Ovk(_image));

  return Opal_Success;
}

void OvkImageShutdown(OpalImage_T* _image)
{
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

OpalResult OvkTransitionImageLayout(VkImage _image, VkImageLayout _layout, bool _toWritable)
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
    memBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    memBarrier.srcAccessMask = 0;
    memBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else
  {
    memBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    memBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    memBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dstStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
  }

  vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, NULL, 0, NULL, 1, &memBarrier);
  OPAL_ATTEMPT(OvkEndSingleUseCommand(oState.vk.graphicsCommandPool, oState.vk.queueGraphics, cmd));

  return Opal_Success;
}
