
#ifndef GEM_OPAL_DEFINE_BUFFER_H_
#define GEM_OPAL_DEFINE_BUFFER_H_

#include <vulkan/vulkan.h>

typedef struct OpalBuffer_T* OpalBuffer;

typedef struct OvkBuffer_T
{
  VkBuffer buffer;
  VkDeviceMemory memory;
} OvkBuffer_T;

typedef struct OpalBuffer_T
{
  uint64_t size;
  uint64_t paddedSize;
  OvkBuffer_T vk;
} OpalBuffer_T;

typedef struct OpalBufferInitInfo
{
  uint64_t size;
  OpalBufferUsageFlags usage;
} OpalBufferInitInfo;

#endif // !GEM_OPAL_DEFINE_BUFFER_H_
