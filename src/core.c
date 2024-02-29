
#include "src/common.h"
#include "src/vulkan/vulkan_common.h"

#include <stdarg.h>

// ============================================================
// ============================================================
//
// Global variables ==========
// g_OpalState
//
// Core ==========
// OpalOutputMessage()
// OpalInit()
//
// ============================================================
// ============================================================


// Global variables
// ============================================================

OpalState g_OpalState = {};

// Core
// ============================================================

void OpalOutputMessage(OpalMessageType type, const char* message, ...)
{
  if (g_OpalState.messageCallback != NULL)
  {
    char messageBuffer[1024];

    va_list args;
    va_start(args, message);
    vsnprintf(messageBuffer, 1024, message, args);
    va_end(args);

    g_OpalState.messageCallback(type, messageBuffer);
  }
}

OpalResult OpalInit(OpalInitInfo initInfo)
{
  switch (initInfo.api)
  {
  case Opal_Api_Vulkan:
  {
    OPAL_ATTEMPT(OpalVulkanInit(initInfo));
    g_OpalState.api.functions.WindowInit = OpalVulkanWindowInit;

  } break;
  default:
  {
    OpalLogError("Invalid Api type : %d", initInfo.api);
    return Opal_Failure_Unknown;
  }
  }

  return Opal_Success;
}

OpalResult OpalWindowInit(OpalWindowInitInfo initInfo, OpalWindow* pWindow)
{
  return g_OpalState.api.functions.WindowInit(initInfo, pWindow);
}
