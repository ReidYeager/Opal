
#ifndef GEM_OPAL_DEFINE_BUFFER_H_
#define GEM_OPAL_DEFINE_BUFFER_H_

#include <vulkan/vulkan.h>

typedef enum OpalBufferElement
{
  Opal_Buffer_Int,    Opal_Buffer_Int2,    Opal_Buffer_Int3,    Opal_Buffer_Int4,
  Opal_Buffer_Uint,   Opal_Buffer_Uint2,   Opal_Buffer_Uint3,   Opal_Buffer_Uint4,
  Opal_Buffer_Float,  Opal_Buffer_Float2,  Opal_Buffer_Float3,  Opal_Buffer_Float4,
  Opal_Buffer_Double, Opal_Buffer_Double2, Opal_Buffer_Double3, Opal_Buffer_Double4,
  Opal_Buffer_Mat4,

  Opal_Buffer_Structure_End
} OpalBufferElement;

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

typedef struct OpalBufferInitAlignedInfo
{
  uint32_t elementCount;
  const OpalBufferElement* pElements;

  OpalBufferUsageFlags usage;

} OpalBufferInitAlignedInfo;

#endif // !GEM_OPAL_DEFINE_BUFFER_H_
