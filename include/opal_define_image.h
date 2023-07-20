
#ifndef GEM_OPAL_DEFINE_IMAGE_H
#define GEM_OPAL_DEFINE_IMAGE_H 1

#include <stdint.h>
#include <stdbool.h>

typedef struct OpalImage_T* OpalImage;

typedef enum OpalImageFormat
{
  Opal_Image_Format_R8G8B8A8,
  Opal_Image_Format_R8G8B8,
  Opal_Image_Format_R32G32B32A32,
  Opal_Image_Format_R32G32B32,
  Opal_Image_Format_Depth
} OpalImageFormat;

typedef enum OpalImageUsage
{
  Opal_Image_Usage_Color_Buffer,
  Opal_Image_Usage_Depth,
  Opal_Image_Usage_Presented
} OpalImageUsage;

#endif // !GEM_OPAL_DEFINE_IMAGE_H
