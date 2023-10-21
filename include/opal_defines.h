
#ifndef GEM_OPAL_DEFINES_H
#define GEM_OPAL_DEFINES_H 1

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "include/opal_define_image.h"
#include "include/opal_define_buffer.h"
#include "include/opal_define_framebuffer.h"
#include "include/opal_define_image.h"
#include "include/opal_define_material.h"
#include "include/opal_define_mesh.h"
#include "include/opal_define_renderpass.h"
#include "include/opal_define_window.h"
#include <lapis.h>

typedef enum OpalResult
{
  Opal_Success = 0,
  Opal_Failure
} OpalResult;

#define OPAL_NULL_HANDLE NULL

typedef enum OpalBufferUsageBits
{
  Opal_Buffer_Usage_Transfer_Src = 0x01,
  Opal_Buffer_Usage_Transfer_Dst = 0x02,
  Opal_Buffer_Usage_Uniform = 0x04,
  Opal_Buffer_Usage_Vertex = 0x08,
  Opal_Buffer_Usage_Index = 0x10,
  Opal_Buffer_Usage_Cpu_Read = 0x20
} OpalBufferUsageBits;

typedef struct OpalBufferInitInfo
{
  uint64_t size;
  uint32_t usage;
} OpalBufferInitInfo;

typedef struct OpalFramebufferInitInfo
{
  OpalRenderpass renderpass;

  uint32_t imageCount;
  OpalImage* pImages;
} OpalFramebufferInitInfo;

typedef struct OpalMeshInitInfo
{
  uint32_t vertexCount;
  void* pVertices;
  uint32_t indexCount;
  uint32_t* pIndices;
} OpalMeshInitInfo;

typedef struct OpalWindowInitInfo
{
  LapisWindow lapisWindow;
} OpalWindowInitInfo;

typedef struct OpalInitInfo
{
  LapisWindow lapisWindow;
  bool debug;

  struct
  {
    uint32_t count;
    OpalFormat* pFormats;
  } vertexStruct;
} OpalInitInfo;

typedef struct OpalVkGpu_T
{
  VkPhysicalDevice device;
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceMemoryProperties memoryProperties;

  uint32_t queueFamilyPropertiesCount;
  VkQueueFamilyProperties* queueFamilyProperties;

  uint32_t queueIndexGraphics;
  uint32_t queueIndexTransfer;
  uint32_t queueIndexPresent;
} OpalVkGpu_T;

typedef struct OpalVkState_T
{
  const VkAllocationCallbacks* allocator;
  VkInstance instance;
  VkPhysicalDevice gpu;
  OpalVkGpu_T gpuInfo;
  VkDevice device;

  VkQueue queueGraphics;
  VkQueue queueTransfer;
  VkQueue queuePresent;

  VkCommandPool transientCommandPool;
  VkCommandPool graphicsCommandPool;

  VkDescriptorPool descriptorPool;
} OpalVkState_T;

typedef struct OpalVkVertexInfo_T
{
  VkVertexInputAttributeDescription* pAttribDescriptions;
  VkVertexInputBindingDescription bindingDescription;
} OpalVkVertexInfo_T;

typedef struct OpalState_T
{
  //OpalWindow_T window;
  OpalVkState_T vk;

  struct
  {
    uint32_t attribCount;
    uint32_t structSize;
    OpalFormat* pFormats;

    OpalVkVertexInfo_T vk;
  } vertexFormat;

} OpalState_T;
extern OpalState_T oState;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !GEM_OPAL_DEFINES_H
