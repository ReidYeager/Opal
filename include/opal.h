
#ifndef GEM_OPAL_H
#define GEM_OPAL_H 1

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "include/opal_defines.h"
#include "include/opal_define_image.h"
#include "include/opal_define_buffer.h"
#include "include/opal_define_framebuffer.h"
#include "include/opal_define_image.h"
#include "include/opal_define_material.h"
#include "include/opal_define_mesh.h"
#include "include/opal_define_renderpass.h"
#include "include/opal_define_window.h"

typedef struct OpalInitInfo
{
  OpalWindowPlatformInfo_T windowPlatformInfo;
  bool debug;

  struct
  {
    uint32_t count;
    OpalFormat* pFormats;
  } vertexStruct;
} OpalInitInfo;

typedef struct OpalVkGpu_T
{
  VkPhysicalDevice device;
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceMemoryProperties memoryProperties;

  uint32_t queueFamilyPropertiesCount;
  VkQueueFamilyProperties* queueFamilyProperties;

  uint32_t queueIndexGraphics;
  uint32_t queueIndexTransfer;
  uint32_t queueIndexPresent;
} OpalVkGpu_T;

typedef struct OpalVkState_T
{
  const VkAllocationCallbacks* allocator;
  VkInstance instance;
  VkPhysicalDevice gpu;
  OpalVkGpu_T gpuInfo;
  VkDevice device;

  VkQueue queueGraphics;
  VkQueue queueTransfer;
  VkQueue queuePresent;

  VkCommandPool transientCommandPool;
  VkCommandPool graphicsCommandPool;

  VkDescriptorPool descriptorPool;
} OpalVkState_T;

typedef struct OpalVkVertexInfo_T
{
  VkVertexInputAttributeDescription* pAttribDescriptions;
  VkVertexInputBindingDescription bindingDescription;
} OpalVkVertexInfo_T;

typedef struct OpalState_T
{
  //OpalWindow_T window;
  OpalVkState_T vk;

  struct
  {
    uint32_t attribCount;
    uint32_t structSize;
    OpalFormat* pFormats;

    OpalVkVertexInfo_T vk;
  } vertexFormat;

} OpalState_T;
extern OpalState_T oState;

OpalResult OpalInit(OpalInitInfo _createInfo);
void OpalShutdown();

OpalResult OpalWindowInit(OpalWindow* _outWindow, OpalWindowInitInfo _initInfo);
void OpalWindowShutdown(OpalWindow* _window);
OpalResult OpalWindowReinit(OpalWindow _window);
void OpalWindowGetBufferImage(OpalWindow _window, OpalImage* _outImage);

OpalResult OpalImageInit(OpalImage* _image, OpalImageInitInfo _initInfo);
void OpalImageShutdown(OpalImage* _image);
OpalResult OpalImageResize(OpalImage _image, OpalExtent _extents);
OpalResult OpalImageFill(OpalImage _image, void* _data);
OpalFormat OpalImageGetFormat(OpalImage _image);
OpalExtent OpalImageGetExtents(OpalImage _image);

OpalResult OpalRenderpassInit(OpalRenderpass* _renderpass, OpalRenderpassInitInfo _initInfo);
void OpalRenderpassShutdown(OpalRenderpass* _renderpass);

OpalResult OpalFramebufferInit(OpalFramebuffer* _framebuffer, OpalFramebufferInitInfo _initInfo);
void OpalFramebufferShutdown(OpalFramebuffer* _framebuffer);
OpalResult OpalFramebufferReinit(OpalFramebuffer _framebuffer);

OpalResult OpalShaderInit(OpalShader* _shader, OpalShaderInitInfo _initInfo);
void OpalShaderShutdown(OpalShader* _shader);

OpalResult OpalInputLayoutInit(OpalInputLayout* _layout, OpalInputLayoutInitInfo _initInfo);
void OpalInputLayoutShutdown(OpalInputLayout* _layout);
OpalResult OpalInputSetInit(OpalInputSet* _set, OpalInputSetInitInfo _initInfo);
void OpalInputSetShutdown(OpalInputSet* _set);
OpalResult OpalInputSetUpdate(OpalInputSet _set, uint32_t _count, OpalInputInfo* _pInputs);

OpalResult OpalMaterialInit(OpalMaterial* _material, OpalMaterialInitInfo _initInfo);
void OpalMaterialShutdown(OpalMaterial* _material);
OpalResult OpalMaterialReinit(OpalMaterial _material);

OpalResult OpalRenderBegin(OpalWindow _window);
OpalResult OpalRenderEnd();
VkCommandBuffer OpalRenderGetCommandBuffer();
void OpalRenderBeginRenderpass(OpalRenderpass _renderpass, OpalFramebuffer _framebuffer);
void OpalRenderEndRenderpass(OpalRenderpass _renderpass);
void OpalRenderBindInputSet(OpalInputSet _set, uint32_t _setIndex);
void OpalRenderBindMaterial(OpalMaterial _material);
void OpalRenderMesh(OpalMesh _mesh);
void OpalRenderSetPushConstant(void* _data);

OpalResult OpalBufferInit(OpalBuffer* _buffer, OpalBufferInitInfo _initInfo);
void OpalBufferShutdown(OpalBuffer* _buffer);
OpalResult OpalBufferPushData(OpalBuffer _buffer, void* _data);
OpalResult OpalBufferPushDataSegment(OpalBuffer _buffer, void* _data, uint32_t size, uint32_t offset);


OpalResult OpalMeshInit(OpalMesh* _mesh, OpalMeshInitInfo _initInfo);
void OpalMeshShutdown(OpalMesh* _mesh);

OpalResult OpalBeginSingleUseCommand(VkCommandPool _pool, VkCommandBuffer* _cmd);
OpalResult OpalEndSingleUseCommand(VkCommandPool _pool, VkQueue _queue, VkCommandBuffer _cmd);

//void OpalRenderBindShaderArguments(); // For descriptor set input

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !GEM_OPAL_H
