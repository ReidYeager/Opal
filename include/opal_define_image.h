
#ifndef GEM_OPAL_DEFINE_IMAGE_H
#define GEM_OPAL_DEFINE_IMAGE_H 1

#include <stdint.h>
#include <stdbool.h>

typedef struct OpalImage_T* OpalImage;

typedef struct OpalExtent
{
  uint32_t width;
  uint32_t height;
  uint32_t depth;
} OpalExtent;

typedef enum OpalImageFormat
{
  Opal_Image_Format_R8G8B8A8,
  Opal_Image_Format_R8G8B8,
  Opal_Image_Format_R32G32B32A32,
  Opal_Image_Format_R32G32B32,
  Opal_Image_Format_Depth
} OpalImageFormat;

typedef enum OpalImageUsageBits
{
  Opal_Image_Usage_Color = 0x01,
  Opal_Image_Usage_Depth = 0x02,
  Opal_Image_Usage_Copy_Src = 0x04,
  Opal_Image_Usage_Copy_Dst = 0x08,

  Opal_Image_Usage_Presented = Opal_Image_Usage_Color | Opal_Image_Usage_Copy_Src,
} OpalImageUsage;

typedef struct OpalImageInitInfo
{
  OpalExtent extent;
  OpalImageFormat format;
  OpalImageUsage usage;
} OpalImageInitInfo;

#endif // !GEM_OPAL_DEFINE_IMAGE_H
