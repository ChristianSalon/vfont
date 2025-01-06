/**
 * @file timed_renderer.cpp
 * @author Christian Saloň
 */

#include "timed_renderer.h"

namespace vft {

TimedRenderer::TimedRenderer(Renderer *renderer) : RendererDecorator{renderer}, _queryPool{nullptr} {}

TimedRenderer::~TimedRenderer() {}

void TimedRenderer::init(TessellationStrategy tessellationStrategy, VulkanContext vulkanContext) {
    RendererDecorator::init(tessellationStrategy, vulkanContext);

    this->_timestampPeriod = this->_getTimestampPeriod();

    // Create vulkan query pool
    VkQueryPoolCreateInfo queryPoolCreateInfo{};
    queryPoolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    queryPoolCreateInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
    queryPoolCreateInfo.queryCount = 2;

    if (vkCreateQueryPool(this->getVulkanContext().logicalDevice, &queryPoolCreateInfo, nullptr, &this->_queryPool) !=
        VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan timestamp query pool.");
    }
}

void TimedRenderer::destroy() {
    vkDestroyQueryPool(this->_renderer->getVulkanContext().logicalDevice, this->_queryPool, nullptr);

    RendererDecorator::destroy();
};

void TimedRenderer::draw(VkCommandBuffer commandBuffer) {
    vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, this->_queryPool, 0);

    RendererDecorator::draw(commandBuffer);

    vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, this->_queryPool, 1);
}

double TimedRenderer::readTimestamps(VkCommandBuffer commandBuffer) {
    vkCmdResetQueryPool(commandBuffer, this->_queryPool, 0, 2);

    uint64_t timestamps[2];
    VkResult result =
        vkGetQueryPoolResults(this->getVulkanContext().logicalDevice, this->_queryPool, 0, 2, sizeof(timestamps),
                              timestamps, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Error reading timestamps from vulkan query pool.");
    }

    double time = (timestamps[1] - timestamps[0]) * this->_timestampPeriod;
    return time;
}

void TimedRenderer::resetQueryPool(VkCommandBuffer commandBuffer) {
    vkCmdResetQueryPool(commandBuffer, this->_queryPool, 0, 2);
}

double TimedRenderer::_getTimestampPeriod() {
    VkPhysicalDeviceProperties deviceProperties{};
    vkGetPhysicalDeviceProperties(this->_renderer->getVulkanContext().physicalDevice, &deviceProperties);

    return deviceProperties.limits.timestampPeriod;
}

};  // namespace vft