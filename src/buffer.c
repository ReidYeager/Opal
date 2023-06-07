
#include "src/common.h"

OpalResult OpalCreateBuffer(
  OpalState _state,
  OpalCreateBufferInfo _createInfo,
  OpalBuffer* _outBuffer)
{
  OpalBuffer_T* newBuffer = (OpalBuffer_T*)LapisMemAllocZero(sizeof(OpalBuffer_T));

  newBuffer->size = _createInfo.size;

  OPAL_ATTEMPT(
    _state->backend.CreateBuffer(_state, _createInfo, newBuffer),
    {
      OPAL_LOG_ERROR(
        "Failed to create backend buffer :\n\tSize = %llu\n\tUsage = %u\n",
        _createInfo.size,
        _createInfo.usage);
      LapisMemFree(newBuffer);
      return Opal_Failure_Backend;
    });

  *_outBuffer = newBuffer;
  return Opal_Success;
}

void OpalDestroyBuffer(OpalState _state, OpalBuffer* _buffer)
{
  OpalBuffer_T* buffer = *_buffer;
  _state->backend.DestroyBuffer(_state, buffer);
  LapisMemFree(buffer);
  *_buffer = NULL;
}

OpalResult OpalBufferPushData(OpalState _state, OpalBuffer _buffer, void* _data)
{
  OPAL_ATTEMPT(
    _state->backend.BufferPushData(_state, _buffer, _data),
    {
      OPAL_LOG_ERROR(
        "Failed to push data to buffer :\n\tBuffer = %p\n\tData = %p\n\tSize = %llu\n",
        _buffer,
        _data,
        _buffer->size);
      return Opal_Failure_Backend;
    });

  return Opal_Success;
}

