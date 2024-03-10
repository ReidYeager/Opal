#ifndef OPAL_H
#define OPAL_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "./defines.h"

OpalResult OpalInit    (OpalInitInfo initInfo);
void       OpalShutdown();
void       OpalWaitIdle();
OpalState* OpalGetState();

// ==============================
// Objects
// ==============================

// Window ==========
OpalResult OpalWindowInit                 (OpalWindow* pWindow, OpalWindowInitInfo initInfo);
void       OpalWindowShutdown             (OpalWindow* pWindow);

// Buffer ==========
OpalResult OpalBufferInit                 (OpalBuffer* pBuffer, OpalBufferInitInfo initInfo);
void       OpalBufferShutdown             (OpalBuffer* pBuffer);
OpalResult OpalBufferPushData             (OpalBuffer* pBuffer, const void* data);
OpalResult OpalBufferPushDataSegment      (OpalBuffer* pBuffer, const void* data, uint64_t size, uint64_t offset);

// Image ==========
OpalResult OpalImageInit                  (OpalImage* pImage, OpalImageInitInfo initInfo);
void       OpalImageShutdown              (OpalImage* pImage);
OpalResult OpalImagePushData              (OpalImage* pImage, const void* data);
OpalResult OpalImageGetMipAsImage         (OpalImage* pImage, OpalImage* pMipImage, uint32_t mipLevel);

// Mesh ==========
OpalResult OpalMeshInit                   (OpalMesh* pMesh, OpalMeshInitInfo initInfo);
void       OpalMeshShutdown               (OpalMesh* pMesh);

// Renderpass ==========
OpalResult OpalRenderpassInit             (OpalRenderpass* pRenderpass, OpalRenderpassInitInfo initInfo);
void       OpalRenderpassShutdown         (OpalRenderpass* pRenderpass);

// Framebuffer ==========
OpalResult OpalFramebufferInit            (OpalFramebuffer* pFramebuffer, OpalFramebufferInitInfo initInfo);
void       OpalFramebufferShutdown        (OpalFramebuffer* pFramebuffer);

// Shader ==========
OpalResult OpalShaderInit                 (OpalShader* pShader, OpalShaderInitInfo initInfo);
void       OpalShaderShutdown             (OpalShader* pShader);

// ShaderGroup ==========
OpalResult OpalShaderGroupInit            (OpalShaderGroup* pShaderGroup, OpalShaderGroupInitInfo initInfo);
void       OpalShaderGroupShutdown        (OpalShaderGroup* pShaderGroup);

// ShaderInput* ==========
OpalResult OpalShaderInputLayoutInit      (OpalShaderInputLayout* pLayout, OpalShaderInputLayoutInitInfo initInfo);
void       OpalShaderInputLayoutShutdown  (OpalShaderInputLayout* pLayout);
OpalResult OpalShaderInputInit            (OpalShaderInput* pShaderInput, OpalShaderInputInitInfo initInfo);
void       OpalShaderInputShutdown        (OpalShaderInput* pShaderInput);

// ==============================
// Rendering
// ==============================

OpalResult OpalRenderBegin                ();
OpalResult OpalRenderEnd                  ();
OpalResult OpalRenderToWindowBegin        (OpalWindow* pWindow);
OpalResult OpalRenderToWindowEnd          (OpalWindow* pWindow);
void       OpalRenderRenderpassBegin      (const OpalRenderpass* pRenderpass, const OpalFramebuffer* pFramebuffer);
void       OpalRenderRenderpassEnd        (const OpalRenderpass* pRenderpass);
void       OpalRenderSetViewportDimensions(uint32_t width, uint32_t height);
void       OpalRenderBindShaderGroup      (const OpalShaderGroup* pGroup);
void       OpalRenderBindShaderInput      (const OpalShaderInput* pInput, uint32_t setIndex);
void       OpalRenderSetPushConstant      (const void* data);
void       OpalRenderMesh                 (const OpalMesh* pMesh);

void OpalRenderDrawCount_DEBUG(uint32_t vertexCount);

#ifdef __cplusplus
}
#endif

#endif // !OPAL_H
