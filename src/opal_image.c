
#include "src/common.h"

OpalResult OpalImageInit(OpalImage* _image, OpalImageInitInfo const* _initInfo)
{
  OpalImage_T* newImage = LapisMemAllocZeroSingle(OpalImage_T);

  OPAL_ATTEMPT(OvkImageInit(newImage, _initInfo));

  *_image = newImage;

  return Opal_Success;
}

void OpalImageShutdown(OpalImage* _image)
{
  OvkImageShutdown(*_image);
  LapisMemFree(*_image);
  *_image = NULL;
}

OpalResult OpalImageResize(OpalImage _image, OpalExtent _extents)
{
  OPAL_ATTEMPT(OvkImageResize(_image, _extents));

  return Opal_Success;
}
