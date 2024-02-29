
#include "src/vulkan/vulkan_common.h"

// Declarations
// ============================================================

// Tools ==========
// VkFormat OpalFormatToVkFormat_Ovk(OpalFormat _format)
// OpalFormat VkFormatToOpalFormat_Ovk(VkFormat _format)
// VkShaderStageFlags OpalStagesToVkStages_Ovk(OpalStageFlags stages);

// Tools
// ============================================================

VkFormat OpalFormatToVkFormat_Ovk(OpalFormat _format)
{
  switch (_format)
  {
  case Opal_Format_R8:       return VK_FORMAT_R8_UNORM;
  case Opal_Format_RG8:      return VK_FORMAT_R8G8_UNORM;
  case Opal_Format_RGB8:     return VK_FORMAT_R8G8B8_UNORM;
  case Opal_Format_RGBA8:    return VK_FORMAT_R8G8B8A8_UNORM;
  case Opal_Format_R16:      return VK_FORMAT_R16_UNORM;
  case Opal_Format_RG16:     return VK_FORMAT_R16G16_UNORM;
  case Opal_Format_RGB16:    return VK_FORMAT_R16G16B16_UNORM;
  case Opal_Format_RGBA16:   return VK_FORMAT_R16G16B16A16_UNORM;
  case Opal_Format_R32:      return VK_FORMAT_R32_SFLOAT;
  case Opal_Format_RG32:     return VK_FORMAT_R32G32_SFLOAT;
  case Opal_Format_RGB32:    return VK_FORMAT_R32G32B32_SFLOAT;
  case Opal_Format_RGBA32:   return VK_FORMAT_R32G32B32A32_SFLOAT;
  case Opal_Format_R64:      return VK_FORMAT_R64_SFLOAT;
  case Opal_Format_RG64:     return VK_FORMAT_R64G64_SFLOAT;
  case Opal_Format_RGB64:    return VK_FORMAT_R64G64B64_SFLOAT;
  case Opal_Format_RGBA64:   return VK_FORMAT_R64G64B64A64_SFLOAT;

  case Opal_Format_R8_I:     return VK_FORMAT_R8_SINT;
  case Opal_Format_RG8_I:    return VK_FORMAT_R8G8_SINT;
  case Opal_Format_RGB8_I:   return VK_FORMAT_R8G8B8_SINT;
  case Opal_Format_RGBA8_I:  return VK_FORMAT_R8G8B8A8_SINT;
  case Opal_Format_R16_I:    return VK_FORMAT_R16_SINT;
  case Opal_Format_RG16_I:   return VK_FORMAT_R16G16_SINT;
  case Opal_Format_RGB16_I:  return VK_FORMAT_R16G16B16_SINT;
  case Opal_Format_RGBA16_I: return VK_FORMAT_R16G16B16A16_SINT;
  case Opal_Format_R32_I:    return VK_FORMAT_R32_SINT;
  case Opal_Format_RG32_I:   return VK_FORMAT_R32G32_SINT;
  case Opal_Format_RGB32_I:  return VK_FORMAT_R32G32B32_SINT;
  case Opal_Format_RGBA32_I: return VK_FORMAT_R32G32B32A32_SINT;
  case Opal_Format_R64_I:    return VK_FORMAT_R64_SINT;
  case Opal_Format_RG64_I:   return VK_FORMAT_R64G64_SINT;
  case Opal_Format_RGB64_I:  return VK_FORMAT_R64G64B64_SINT;
  case Opal_Format_RGBA64_I: return VK_FORMAT_R64G64B64A64_SINT;

  case Opal_Format_R8_U:     return VK_FORMAT_R8_UINT;
  case Opal_Format_RG8_U:    return VK_FORMAT_R8G8_UINT;
  case Opal_Format_RGB8_U:   return VK_FORMAT_R8G8B8_UINT;
  case Opal_Format_RGBA8_U:  return VK_FORMAT_R8G8B8A8_UINT;
  case Opal_Format_R16_U:    return VK_FORMAT_R16_UINT;
  case Opal_Format_RG16_U:   return VK_FORMAT_R16G16_UINT;
  case Opal_Format_RGB16_U:  return VK_FORMAT_R16G16B16_UINT;
  case Opal_Format_RGBA16_U: return VK_FORMAT_R16G16B16A16_UINT;
  case Opal_Format_R32_U:    return VK_FORMAT_R32_UINT;
  case Opal_Format_RG32_U:   return VK_FORMAT_R32G32_UINT;
  case Opal_Format_RGB32_U:  return VK_FORMAT_R32G32B32_UINT;
  case Opal_Format_RGBA32_U: return VK_FORMAT_R32G32B32A32_UINT;
  case Opal_Format_R64_U:    return VK_FORMAT_R64_UINT;
  case Opal_Format_RG64_U:   return VK_FORMAT_R64G64_UINT;
  case Opal_Format_RGB64_U:  return VK_FORMAT_R64G64B64_UINT;
  case Opal_Format_RGBA64_U: return VK_FORMAT_R64G64B64A64_UINT;

  case Opal_Format_Presented_BGRA8: return VK_FORMAT_B8G8R8A8_SRGB;

    // Depth stencils
  case Opal_Format_D24_S8:   return VK_FORMAT_D24_UNORM_S8_UINT;
  case Opal_Format_D32:      return VK_FORMAT_D32_SFLOAT;

  default: OpalLogError("Vulkan unkown or unsupported opal format %d", _format); return VK_FORMAT_UNDEFINED;
  }
}

OpalFormat VkFormatToOpalFormat_Ovk(VkFormat _format)
{
  switch (_format)
  {
  case VK_FORMAT_R8_UNORM:            return Opal_Format_R8;
  case VK_FORMAT_R8G8_UNORM:          return Opal_Format_RG8;
  case VK_FORMAT_R8G8B8_UNORM:        return Opal_Format_RGB8;
  case VK_FORMAT_R8G8B8A8_UNORM:      return Opal_Format_RGBA8;
  case VK_FORMAT_R16_UNORM:           return Opal_Format_R16;
  case VK_FORMAT_R16G16_UNORM:        return Opal_Format_RG16;
  case VK_FORMAT_R16G16B16_UNORM:     return Opal_Format_RGB16;
  case VK_FORMAT_R16G16B16A16_UNORM:  return Opal_Format_RGBA16;
  case VK_FORMAT_R32_SFLOAT:          return Opal_Format_R32;
  case VK_FORMAT_R32G32_SFLOAT:       return Opal_Format_RG32;
  case VK_FORMAT_R32G32B32_SFLOAT:    return Opal_Format_RGB32;
  case VK_FORMAT_R32G32B32A32_SFLOAT: return Opal_Format_RGBA32;
  case VK_FORMAT_R64_SFLOAT:          return Opal_Format_R64;
  case VK_FORMAT_R64G64_SFLOAT:       return Opal_Format_RG64;
  case VK_FORMAT_R64G64B64_SFLOAT:    return Opal_Format_RGB64;
  case VK_FORMAT_R64G64B64A64_SFLOAT: return Opal_Format_RGBA64;

  case VK_FORMAT_R8_SINT:           return Opal_Format_R8_I;
  case VK_FORMAT_R8G8_SINT:         return Opal_Format_RG8_I;
  case VK_FORMAT_R8G8B8_SINT:       return Opal_Format_RGB8_I;
  case VK_FORMAT_R8G8B8A8_SINT:     return Opal_Format_RGBA8_I;
  case VK_FORMAT_R16_SINT:          return Opal_Format_R16_I;
  case VK_FORMAT_R16G16_SINT:       return Opal_Format_RG16_I;
  case VK_FORMAT_R16G16B16_SINT:    return Opal_Format_RGB16_I;
  case VK_FORMAT_R16G16B16A16_SINT: return Opal_Format_RGBA16_I;
  case VK_FORMAT_R32_SINT:          return Opal_Format_R32_I;
  case VK_FORMAT_R32G32_SINT:       return Opal_Format_RG32_I;
  case VK_FORMAT_R32G32B32_SINT:    return Opal_Format_RGB32_I;
  case VK_FORMAT_R32G32B32A32_SINT: return Opal_Format_RGBA32_I;
  case VK_FORMAT_R64_SINT:          return Opal_Format_R64_I;
  case VK_FORMAT_R64G64_SINT:       return Opal_Format_RG64_I;
  case VK_FORMAT_R64G64B64_SINT:    return Opal_Format_RGB64_I;
  case VK_FORMAT_R64G64B64A64_SINT: return Opal_Format_RGBA64_I;

  case VK_FORMAT_R8_UINT:           return Opal_Format_R8_U;
  case VK_FORMAT_R8G8_UINT:         return Opal_Format_RG8_U;
  case VK_FORMAT_R8G8B8_UINT:       return Opal_Format_RGB8_U;
  case VK_FORMAT_R8G8B8A8_UINT:     return Opal_Format_RGBA8_U;
  case VK_FORMAT_R16_UINT:          return Opal_Format_R16_U;
  case VK_FORMAT_R16G16_UINT:       return Opal_Format_RG16_U;
  case VK_FORMAT_R16G16B16_UINT:    return Opal_Format_RGB16_U;
  case VK_FORMAT_R16G16B16A16_UINT: return Opal_Format_RGBA16_U;
  case VK_FORMAT_R32_UINT:          return Opal_Format_R32_U;
  case VK_FORMAT_R32G32_UINT:       return Opal_Format_RG32_U;
  case VK_FORMAT_R32G32B32_UINT:    return Opal_Format_RGB32_U;
  case VK_FORMAT_R32G32B32A32_UINT: return Opal_Format_RGBA32_U;
  case VK_FORMAT_R64_UINT:          return Opal_Format_R64_U;
  case VK_FORMAT_R64G64_UINT:       return Opal_Format_RG64_U;
  case VK_FORMAT_R64G64B64_UINT:    return Opal_Format_RGB64_U;
  case VK_FORMAT_R64G64B64A64_UINT: return Opal_Format_RGBA64_U;

  case VK_FORMAT_B8G8R8A8_SRGB: return Opal_Format_Presented_BGRA8;

    // Depth stencils
  case VK_FORMAT_D24_UNORM_S8_UINT: return Opal_Format_D24_S8;
  case VK_FORMAT_D32_SFLOAT:        return Opal_Format_D32;

  default: OpalLogError("Unsupported vulkan format in Opal : %d", _format); return Opal_Format_Unknown;
  }
}

VkShaderStageFlags OpalStagesToVkStages_Ovk(OpalStageFlags stages)
{
  VkShaderStageFlags flags = 0;

  flags |= VK_SHADER_STAGE_VERTEX_BIT * ((stages & Opal_Stage_Vertex) == Opal_Stage_Vertex);
  flags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT * ((stages & Opal_Stage_Tesselation) == Opal_Stage_Tesselation);
  flags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT * ((stages & Opal_Stage_Tesselation) == Opal_Stage_Tesselation);
  flags |= VK_SHADER_STAGE_GEOMETRY_BIT * ((stages & Opal_Stage_Geometry) == Opal_Stage_Geometry);
  flags |= VK_SHADER_STAGE_FRAGMENT_BIT * ((stages & Opal_Stage_Fragment) == Opal_Stage_Fragment);
  flags |= VK_SHADER_STAGE_COMPUTE_BIT * ((stages & Opal_Stage_Compute) == Opal_Stage_Compute);
  flags |= VK_SHADER_STAGE_ALL_GRAPHICS * ((stages & Opal_Stage_All_Graphics) == Opal_Stage_All_Graphics);
  flags |= VK_SHADER_STAGE_ALL * ((stages & Opal_Stage_All) == Opal_Stage_All);

  return flags;
}