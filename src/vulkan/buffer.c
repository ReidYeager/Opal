
#include "src/defines.h"
#include "src/vulkan/vulkan.h"

#include <vulkan/vulkan.h>

uint64_t PadBufferSize(OvkState_T* _state, OpalBufferUsageFlags _usage, uint64_t _size)
{
  uint64_t alignment = 0;
  if (_usage & Opal_Buffer_Usage_Shader_Uniform)
  {
    alignment = _state->gpu.properties.limits.minUniformBufferOffsetAlignment - 1;
  }
  else
  {
    alignment = _state->gpu.properties.limits.minStorageBufferOffsetAlignment - 1;
  }

  return ( _size + alignment ) & ~alignment;
}

OpalResult CreateBuffer(OvkState_T* _state, OpalCreateBufferInfo _createInfo, OvkBuffer_T* _buffer)
{
  uint32_t queueFamilyIndices[2] = {
    _state->gpu.queueIndexGraphics,
    _state->gpu.queueIndexPresent
  };

  VkBufferCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.size = _createInfo.size;

  createInfo.pQueueFamilyIndices = queueFamilyIndices;
  if (_state->gpu.queueIndexGraphics == _state->gpu.queueIndexPresent)
  {
    createInfo.queueFamilyIndexCount = 1;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }
  else
  {
    createInfo.queueFamilyIndexCount = 2;
    createInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
  }

#define ObuToVk(opal, vk) ((vk) * ((_createInfo.usage & (opal)) != 0))
  createInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT // Assume everything will be set at some point
    | ObuToVk(Opal_Buffer_Usage_Cpu_Read, VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
    | ObuToVk(Opal_Buffer_Usage_Vertex, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
    | ObuToVk(Opal_Buffer_Usage_Index, VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
    | ObuToVk(Opal_Buffer_Usage_Shader_Uniform, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
#undef ObuToVk

  OVK_ATTEMPT(
    vkCreateBuffer(_state->device, &createInfo, NULL, &_buffer->buffer),
    return Opal_Failure_Vk_Create);

  return Opal_Success;
}

uint32_t GetMemTypeIndex(OvkState_T* _state, uint32_t _supportedTypes, VkMemoryPropertyFlags _flags)
{
  const VkPhysicalDeviceMemoryProperties* props = &_state->gpu.memoryProperties;

  for (uint32_t i = 0; i < props->memoryTypeCount; i++)
  {
    if (_supportedTypes & ( 1 << i ) && ( props->memoryTypes[i].propertyFlags & _flags ) == _flags)
    {
      return i;
    }
  }

  OPAL_LOG_VK_ERROR("Failed to find a suitable memory type for the buffer\n");
  return ~0u;
}

OpalResult AllocateMemory(OvkState_T* _state, OpalCreateBufferInfo _createInfo, OvkBuffer_T* _buffer)
{
  VkMemoryRequirements bufferMemRequirements;
  vkGetBufferMemoryRequirements(_state->device, _buffer->buffer, &bufferMemRequirements);

#define ObuToVk(opal, vk) ((vk) * ((_createInfo.usage & (opal)) != 0))
  VkMemoryPropertyFlags memProperties =
    ObuToVk(
      Opal_Buffer_Usage_Cpu_Read,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
#undef ObuToVk

  VkMemoryAllocateInfo allocInfo = { 0 };
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.pNext = NULL;
  allocInfo.allocationSize = _createInfo.size;
  allocInfo.memoryTypeIndex =
    GetMemTypeIndex(_state, bufferMemRequirements.memoryTypeBits, memProperties);

  if (allocInfo.memoryTypeIndex == ~0u)
  {
    return Opal_Failure_Vk_Create;
  }

  OVK_ATTEMPT(
    vkAllocateMemory(_state->device, &allocInfo, NULL, &_buffer->memory),
    return Opal_Failure_Vk_Create);

  return Opal_Success;
}

OpalResult OpalVkCreateBuffer(OpalState _oState, OpalCreateBufferInfo _createInfo, OpalBuffer _oBuffer)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;
  OvkBuffer_T* buffer = &_oBuffer->backend.vulkan;

  vkDeviceWaitIdle(state->device);

  _oBuffer->paddedSize = PadBufferSize(state, _createInfo.usage, _createInfo.size);
  _createInfo.size = _oBuffer->paddedSize;

  OPAL_ATTEMPT(CreateBuffer(state, _createInfo, buffer), return Opal_Failure_Vk_Create);
  OPAL_ATTEMPT(AllocateMemory(state, _createInfo, buffer), return Opal_Failure_Vk_Create);

  OVK_ATTEMPT(
    vkBindBufferMemory(state->device, buffer->buffer, buffer->memory, 0),
    return Opal_Failure_Vk_Create);

  return Opal_Success;
}

void OpalVkDestroyBuffer(OpalState _oState, OpalBuffer _oBuffer)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;
  OvkBuffer_T* buffer = &_oBuffer->backend.vulkan;

  vkFreeMemory(state->device, buffer->memory, NULL);
  vkDestroyBuffer(state->device, buffer->buffer, NULL);
}

OpalResult TransferBufferData(OvkState_T* _state, OpalBuffer _src, OpalBuffer _dst, uint64_t _size)
{
  VkCommandBuffer cmd;
  OvkBeginSingleUseCommand(_state, _state->transientCommantPool, &cmd);

  VkBufferCopy region = { 0 };
  region.srcOffset = 0;
  region.dstOffset = 0;
  region.size = _size;

  vkCmdCopyBuffer(
    cmd,
    _src->backend.vulkan.buffer,
    _dst->backend.vulkan.buffer,
    1,
    &region);

  OvkEndSingleUseCommand(_state, _state->transientCommantPool, _state->queueTransfer, cmd);

  return Opal_Success;
}

OpalResult OpalVkBufferPushData(OpalState _oState, OpalBuffer _oBuffer, void* _data)
{
  OvkState_T* state = (OvkState_T*)_oState->backend.state;
  OvkBuffer_T* buffer = &_oBuffer->backend.vulkan;

  // Transient buffer =====
  OpalBuffer transientBuffer;
  OpalCreateBufferInfo transientBufferInfo = { 0 };
  transientBufferInfo.size = _oBuffer->paddedSize;
  transientBufferInfo.usage = Opal_Buffer_Usage_Cpu_Read;
  OPAL_ATTEMPT(
    OpalCreateBuffer(_oState, transientBufferInfo, &transientBuffer),
    return Opal_Failure_Vk_Misc);

  // Copy data =====
  void* mappedMemory;
  OVK_ATTEMPT(
    vkMapMemory(
      state->device,
      transientBuffer->backend.vulkan.memory,
      0,
      _oBuffer->size,
      0,
      &mappedMemory),
    return Opal_Failure_Vk_Misc);
  LapisMemCopy(_data, mappedMemory, _oBuffer->size);
  vkUnmapMemory(state->device, transientBuffer->backend.vulkan.memory);

  // Transfer data =====
  TransferBufferData(state, transientBuffer, _oBuffer, _oBuffer->size);

  OpalDestroyBuffer(_oState, &transientBuffer);
  return Opal_Success;
}

