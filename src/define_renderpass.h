
#ifndef GEM_OPAL_DEFINE_RENDERPASS_H_
#define GEM_OPAL_DEFINE_RENDERPASS_H_ 1

#include "include/opal.h"
#include <vulkan/vulkan.h>

typedef struct OvkRenderpass_T
{
  VkRenderPass renderpass;
} OvkRenderpass_T;

typedef struct OpalRenderpass_T
{
  OvkRenderpass_T vk;
} OpalRenderpass_T;

#endif // !GEM_OPAL_DEFINE_RENDERPASS_H_
