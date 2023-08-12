
#ifndef GEM_OPAL_DEFINE_IMAGE_H_
#define GEM_OPAL_DEFINE_IMAGE_H_ 1

#include "include/opal.h"
#include <vulkan/vulkan.h>

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

#endif // !GEM_OPAL_DEFINE_IMAGE_H_
