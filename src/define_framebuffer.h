
#ifndef GEM_OPAL_DEFINE_FRAMEBUFFER_H_
#define GEM_OPAL_DEFINE_FRAMEBUFFER_H_ 1

#include "include/opal.h"
#include "src/define_image.h"

#include <vulkan/vulkan.h>

#include <stdint.h>
#include <stdbool.h>

typedef struct OvkFramebuffer_T
{
  VkFramebuffer framebuffer;
} OvkFramebuffer_T;

typedef struct OpalFramebuffer_T
{
  OpalExtent extent;
  OpalImage_T** ppImages;

  OpalRenderpass ownerRenderpass;

  OvkFramebuffer_T vk;
} OpalFramebuffer_T;

#endif // !GEM_OPAL_DEFINE_FRAMEBUFFER_H_
