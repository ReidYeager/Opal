
#include "src/common.h"

uint64_t PadBufferSize_Ovk(uint32_t _usage, uint64_t _size)
{
  uint64_t alignment = 0;
  if (_usage & Opal_Buffer_Usage_Uniform)
  {
    alignment = oState.vk.gpuInfo.properties.limits.minUniformBufferOffsetAlignment - 1;
  }
  else
  {
    alignment = oState.vk.gpuInfo.properties.limits.minStorageBufferOffsetAlignment - 1;
  }

  return (_size + alignment) & ~alignment;
}

OpalResult CreateBuffer_Ovk(OpalBuffer _buffer, OpalBufferInitInfo _initInfo)
{
  uint32_t queueIndices[2] = {
    oState.vk.gpuInfo.queueIndexGraphics,
    oState.vk.gpuInfo.queueIndexPresent };

  VkBufferCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  createInfo.size = _initInfo.size;

  createInfo.pQueueFamilyIndices = queueIndices;
  if (oState.vk.gpuInfo.queueIndexGraphics == oState.vk.gpuInfo.queueIndexPresent)
  {
    createInfo.queueFamilyIndexCount = 1;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }
  else
  {
    createInfo.queueFamilyIndexCount = 2;
    createInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
  }

#define ovusage(opal, vk) ((_initInfo.usage & opal) != 0) * vk
  createInfo.usage =
      ovusage(Opal_Buffer_Usage_Transfer_Dst, VK_BUFFER_USAGE_TRANSFER_DST_BIT)
    | ovusage(Opal_Buffer_Usage_Transfer_Src, VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
    | ovusage(Opal_Buffer_Usage_Uniform, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
    | ovusage(Opal_Buffer_Usage_Vertex, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
    | ovusage(Opal_Buffer_Usage_Index, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
#undef ovusage

  OVK_ATTEMPT(vkCreateBuffer(oState.vk.device, &createInfo, oState.vk.allocator, &_buffer->vk.buffer));

  return Opal_Success;
}

uint32_t GetMemTypeIndex_Ovk(uint32_t _supportedTypes, VkMemoryPropertyFlags _flags)
{
  const VkPhysicalDeviceMemoryProperties* props = &oState.vk.gpuInfo.memoryProperties;

  for (uint32_t i = 0; i < props->memoryTypeCount; i++)
  {
    if (_supportedTypes & (1 << i) && (props->memoryTypes[i].propertyFlags & _flags) == _flags)
    {
      return i;
    }
  }

  OpalLog("Failed to find a suitable memory type for the buffer\n");
  return ~0u;
}

OpalResult CreateMemory_Ovk(OpalBuffer _buffer, OpalBufferInitInfo _initInfo)
{
  VkMemoryRequirements bufferMemRequirements;
  vkGetBufferMemoryRequirements(oState.vk.device, _buffer->vk.buffer, &bufferMemRequirements);

#define OToVk(opal, vk) ((vk) * ((_initInfo.usage & (opal)) != 0))
  VkMemoryPropertyFlags memProperties =
    OToVk(Opal_Buffer_Usage_Cpu_Read, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
#undef OToVk

  VkMemoryAllocateInfo allocInfo = { 0 };
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.pNext = NULL;
  allocInfo.allocationSize = _initInfo.size;
  allocInfo.memoryTypeIndex = GetMemTypeIndex_Ovk(bufferMemRequirements.memoryTypeBits, memProperties);

  if (allocInfo.memoryTypeIndex == ~0u)
  {
    return Opal_Failure;
  }

  OVK_ATTEMPT(vkAllocateMemory(oState.vk.device, &allocInfo, oState.vk.allocator, &_buffer->vk.memory));
  return Opal_Success;
}

OpalResult OvkBufferInit(OpalBuffer _buffer, OpalBufferInitInfo _initInfo)
{
  // TODO : Remove this eventually. Currently faster to just keep buffers cpu visible.
  _initInfo.usage |= Opal_Buffer_Usage_Cpu_Read;

  vkDeviceWaitIdle(oState.vk.device);

  _buffer->size = _initInfo.size;
  _buffer->paddedSize = PadBufferSize_Ovk(_initInfo.usage, _initInfo.size);
  _initInfo.size = _buffer->paddedSize;

  OPAL_ATTEMPT(CreateBuffer_Ovk(_buffer, _initInfo));
  OPAL_ATTEMPT(CreateMemory_Ovk(_buffer, _initInfo));
  OVK_ATTEMPT(vkBindBufferMemory(oState.vk.device, _buffer->vk.buffer, _buffer->vk.memory, 0));

  return Opal_Success;
}

void OvkBufferShutdown(OpalBuffer _buffer)
{
  vkFreeMemory(oState.vk.device, _buffer->vk.memory, oState.vk.allocator);
  vkDestroyBuffer(oState.vk.device, _buffer->vk.buffer, oState.vk.allocator);
}

OpalResult TransferBufferData_Ovk(OpalBuffer _src, OpalBuffer _dst, uint64_t _size)
{
  VkCommandBuffer cmd;
  OpalBeginSingleUseCommand(oState.vk.transientCommandPool, &cmd);

  VkBufferCopy region = { 0 };
  region.srcOffset = 0;
  region.dstOffset = 0;
  region.size = _size;

  vkCmdCopyBuffer(cmd, _src->vk.buffer, _dst->vk.buffer, 1, &region);

  OpalEndSingleUseCommand(oState.vk.transientCommandPool, oState.vk.queueTransfer, cmd);

  return Opal_Success;
}

// NOTE : Will eventually want to separate quick cpu-visible buffer updates
OpalResult OvkBufferPushDataSegment(OpalBuffer _buffer, void* _data, uint32_t size, uint32_t offset)
{
  //static uint64_t maxSize = 0;
  //static OpalBuffer transientBuffer;

  //if (_buffer->paddedSize > maxSize)
  //{
  //  OvkBufferShutdown(transientBuffer);

  //  // Transient buffer =====
  //  OpalBufferInitInfo transientBufferInfo = { 0 };
  //  transientBufferInfo.size = _buffer->paddedSize;
  //  transientBufferInfo.usage = Opal_Buffer_Usage_Cpu_Read;
  //  OPAL_ATTEMPT(OvkBufferInit(transientBuffer, transientBufferInfo));

  //  maxSize = _buffer->paddedSize;
  //}

  void* mappedMemory;
  OVK_ATTEMPT(vkMapMemory(oState.vk.device, _buffer->vk.memory, (VkDeviceSize)offset, (VkDeviceSize)size, 0, &mappedMemory));
  LapisMemCopy(_data, mappedMemory, _buffer->size);
  vkUnmapMemory(oState.vk.device, _buffer->vk.memory);

  //TransferBufferData_Ovk(transientBuffer, _buffer, _buffer->size);

  return Opal_Success;
}

OpalResult OvkBufferPushData(OpalBuffer _buffer, void* _data)
{
  return OvkBufferPushDataSegment(_buffer, _data, _buffer->size, 0);
}

uint32_t OvkBufferDumpData(OpalBuffer buffer, void** data)
{
  void* mappedMemory;
  OVK_ATTEMPT(vkMapMemory(oState.vk.device, buffer->vk.memory, 0, (VkDeviceSize)buffer->size, 0, &mappedMemory));
  *data = LapisMemAlloc(buffer->size);
  LapisMemCopy(mappedMemory, *data, buffer->size);
  vkUnmapMemory(oState.vk.device, buffer->vk.memory);
  return buffer->size;
}
