
#include "src/common.h"

OpalResult OpalImageInit(OpalImage* _image, OpalImageInitInfo _initInfo)
{
  if (!_initInfo.extent.width || !_initInfo.extent.height || !_initInfo.extent.depth)
  {
    OpalLogError("Image extents must all be greater than zero");
    OpalLogError("    Current extents : (%u, %u, %u)", _initInfo.extent.width, _initInfo.extent.height, _initInfo.extent.depth);
    return Opal_Failure;
  }

  _initInfo.mipLevels = (_initInfo.mipLevels == 0) ? 1 : _initInfo.mipLevels;

  OpalImage_T* newImage = OpalMemAllocZeroSingle(OpalImage_T);

  newImage->extents = _initInfo.extent;
  newImage->format = _initInfo.format;
  newImage->usage = _initInfo.usage;
  newImage->mipLevels = _initInfo.mipLevels;

  if ((_initInfo.usage & Opal_Image_Usage_Uniform) != 0 && _initInfo.mipLevels > 1)
  {
    _initInfo.usage |= Opal_Image_Usage_Copy_Src;
  }
  else
  {
    _initInfo.mipLevels = 1;
  }
  newImage->mipLevels = _initInfo.mipLevels;

  OPAL_ATTEMPT(OvkImageInit(newImage, _initInfo));

  if (newImage->extents.depth == 0)
  {
    newImage->extents.depth = 1;
  }

  *_image = newImage;

  return Opal_Success;
}

void OpalImageShutdown(OpalImage* _image)
{
  OvkImageShutdown(*_image);
  OpalMemFree(*_image);
  *_image = OPAL_NULL_HANDLE;
}

OpalResult OpalImageResize(OpalImage _image, OpalExtent _extents)
{
  OPAL_ATTEMPT(OvkImageResize(_image, _extents));

  return Opal_Success;
}

OpalResult OpalImageFill(OpalImage _image, void* _data)
{
  OPAL_ATTEMPT(OvkImageFill(_image, _data));
  return Opal_Success;
}

OpalFormat OpalImageGetFormat(OpalImage _image)
{
  return _image->format;
}

OpalExtent OpalImageGetExtents(OpalImage _image)
{
  return _image->extents;
}

uint32_t OpalImageDumpData(OpalImage image, void** data)
{
  return OvkImageDumpData(image, data);
}
