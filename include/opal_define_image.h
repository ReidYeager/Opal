
#ifndef GEM_OPAL_DEFINE_IMAGE_H
#define GEM_OPAL_DEFINE_IMAGE_H 1

#include <stdint.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

typedef struct OpalImage_T* OpalImage;

typedef struct OpalExtent
{
  uint32_t width;
  uint32_t height;
  uint32_t depth;
} OpalExtent;

typedef union OpalColorValue
{
  float float32[4];
  int32_t int32[4];
  uint32_t uint32[4];
} OpalColorValue;

typedef struct OpalDepthStencilValue
{
  float depth;
  uint32_t stencil;
} OpalDepthStencilValue;

typedef union OpalClearValue
{
  OpalColorValue color;
  OpalDepthStencilValue depthStencil;
} OpalClearValue;

typedef enum OpalFormat
{
  Opal_Format_RGBA8, // r-g-b
  Opal_Format_RGB8,
  Opal_Format_RG8,
  Opal_Format_R8,
  Opal_Format_RGBA32,
  Opal_Format_RGB32,
  Opal_Format_RG32,
  Opal_Format_R32,

  Opal_Format_BGRA8, // b-g-r
  Opal_Format_BGR8,
  Opal_Format_BGRA32,
  Opal_Format_BGR32,

  Opal_Format_Depth
} OpalFormat;

typedef enum OpalImageUsageBits
{
  Opal_Image_Usage_Color = 0x01,
  Opal_Image_Usage_Depth = 0x02,
  Opal_Image_Usage_Copy_Src = 0x04,
  Opal_Image_Usage_Copy_Dst = 0x08,
  Opal_Image_Usage_Uniform = 0x10,
} OpalImageUsage;

typedef enum OpalImageSampleType
{
  Opal_Sample_Point,
  Opal_Sample_Bilinear
} OpalImageSampleType;

typedef struct OpalImageInitInfo
{
  OpalExtent extent;
  OpalFormat format;
  OpalImageUsage usage;
  OpalImageSampleType sampleType;
} OpalImageInitInfo;

typedef struct OvkImage_T
{
  VkImage image;
  VkImageView view;
  VkSampler sampler;
  VkDeviceMemory memory;

  VkFormat format;
  VkImageLayout layout;
} OvkImage_T;

typedef struct OpalImage_T
{
  OpalExtent extents;
  OpalFormat format;
  OpalImageUsage usage;

  OvkImage_T vk;
} OpalImage_T;

#endif // !GEM_OPAL_DEFINE_IMAGE_H
