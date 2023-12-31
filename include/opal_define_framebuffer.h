
#ifndef GEM_OPAL_DEFINE_FRAMEBUFFER_H_
#define GEM_OPAL_DEFINE_FRAMEBUFFER_H_ 1

#include "include/opal_define_image.h"
#include "include/opal_define_renderpass.h"

#include <vulkan/vulkan.h>

#include <stdint.h>
#include <stdbool.h>

typedef struct OpalFramebuffer_T* OpalFramebuffer;

typedef struct OvkFramebuffer_T
{
  VkFramebuffer framebuffer;
} OvkFramebuffer_T;

typedef struct OpalFramebuffer_T
{
  OpalExtent extent;
  uint32_t imageCount;
  OpalImage_T** ppImages;

  OpalRenderpass ownerRenderpass;

  OvkFramebuffer_T vk;
} OpalFramebuffer_T;

typedef struct OpalFramebufferInitInfo
{
  OpalRenderpass renderpass;

  uint32_t imageCount;
  const OpalImage* pImages;
} OpalFramebufferInitInfo;

#endif // !GEM_OPAL_DEFINE_FRAMEBUFFER_H_
