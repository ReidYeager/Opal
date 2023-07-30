
#include "src/common.h"

OpalResult OvkBeginSingleUseCommand(VkCommandPool _pool, VkCommandBuffer* _cmd)
{
  VkCommandBufferAllocateInfo allocInfo = { 0 };
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = _pool;
  allocInfo.commandBufferCount = 1;

  OVK_ATTEMPT(vkAllocateCommandBuffers(oState.vk.device, &allocInfo, _cmd));

  VkCommandBufferBeginInfo beginInfo = { 0 };
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  beginInfo.pInheritanceInfo = NULL;

  OVK_ATTEMPT(vkBeginCommandBuffer(*_cmd, &beginInfo));

  return Opal_Success;
}

OpalResult OvkEndSingleUseCommand(VkCommandPool _pool, VkQueue _queue, VkCommandBuffer _cmd)
{
  OVK_ATTEMPT(vkEndCommandBuffer(_cmd));

  VkSubmitInfo submitInfo = { 0 };
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &_cmd;

  OVK_ATTEMPT(vkQueueSubmit(_queue, 1, &submitInfo, VK_NULL_HANDLE));

  OVK_ATTEMPT(vkQueueWaitIdle(_queue));

  vkFreeCommandBuffers(oState.vk.device, _pool, 1, &_cmd);
  return Opal_Success;
}
