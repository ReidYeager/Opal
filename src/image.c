
#include "src/defines.h"

OpalResult OpalCreateImage(OpalState _state, OpalCreateImageInfo _createInfo, OpalImage* _outImage)
{
  OpalImage_T* newImage = (OpalImage_T*)LapisMemAllocZero(sizeof(OpalImage_T));

  OPAL_ATTEMPT(
    _state->backend.CreateImage(_state, _createInfo, newImage),
    {
      LapisMemFree(newImage);
      return Opal_Failure_Backend;
    });

  newImage->extents = _createInfo.extents;

  *_outImage = newImage;
  return Opal_Success;
}

void OpalDestroyImage(OpalState _state, OpalImage* _image)
{
  OpalImage_T* image = *_image;
  _state->backend.DestroyImage(_state, image);
  LapisMemFree(image);
  *_image = NULL;
}
