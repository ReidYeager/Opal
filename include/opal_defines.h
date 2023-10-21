
#ifndef GEM_OPAL_DEFINES_H
#define GEM_OPAL_DEFINES_H 1

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define OPAL_PLATFORM_WIN32 1
#ifndef _WIN64
#error "Must have 64-bit windows"
#endif
#else
#error "Unsupported platform"
#endif

typedef enum OpalResult
{
  Opal_Success = 0,
  Opal_Failure
} OpalResult;

#define OPAL_NULL_HANDLE NULL

typedef enum OpalBufferUsageBits
{
  Opal_Buffer_Usage_Transfer_Src = 0x01,
  Opal_Buffer_Usage_Transfer_Dst = 0x02,
  Opal_Buffer_Usage_Uniform = 0x04,
  Opal_Buffer_Usage_Vertex = 0x08,
  Opal_Buffer_Usage_Index = 0x10,
  Opal_Buffer_Usage_Cpu_Read = 0x20
} OpalBufferUsageBits;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !GEM_OPAL_DEFINES_H
