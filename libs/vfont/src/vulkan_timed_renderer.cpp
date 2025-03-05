/**
 * @file vulkan_timed_renderer.cpp
 * @author Christian Saloň
 */

#include "vulkan_timed_renderer.h"

namespace vft {

VulkanTimedRenderer::VulkanTimedRenderer(VulkanTextRenderer *renderer)
    : VulkanTextRendererDecorator{renderer}, _queryPool{nullptr} {}

VulkanTimedRenderer::~VulkanTimedRenderer() {}

void VulkanTimedRenderer::initialize() {
    VulkanTextRendererDecorator::initialize();

    this->_timestampPeriod = this->_getTimestampPeriod();

    // Create vulkan query pool
    VkQueryPoolCreateInfo queryPoolCreateInfo{};
    queryPoolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    queryPoolCreateInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
    queryPoolCreateInfo.queryCount = 2;

    if (vkCreateQueryPool(this->_renderer->getLogicalDevice(), &queryPoolCreateInfo, nullptr, &this->_queryPool) !=
        VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan timestamp query pool.");
    }
}

void VulkanTimedRenderer::destroy() {
    if (this->_queryPool != nullptr)
        vkDestroyQueryPool(this->_renderer->getLogicalDevice(), this->_queryPool, nullptr);

    VulkanTextRendererDecorator::destroy();
};

void VulkanTimedRenderer::draw() {
    vkCmdWriteTimestamp(this->_renderer->getCommandBuffer(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, this->_queryPool, 0);

    VulkanTextRendererDecorator::draw();

    vkCmdWriteTimestamp(this->_renderer->getCommandBuffer(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, this->_queryPool, 1);
}

double VulkanTimedRenderer::readTimestamps() {
    vkCmdResetQueryPool(this->_renderer->getCommandBuffer(), this->_queryPool, 0, 2);

    uint64_t timestamps[2];
    VkResult result =
        vkGetQueryPoolResults(this->_renderer->getLogicalDevice(), this->_queryPool, 0, 2, sizeof(timestamps),
                              timestamps, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Error reading timestamps from vulkan query pool.");
    }

    double time = (timestamps[1] - timestamps[0]) * this->_timestampPeriod;
    return time;
}

void VulkanTimedRenderer::resetQueryPool() {
    vkCmdResetQueryPool(this->_renderer->getCommandBuffer(), this->_queryPool, 0, 2);
}

double VulkanTimedRenderer::_getTimestampPeriod() {
    VkPhysicalDeviceProperties deviceProperties{};
    vkGetPhysicalDeviceProperties(this->_renderer->getPhysicalDevice(), &deviceProperties);

    return deviceProperties.limits.timestampPeriod;
}

};  // namespace vft