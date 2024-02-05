
#ifndef GEM_OPAL_DEFINE_IMAGE_H
#define GEM_OPAL_DEFINE_IMAGE_H 1

#include "include/opal_defines.h"

#include <vulkan/vulkan.h>

#include <stdint.h>
#include <stdbool.h>

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

typedef enum OpalImageUsageBits
{
  Opal_Image_Usage_Color = 0x01,
  Opal_Image_Usage_Depth = 0x02,
  Opal_Image_Usage_Copy_Src = 0x04,
  Opal_Image_Usage_Copy_Dst = 0x08,
  Opal_Image_Usage_Uniform = 0x10,
  Opal_Image_Usage_COUNT = 0x05
} OpalImageUsageBits;
typedef OpalFlags OpalImageUsageFlags;

typedef enum OpalImageFilterType
{
  Opal_Image_Filter_Point,
  Opal_Image_Filter_Bilinear,
  Opal_Image_Filter_COUNT
} OpalImageFilterType;

typedef enum OpalImageSampleMode
{
  Opal_Image_Sample_Wrap,
  Opal_Image_Sample_Clamp,
  Opal_Image_Sample_Reflect,
  Opal_Image_Sample_COUNT,
} OpalImageSampleMode;

typedef struct OpalImageInitInfo
{
  OpalExtent extent;
  OpalFormat format;
  OpalImageUsageFlags usage;
  OpalImageFilterType filterType;
  OpalImageSampleMode sampleMode;
  uint32_t mipLevels;
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
  OpalImageUsageFlags usage;

  uint32_t mipLevels;

  OvkImage_T vk;
} OpalImage_T;

#endif // !GEM_OPAL_DEFINE_IMAGE_H
