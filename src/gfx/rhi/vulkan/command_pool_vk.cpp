

#include "gfx/rhi/vulkan/command_pool_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

bool CommandBufferManagerVk::createPool(uint32_t QueueIndex) {
  VkCommandPoolCreateInfo poolInfo = {};
  poolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex        = QueueIndex;

  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;  // Optional

  if (vkCreateCommandPool(
          g_rhiVk->m_device_, &poolInfo, nullptr, &m_commandPool_)
      != VK_SUCCESS) {
    // TODO: Handle errors appropriately
    return false;
  }
  return true;
}

void CommandBufferManagerVk::releaseInternal() {
  ScopedLock s(&m_commandListLock_);

  // vkFreeCommandBuffers(device, m_commandBufferManager.getPool(),
  // static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

  for (auto& iter : m_usingCommandBuffers_) {
    iter->getFence()->waitForFence();
    g_rhiVk->getFenceManager()->returnFence(iter->getFence());
    // delete iter;
  }
  m_usingCommandBuffers_.clear();

  for (auto& iter : m_availableCommandBuffers_) {
    iter->getFence()->waitForFence();
    g_rhiVk->getFenceManager()->returnFence(iter->getFence());
    // delete iter;
  }
  m_availableCommandBuffers_.clear();

  if (m_commandPool_) {
    vkDestroyCommandPool(g_rhiVk->m_device_, m_commandPool_, nullptr);
    m_commandPool_ = nullptr;
  }
}

std::shared_ptr<CommandBuffer>
    CommandBufferManagerVk::getOrCreateCommandBuffer() {
  ScopedLock s(&m_commandListLock_);

  std::shared_ptr<CommandBufferVk> SelectedCommandBuffer = nullptr;
  if (m_availableCommandBuffers_.size() > 0) {
    for (int32_t i = 0; i < (int32_t)m_availableCommandBuffers_.size(); ++i) {
      const VkResult Result = vkGetFenceStatus(
          g_rhiVk->m_device_,
          (VkFence)m_availableCommandBuffers_[i]->getFenceHandle());
      if (Result == VK_SUCCESS) {
        std::shared_ptr<CommandBufferVk> commandBuffer
            = m_availableCommandBuffers_[i];
        m_availableCommandBuffers_.erase(m_availableCommandBuffers_.begin()
                                         + i);

        m_usingCommandBuffers_.push_back(commandBuffer);
        commandBuffer->reset();
        SelectedCommandBuffer = commandBuffer;
        break;
      }
    }
  }

  if (!SelectedCommandBuffer) {
    // TODO: separate to function alloc
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = getPool();
    allocInfo.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer vkCommandBuffer = nullptr;
    if (vkAllocateCommandBuffers(
            g_rhiVk->m_device_, &allocInfo, &vkCommandBuffer)
        != VK_SUCCESS) {
      GlobalLogger::Log(LogLevel::Error, "Failed to allocate command buffer");
      return nullptr;
    }

    auto newCommandBuffer      = std::make_shared<CommandBufferVk>();
    newCommandBuffer->getRef() = vkCommandBuffer;
    newCommandBuffer->setFence(g_rhiVk->m_fenceManager_->getOrCreateFence());

    m_usingCommandBuffers_.push_back(newCommandBuffer);

    SelectedCommandBuffer = newCommandBuffer;
  }
  SelectedCommandBuffer->begin();
  return SelectedCommandBuffer;
}

void CommandBufferManagerVk::returnCommandBuffer(
    std::shared_ptr<CommandBuffer> commandBuffer) {
  ScopedLock s(&m_commandListLock_);
  // auto       it = std::find(
  //     UsingCommandBuffers.begin(), UsingCommandBuffers.end(), commandBuffer);
  // if (it != UsingCommandBuffers.end()) {
  //   UsingCommandBuffers.erase(it);
  //   // TODO: temoporary removed
  //   //commandBuffer->Reset();  // Reset the command buffer before reusing
  //   AvailableCommandBuffers.push_back(commandBuffer);
  // }
  for (int32_t i = 0; i < m_usingCommandBuffers_.size(); ++i) {
    if (m_usingCommandBuffers_[i]->getNativeHandle()
        == commandBuffer->getNativeHandle()) {
      // std::cout << "------------------------------------\n";
      // GlobalLogger::Log(LogLevel::Debug,
      //                   "Number of UsingCommandBuffers before: "
      //                       + std::to_string(UsingCommandBuffers.size()));

      m_usingCommandBuffers_.erase(m_usingCommandBuffers_.begin() + i);
      auto commandBufferVk
          = std::static_pointer_cast<CommandBufferVk>(commandBuffer);
      m_availableCommandBuffers_.push_back(commandBufferVk);

      // GlobalLogger::Log(LogLevel::Debug,
      //                   "Number of UsingCommandBuffers after: "
      //                       + std::to_string(UsingCommandBuffers.size()));

      return;
    }
  }

  GlobalLogger::Log(
      LogLevel::Warning,
      "Command buffer not found in UsingCommandBuffers and not deleted.");
}

}  // namespace game_engine
