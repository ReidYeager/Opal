
#ifndef GEM_OPAL_DEFINE_RENDERPASS_H_
#define GEM_OPAL_DEFINE_RENDERPASS_H_ 1

#include "include/opal.h"
#include <vulkan/vulkan.h>

typedef struct OpalRenderpass_T* OpalRenderpass;

typedef enum OpalAttachmentUsage
{
  Opal_Attachment_Usage_Color,
  Opal_Attachment_Usage_Depth,
  Opal_Attachment_Usage_Presented,
} OpalAttachmentUsage;

typedef enum OpalAttachmentLoadOp
{
  Opal_Attachment_Op_Clear,
  Opal_Attachment_Op_Load,
  Opal_Attachment_Op_Dont_Care,
} OpalAttachmentLoadOp;

typedef struct OpalAttachmentInfo
{
  OpalClearValue clearValue;
  OpalFormat format;
  OpalAttachmentUsage usage;
  OpalAttachmentLoadOp loadOp;
  bool shouldStore;
} OpalAttachmentInfo;

#define OPAL_DEPTH_ATTACHMENT_NONE -1

typedef struct OpalSubpassInfo
{
  uint32_t depthAttachmentIndex;
  uint32_t colorAttachmentCount;
  uint32_t* pColorAttachmentIndices;
  uint32_t inputAttachmentCount;
  uint32_t* pInputColorAttachmentIndices;
} OpalSubpassInfo;

typedef struct OpalDependencyInfo
{
  uint32_t srcIndex;
  uint32_t dstIndex;
} OpalDependencyInfo;

typedef struct OpalRenderpassInitInfo
{
  uint32_t imageCount;
  const OpalAttachmentInfo* pAttachments;

  uint32_t subpassCount;
  const OpalSubpassInfo* pSubpasses;

  uint32_t dependencyCount;
  const OpalDependencyInfo* pDependencies;
} OpalRenderpassInitInfo;

typedef struct OvkRenderpass_T
{
  VkRenderPass renderpass;
  VkClearValue* pClearValues;
} OvkRenderpass_T;

typedef struct OpalRenderpass_T
{
  uint32_t imageCount;
  OpalAttachmentInfo* pAttachments;

  uint32_t subpassCount;
  OpalSubpassInfo* pSubpasses;

  OvkRenderpass_T vk;
} OpalRenderpass_T;

#endif // !GEM_OPAL_DEFINE_RENDERPASS_H_
