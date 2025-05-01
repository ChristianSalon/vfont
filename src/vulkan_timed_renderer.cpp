/**
 * @file vulkan_timed_renderer.cpp
 * @author Christian Saloň
 */

#include "vulkan_timed_renderer.h"

namespace vft {

/**
 * @brief VulkanTimedRenderer constructor
 *
 * @param renderer Wrapped vulkan text renderer
 */
VulkanTimedRenderer::VulkanTimedRenderer(VulkanTextRenderer *renderer) : VulkanTextRendererDecorator{renderer} {
    this->_timestampPeriod = this->_getTimestampPeriod();

    // Create vulkan query pool
    VkQueryPoolCreateInfo queryPoolCreateInfo{};
    queryPoolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    queryPoolCreateInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
    queryPoolCreateInfo.queryCount = 2;

    if (vkCreateQueryPool(this->_renderer->getLogicalDevice(), &queryPoolCreateInfo, nullptr, &this->_queryPool) !=
        VK_SUCCESS) {
        throw std::runtime_error("VulkanTimedRenderer::initialize(): Error creating vulkan timestamp query pool");
    }
}

/**
 * @brief VulkanTimedRenderer destructor
 */
VulkanTimedRenderer::~VulkanTimedRenderer() {
    if (this->_queryPool != nullptr)
        vkDestroyQueryPool(this->_renderer->getLogicalDevice(), this->_queryPool, nullptr);
}

/**
 * @brief  Add draw commands to the command buffer and add timestamps at top and bottom of pipeline
 */
void VulkanTimedRenderer::draw() {
    vkCmdWriteTimestamp(this->_renderer->getCommandBuffer(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, this->_queryPool, 0);

    VulkanTextRendererDecorator::draw();

    vkCmdWriteTimestamp(this->_renderer->getCommandBuffer(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, this->_queryPool, 1);
}

/**
 * @brief Read timestamps and get draw time
 *
 * @return Draw time
 */
double VulkanTimedRenderer::readTimestamps() {
    uint64_t timestamps[2];
    VkResult result =
        vkGetQueryPoolResults(this->_renderer->getLogicalDevice(), this->_queryPool, 0, 2, sizeof(timestamps),
                              timestamps, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    if (result != VK_SUCCESS) {
        throw std::runtime_error(
            "VulkanTimedRenderer::readTimestamps(): Error reading timestamps from vulkan query pool");
    }

    double time = (timestamps[1] - timestamps[0]) * this->_timestampPeriod;
    return time;
}

/**
 * @brief Reset vulkan query pool
 */
void VulkanTimedRenderer::resetQueryPool() {
    vkCmdResetQueryPool(this->_renderer->getCommandBuffer(), this->_queryPool, 0, 2);
}

/**
 * @brief Get vulkan timestamp period of physical device
 *
 * @return Timestamp period of physical device
 */
double VulkanTimedRenderer::_getTimestampPeriod() {
    VkPhysicalDeviceProperties deviceProperties{};
    vkGetPhysicalDeviceProperties(this->_renderer->getPhysicalDevice(), &deviceProperties);

    return deviceProperties.limits.timestampPeriod;
}

};  // namespace vft