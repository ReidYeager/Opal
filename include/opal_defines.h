
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
#endif // OPAL_PLATFORM_*

#define OPAL_NULL_HANDLE NULL

typedef enum OpalResult
{
  Opal_Success = 0,
  Opal_Failure
} OpalResult;

typedef enum OpalFormat
{
  Opal_Format_Undefined,

  // R G B A variants
  Opal_Format_R8,
  Opal_Format_RG8,
  Opal_Format_RGB8,
  Opal_Format_RGBA8,
  Opal_Format_R32,
  Opal_Format_RG32,
  Opal_Format_RGB32,
  Opal_Format_RGBA32,
  Opal_Format_R64,
  Opal_Format_RG64,
  Opal_Format_RGB64,
  Opal_Format_RGBA64,

  Opal_Format_R8I,
  Opal_Format_RG8I,
  Opal_Format_RGB8I,
  Opal_Format_RGBA8I,
  Opal_Format_R32I,
  Opal_Format_RG32I,
  Opal_Format_RGB32I,
  Opal_Format_RGBA32I,
  Opal_Format_R64I,
  Opal_Format_RG64I,
  Opal_Format_RGB64I,
  Opal_Format_RGBA64I,

  Opal_Format_R8U,
  Opal_Format_RG8U,
  Opal_Format_RGB8U,
  Opal_Format_RGBA8U,
  Opal_Format_R32U,
  Opal_Format_RG32U,
  Opal_Format_RGB32U,
  Opal_Format_RGBA32U,
  Opal_Format_R64U,
  Opal_Format_RG64U,
  Opal_Format_RGB64U,
  Opal_Format_RGBA64U,

  // SRGB
  Opal_Format_R8_Nonlinear,
  Opal_Format_RG8_Nonlinear,
  Opal_Format_RGB8_Nonlinear,
  Opal_Format_RGBA8_Nonlinear,
  Opal_Format_R32_Nonlinear,
  Opal_Format_RG32_Nonlinear,
  Opal_Format_RGB32_Nonlinear,
  Opal_Format_RGBA32_Nonlinear,
  Opal_Format_R64_Nonlinear,
  Opal_Format_RG64_Nonlinear,
  Opal_Format_RGB64_Nonlinear,
  Opal_Format_RGBA64_Nonlinear,

  // B G R A variants
  Opal_Format_BGR8,
  Opal_Format_BGRA8,
  Opal_Format_BGR32,
  Opal_Format_BGRA32,

  // Depth stencils
  Opal_Format_D24_S8,
  Opal_Format_D16_S8,
  Opal_Format_D32,

  // Others
  Opal_Format_Mat4x4,

  Opal_Format_Count
} OpalFormat;

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
