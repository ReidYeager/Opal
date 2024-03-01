#ifndef OPAL_H
#define OPAL_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "./defines.h"

OpalResult OpalInit(OpalInitInfo initInfo);

// ==============================
// Objects
// ==============================

// Window ==========
OpalResult OpalWindowInit         (OpalWindow* pWindow, OpalWindowInitInfo initInfo);
void       OpalWindowShutdown     (OpalWindow* pWindow);

// Buffer ==========
OpalResult OpalBufferInit    (OpalBuffer* pBuffer, OpalBufferInitInfo initInfo);
void       OpalBufferShutdown(OpalBuffer* pBuffer);

// Image ==========
OpalResult OpalImageInit    (OpalImage* pImage, OpalImageInitInfo initInfo);
void       OpalImageShutdown(OpalImage* pImage);

// Renderpass ==========
OpalResult OpalRenderpassInit    (OpalRenderpass* pRenderpass, OpalRenderpassInitInfo initInfo);
void       OpalRenderpassShutdown(OpalRenderpass* pRenderpass);

// Framebuffer ==========
OpalResult OpalFramebufferInit    (OpalFramebuffer* pFramebuffer, OpalFramebufferInitInfo initInfo);
void       OpalFramebufferShutdown(OpalFramebuffer* pFramebuffer);

// Shader ==========
OpalResult OpalShaderInit    (OpalShader* pShader, OpalShaderInitInfo initInfo);
void       OpalShaderShutdown(OpalShader* pShader);

// ShaderGroup ==========
OpalResult OpalShaderGroupInit    (OpalShaderGroup* pShaderGroup, OpalShaderGroupInitInfo initInfo);
void       OpalShaderGroupShutdown(OpalShaderGroup* pShaderGroup);

// ShaderInput ==========
OpalResult OpalShaderInputInit    (OpalShaderInput* pShaderInput, OpalShaderInputInitInfo initInfo);
void       OpalShaderInputShutdown(OpalShaderInput* pShaderInput);

// ==============================
// Rendering
// ==============================

OpalResult OpalRenderBegin        ();
OpalResult OpalRenderEnd          ();
OpalResult OpalRenderToWindowBegin(OpalWindow* pWindow);
OpalResult OpalRenderToWindowEnd  (OpalWindow* pWindow);
void OpalRenderRenderpassBegin    (const OpalRenderpass* pRenderpass, const OpalFramebuffer* pFramebuffer);
void OpalRenderRenderpassEnd      (const OpalRenderpass* pRenderpass);
//void OpalRenderBindShaderInput(const OpalShaderInput* pInput, uint32_t setIndex);
//void OpalRenderBindShaderGroup(const OpalShaderGroup* pGroup);
//void OpalRenderSetViewportDimensions(uint32_t width, uint32_t height);
//void OpalRenderSetPushConstant(void* _data);
//void OpalRenderMesh(OpalMesh _mesh);


#ifdef __cplusplus
}
#endif

#endif // !OPAL_H
