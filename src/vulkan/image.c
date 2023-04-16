
#include "src/defines.h"
#include "src/vulkan/vulkan.h"

VkImageUsageFlags OpalImageUsageToVkImageUsage(OpalImageUsageFlags _usage)
{
  VkImageUsageFlags finalUsage = 0;

  finalUsage |=
    ((_usage & Opal_Image_Usage_Shader_Sampled) != 0)
    * (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

  return finalUsage;
}

OpalResult CreateVkImage(OvkState_T* _state, OpalCreateImageInfo _createInfo, OvkImage_T* _outImage)
{
  VkImageCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.extent.width = _createInfo.width;
  createInfo.extent.height = _createInfo.height;
  createInfo.extent.depth = 1;
  createInfo.format = OpalFormatToVkFormat(_createInfo.pixelFormat);
  createInfo.usage = OpalImageUsageToVkImageUsage(_createInfo.usage);
  createInfo.mipLevels = 1;
  createInfo.arrayLayers = 1;
  createInfo.imageType = VK_IMAGE_TYPE_2D;
  createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.samples = VK_SAMPLE_COUNT_1_BIT;

  OVK_ATTEMPT(
    vkCreateImage(_state->device, &createInfo, NULL, &_outImage->image),
    return Opal_Failure_Vk_Create);

  _outImage->format = createInfo.format;

  return Opal_Success;
}

uint32_t GetMemoryTypeIndex(OvkState_T* _state, uint32_t _mask, VkMemoryPropertyFlags _flags)
{
  const VkPhysicalDeviceMemoryProperties* memProps = &_state->gpu.memoryProperties;
  for (uint32_t i = 0; i < memProps->memoryTypeCount; i++)
  {
    if (_mask & ( 1 << i ) && ( memProps->memoryTypes[i].propertyFlags & _flags ) == _flags)
    {
      return i;
    }
  }

  OPAL_LOG_VK_ERROR("Failed to find a suitable memory type index\n");
  return ~0u;
}

OpalResult CreateVkImageMemory(
  OvkState_T* _state,
  OpalCreateImageInfo _createInfo,
  OvkImage_T* _outImage)
{
  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(_state->device, _outImage->image, &memRequirements);

  VkMemoryAllocateInfo allocInfo = { 0 };
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.pNext = NULL;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = 
    GetMemoryTypeIndex(_state, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  OVK_ATTEMPT(
    vkAllocateMemory(_state->device, &allocInfo, NULL, &_outImage->memory),
    return Opal_Failure_Vk_Create);

  return Opal_Success;
}

OpalResult CreateVkImageView(OvkState_T* _state, OvkImage_T* _image, VkImageAspectFlags _aspectMask)
{
  VkImageViewCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.viewType = VK_IMAGE_TYPE_2D;
  createInfo.image = _image->image;
  createInfo.format = _image->format;
  createInfo.subresourceRange.aspectMask = _aspectMask;
  createInfo.subresourceRange.levelCount = 1;
  createInfo.subresourceRange.baseMipLevel = 0;
  createInfo.subresourceRange.layerCount = 1;
  createInfo.subresourceRange.baseArrayLayer = 0;

  OVK_ATTEMPT(
    vkCreateImageView(_state->device, &createInfo, NULL, &_image->view),
    return Opal_Failure_Vk_Create);

  return Opal_Success;
}

OpalResult CreateVkSampler(OvkState_T* _state, OvkImage_T* _image)
{
  VkSamplerCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.magFilter = VK_FILTER_LINEAR;
  createInfo.minFilter = VK_FILTER_LINEAR;
  createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
  createInfo.unnormalizedCoordinates = VK_FALSE;
  createInfo.compareEnable = VK_FALSE;
  createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  createInfo.mipLodBias = 0.0f;
  createInfo.minLod = 0.0f;
  createInfo.maxLod = 1.0f;
  createInfo.maxAnisotropy = 1.0f;

  OVK_ATTEMPT(
    vkCreateSampler(_state->device, &createInfo, NULL, &_image->sampler),
    return Opal_Failure_Vk_Create);

  return Opal_Success;
}

OpalResult TransitionImageLayout(OvkState_T* _state, OvkImage_T* _image, uint32_t _toWritable, VkPipelineStageFlagBits _shaderStage)
{
  VkCommandBuffer cmd;
  OPAL_ATTEMPT(
    OvkBeginSingleUseCommand(_state, _state->graphicsCommandPool, &cmd),
    return Opal_Failure);

  VkImageMemoryBarrier memBarrier = { 0 };
  memBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  memBarrier.pNext = NULL;
  memBarrier.oldLayout = _image->layout;
  memBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  memBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  memBarrier.image = _image->image;
  memBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  memBarrier.subresourceRange.levelCount = 1;
  memBarrier.subresourceRange.baseMipLevel = 0;
  memBarrier.subresourceRange.layerCount = 1;
  memBarrier.subresourceRange.baseArrayLayer = 0;

  VkPipelineStageFlagBits srcStage, dstStage;

  if (!_toWritable)
  {
    memBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    memBarrier.srcAccessMask = 0;
    memBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else
  {
    memBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    memBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    memBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dstStage = _shaderStage;
  }

  vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, NULL, 0, NULL, 1, &memBarrier);
  OPAL_ATTEMPT(
    OvkEndSingleUseCommand(_state, _state->graphicsCommandPool, _state->queueGraphics, cmd),
    return Opal_Failure);

  _image->layout = memBarrier.newLayout;
  return Opal_Success;
}

OpalResult CopyBufferToImage(OvkState_T* _state, OvkBuffer_T* _buffer, OpalCreateImageInfo _createInfo, OvkImage_T* _image)
{
  VkCommandBuffer cmd;
  OPAL_ATTEMPT(
    OvkBeginSingleUseCommand(_state, _state->graphicsCommandPool, &cmd),
    return Opal_Failure);

  VkBufferImageCopy copyRegion = { 0 };
  copyRegion.bufferOffset = 0;
  copyRegion.bufferRowLength = _createInfo.width;
  copyRegion.bufferImageHeight = _createInfo.height;
  copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  copyRegion.imageSubresource.mipLevel = 0;
  copyRegion.imageSubresource.layerCount = 1;
  copyRegion.imageSubresource.baseArrayLayer = 0;
  copyRegion.imageOffset = (VkOffset3D){ 0, 0, 0 };
  copyRegion.imageExtent = (VkExtent3D){ _createInfo.width, _createInfo.height, 1 };

  vkCmdCopyBufferToImage(
    cmd,
    _buffer->buffer,
    _image->image,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    1,
    &copyRegion);

  OPAL_ATTEMPT(
    OvkEndSingleUseCommand(_state, _state->graphicsCommandPool, _state->queueGraphics, cmd),
    return Opal_Failure);

  return Opal_Success;
}

OpalResult OvkCreateImage(OpalState _oState, OpalCreateImageInfo _createInfo, OpalImage _oImage)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;
  OvkImage_T* image = &_oImage->backend.vulkan;

  OPAL_ATTEMPT(CreateVkImage(state, _createInfo, image), return Opal_Failure_Vk_Create);
  OPAL_ATTEMPT(CreateVkImageMemory(state, _createInfo, image), return Opal_Failure_Vk_Create);

  OVK_ATTEMPT(
    vkBindImageMemory(state->device, image->image, image->memory, 0),
    return Opal_Failure_Vk_Create);

  OPAL_ATTEMPT(
    CreateVkImageView(state, image, VK_IMAGE_ASPECT_COLOR_BIT),
    return Opal_Failure_Vk_Create);

  OPAL_ATTEMPT(CreateVkSampler(state, image), return Opal_Failure_Vk_Create);
  image->layout = VK_IMAGE_LAYOUT_UNDEFINED;

  OpalCreateBufferInfo createBufferInfo = { 0 };
  createBufferInfo.usage = Opal_Buffer_Usage_Cpu_Read;
  createBufferInfo.size =
    _createInfo.width * _createInfo.height * OpalFormatToSize(_createInfo.pixelFormat);
  OpalBuffer imageDataCopyBuffer;
  OpalCreateBuffer(_oState, createBufferInfo, &imageDataCopyBuffer);
  OpalBufferPushData(_oState, imageDataCopyBuffer, _createInfo.pixelData);

  OPAL_ATTEMPT(
    TransitionImageLayout(state, image, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT),
    return Opal_Failure_Vk_Create);
  OPAL_ATTEMPT(
    CopyBufferToImage(state, &imageDataCopyBuffer->backend.vulkan, _createInfo, image),
    return Opal_Failure_Vk_Create);
  OPAL_ATTEMPT(
    TransitionImageLayout(state, image, 1, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT),
    return Opal_Failure_Vk_Create);

  OpalDestroyBuffer(_oState, &imageDataCopyBuffer);

  return Opal_Success;
}

void OvkDestroyImage(OpalState _oState, OpalImage _oImage)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;
  OvkImage_T* image = &_oImage->backend.vulkan;

  vkDestroySampler(state->device, image->sampler, NULL);
  vkDestroyImageView(state->device, image->view, NULL);
  vkDestroyImage(state->device, image->image, NULL);
  vkFreeMemory(state->device, image->memory, NULL);
}
