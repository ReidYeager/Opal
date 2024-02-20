
#include "src/common.h"

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
  //if (oState.messageCallback != NULL)
  //{
  //  char messageBuffer[1024];

  //  va_list args;
  //  va_start(args, message);
  //  vsnprintf(messageBuffer, 1024, message, args);
  //  va_end(args);

  //  oState.messageCallback(type, messageBuffer);
  //}
}

OpalResult OpalInit(OpalInitInfo initInfo)
{
  switch (initInfo.api)
  {
  case Opal_Api_Vulkan:
  {
    OPAL_ATTEMPT(OpalVulkanInit(initInfo));
  } break;
  default:
  {
    OpalLogError("Invalid Api type : %d", initInfo.api);
    return Opal_Failure_Unknown;
  }
  }

  return Opal_Success;
}
