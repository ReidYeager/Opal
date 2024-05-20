
#include "vulkan/vulkan_common.h"
#include "opal.h"

// Global variables
// ============================================================

static OpalBuffer g_transferBuffer_Ovk;

// Declarations
// ============================================================

// Core ==========
//OpalResult TransferBufferInit_Ovk         ()
//void       TransferBufferShutdown_Ovk     ()
//OpalResult OpalVulkanBufferInit           (OpalBuffer* pBuffer, OpalBufferInitInfo initInfo)
//void       OpalVulkanBufferShutdown       (OpalBuffer* pBuffer)
OpalResult   InitBuffer_Ovk                 (OpalVulkanBuffer* pBuffer, OpalBufferInitInfo initInfo);
OpalResult   InitBufferMemory_Ovk           (OpalVulkanBuffer* pBuffer, OpalBufferInitInfo initInfo);

// Manipulation ==========
//OpalResult OpalVulkanBufferPushData       (OpalBuffer* pBuffer, void* data)
//OpalResult OpalVulkanBufferPushDataSegment(OpalBuffer* pBuffer, void* data, uint64_t size, uint64_t offset)

// Tools ==========
uint64_t     PadBufferSize_Ovk              (uint32_t usage, uint64_t size);
uint32_t     GetMemTypeIndex_Ovk            (uint32_t supportedTypes, VkMemoryPropertyFlags flags);

// Core
// ============================================================

OpalResult TransferBufferInit_Ovk()
{
  OpalBufferInitInfo transferBufferInfo;
  transferBufferInfo.size = 1024;
  transferBufferInfo.usage = Opal_Buffer_Usage_Cpu_Read | Opal_Buffer_Usage_Transfer_Src;
  OPAL_ATTEMPT(OpalBufferInit(&g_transferBuffer_Ovk, transferBufferInfo));

  return Opal_Success;
}

void TransferBufferShutdown_Ovk()
{
  OpalBufferShutdown(&g_transferBuffer_Ovk);
}

OpalResult OpalVulkanBufferInit(OpalBuffer* pBuffer, OpalBufferInitInfo initInfo)
{
  if (initInfo.usage & Opal_Buffer_Usage_Uniform)
  {
    // TODO : Remove when pushing is optimized
    // Currently more efficient to do this than use a transfer intermediary
    initInfo.usage |= Opal_Buffer_Usage_Cpu_Read;
  }
  else if (initInfo.usage & (Opal_Buffer_Usage_Index | Opal_Buffer_Usage_Vertex))
  {
    initInfo.usage |= Opal_Buffer_Usage_Transfer_Dst;
  }

  pBuffer->size = initInfo.size;
  pBuffer->usage = initInfo.usage;
  pBuffer->paddedSize = PadBufferSize_Ovk(initInfo.usage, initInfo.size);
  initInfo.size = pBuffer->paddedSize;

  OPAL_ATTEMPT(InitBuffer_Ovk(&pBuffer->api.vk, initInfo));
  OPAL_ATTEMPT(InitBufferMemory_Ovk(&pBuffer->api.vk, initInfo));
  OPAL_ATTEMPT_VK(vkBindBufferMemory(g_ovkState->device, pBuffer->api.vk.buffer, pBuffer->api.vk.memory, 0));

  return Opal_Success;
}

void OpalVulkanBufferShutdown(OpalBuffer* pBuffer)
{
  vkDestroyBuffer(g_ovkState->device, pBuffer->api.vk.buffer, NULL);
  vkFreeMemory(g_ovkState->device, pBuffer->api.vk.memory, NULL);
}

OpalResult InitBuffer_Ovk(OpalVulkanBuffer* pBuffer, OpalBufferInitInfo initInfo)
{
  uint32_t queueIndices[2] = { g_ovkState->gpu.queueIndexGraphicsCompute, g_ovkState->gpu.queueIndexPresent };

  VkBufferCreateInfo createInfo = { 0 };
  createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;

  createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.queueFamilyIndexCount = 1;
  createInfo.pQueueFamilyIndices = queueIndices;
  createInfo.size = initInfo.size;

#define ovusage(opal, vk) ((initInfo.usage & opal) != 0) * vk
  createInfo.usage =
    ovusage(Opal_Buffer_Usage_Transfer_Dst, VK_BUFFER_USAGE_TRANSFER_DST_BIT)
    | ovusage(Opal_Buffer_Usage_Transfer_Src, VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
    | ovusage(Opal_Buffer_Usage_Uniform, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
    | ovusage(Opal_Buffer_Usage_Vertex, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
    | ovusage(Opal_Buffer_Usage_Index, VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
    | ovusage(Opal_Buffer_Usage_Storage, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
#undef ovusage

  OPAL_ATTEMPT_VK(vkCreateBuffer(g_ovkState->device, &createInfo, NULL, &pBuffer->buffer));

  return Opal_Success;
}

OpalResult InitBufferMemory_Ovk(OpalVulkanBuffer* pBuffer, OpalBufferInitInfo initInfo)
{
  VkMemoryRequirements memoryRequirements;
  vkGetBufferMemoryRequirements(g_ovkState->device, pBuffer->buffer, &memoryRequirements);

#define OToVk(opal, vk) ((vk) * ((initInfo.usage & (opal)) != 0))
  VkMemoryPropertyFlags memProperties =
    OToVk(Opal_Buffer_Usage_Cpu_Read, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
#undef OToVk

  VkMemoryAllocateInfo allocInfo = { 0 };
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.pNext = NULL;
  allocInfo.allocationSize = initInfo.size;
  allocInfo.memoryTypeIndex = GetMemTypeIndex_Ovk(memoryRequirements.memoryTypeBits, memProperties);

  if (allocInfo.memoryTypeIndex == ~0u)
  {
    return Opal_Failure_Unknown;
  }

  OPAL_ATTEMPT_VK(vkAllocateMemory(g_ovkState->device, &allocInfo, NULL, &pBuffer->memory));
  return Opal_Success;
}

// Manipulation
// ============================================================

OpalResult OpalVulkanBufferPushData(OpalBuffer* pBuffer, const void* data)
{
  return OpalVulkanBufferPushDataSegment(pBuffer, data, pBuffer->size, 0);
}

OpalResult OpalVulkanBufferPushDataSegment(OpalBuffer* pBuffer, const void* data, uint64_t size, uint64_t bufferOffset)
{
  if (pBuffer->usage & Opal_Buffer_Usage_Cpu_Read)
  {
    void* mappedMemory;
    OPAL_ATTEMPT_VK(vkMapMemory(g_ovkState->device, pBuffer->api.vk.memory, (VkDeviceSize)bufferOffset, (VkDeviceSize)size, 0, &mappedMemory));
    OpalMemCopy(data, mappedMemory, size);
    vkUnmapMemory(g_ovkState->device, pBuffer->api.vk.memory);

    return Opal_Success;
  }

  VkCommandBuffer cmd;
  OPAL_ATTEMPT(SingleUseCmdBeginTransfer_Ovk(&cmd));

  if (g_transferBuffer_Ovk.paddedSize < size)
  {
    OpalBufferShutdown(&g_transferBuffer_Ovk);

    OpalBufferInitInfo transferBufferInfo;
    transferBufferInfo.size = size;
    transferBufferInfo.usage = Opal_Buffer_Usage_Cpu_Read | Opal_Buffer_Usage_Transfer_Src;
    OPAL_ATTEMPT(OpalBufferInit(&g_transferBuffer_Ovk, transferBufferInfo));
  }

  void* mappedMemory;
  OPAL_ATTEMPT_VK(vkMapMemory(g_ovkState->device, g_transferBuffer_Ovk.api.vk.memory, (VkDeviceSize)bufferOffset, (VkDeviceSize)size, 0, &mappedMemory));
  OpalMemCopy(data, mappedMemory, size);
  vkUnmapMemory(g_ovkState->device, g_transferBuffer_Ovk.api.vk.memory);

  VkBufferCopy region = { 0 };
  region.srcOffset = 0;
  region.dstOffset = bufferOffset;
  region.size = size;

  vkCmdCopyBuffer(cmd, g_transferBuffer_Ovk.api.vk.buffer, pBuffer->api.vk.buffer, 1, &region);

  OPAL_ATTEMPT(SingleUseCmdEndTransfer_Ovk(cmd));

  return Opal_Success;
}

OpalResult OpalVulkanBufferDumpData(OpalBuffer* pBuffer, void* outData)
{
  return OpalVulkanBufferDumpDataSegment(pBuffer, outData, pBuffer->size, 0);
}

OpalResult OpalVulkanBufferDumpDataSegment(OpalBuffer* pBuffer, void* outData, uint64_t size, uint64_t offset)
{
  if ((pBuffer->usage & Opal_Buffer_Usage_Transfer_Src | Opal_Buffer_Usage_Cpu_Read) == 0)
  {
    OpalLogError("Can not dump buffer without Transfer_Src or Cpu_Read usage");
    return Opal_Failure_Invalid_Input;
  }

  if (pBuffer->size < size || pBuffer->size < offset + size)
  {
    OpalLogError("Trying to dump memory outside of the buffer's limits");
    return Opal_Failure_Invalid_Input;
  }

  if (pBuffer->usage & Opal_Buffer_Usage_Cpu_Read)
  {
    void* mappedMemory;
    OPAL_ATTEMPT_VK(vkMapMemory(g_ovkState->device, pBuffer->api.vk.memory, (VkDeviceSize)offset, (VkDeviceSize)size, 0, &mappedMemory));
    OpalMemCopy(mappedMemory, outData, size);
    vkUnmapMemory(g_ovkState->device, pBuffer->api.vk.memory);
    return Opal_Success;
  }

  VkCommandBuffer cmd;
  OPAL_ATTEMPT(SingleUseCmdBeginTransfer_Ovk(&cmd));

  VkBufferCopy region = { 0 };
  region.srcOffset = offset;
  region.dstOffset = 0;
  region.size = size;

  vkCmdCopyBuffer(cmd, pBuffer->api.vk.buffer, g_transferBuffer_Ovk.api.vk.buffer, 1, &region);

  OPAL_ATTEMPT(SingleUseCmdEndTransfer_Ovk(cmd));

  void* mappedMemory;
  OPAL_ATTEMPT_VK(vkMapMemory(g_ovkState->device, g_transferBuffer_Ovk.api.vk.memory, (VkDeviceSize)offset, (VkDeviceSize)size, 0, &mappedMemory));
  OpalMemCopy(mappedMemory, outData, size);
  vkUnmapMemory(g_ovkState->device, g_transferBuffer_Ovk.api.vk.memory);

  return Opal_Success;
}

// Tools
// ============================================================

uint64_t PadBufferSize_Ovk(uint32_t usage, uint64_t size)
{
  uint64_t alignment = 0;
  if (usage & Opal_Buffer_Usage_Uniform)
  {
    alignment = g_ovkState->gpu.properties.limits.minUniformBufferOffsetAlignment - 1;
  }
  else
  {
    alignment = g_ovkState->gpu.properties.limits.minStorageBufferOffsetAlignment - 1;
  }

  return (size + alignment) & ~alignment;
}

uint32_t GetMemTypeIndex_Ovk(uint32_t supportedTypes, VkMemoryPropertyFlags flags)
{
  const VkPhysicalDeviceMemoryProperties* props = &g_ovkState->gpu.memoryProperties;

  for (uint32_t i = 0; i < props->memoryTypeCount; i++)
  {
    if (supportedTypes & (1 << i) && (props->memoryTypes[i].propertyFlags & flags) == flags)
    {
      return i;
    }
  }

  OpalLog("Failed to find a suitable memory type for the buffer");
  return ~0u;
}
