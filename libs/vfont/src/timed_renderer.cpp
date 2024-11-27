/**
 * @file timed_renderer.cpp
 * @author Christian Saloň
 */

#include "timed_renderer.h"

namespace vft {

TimedRenderer::TimedRenderer(Renderer *renderer, VkDevice logicalDevice)
    : RendererDecorator{renderer}, _logicalDevice{logicalDevice}, _queryPool{nullptr} {
    // Create vulkan query pool
    VkQueryPoolCreateInfo queryPoolCreateInfo{};
    queryPoolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    queryPoolCreateInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
    queryPoolCreateInfo.queryCount = 2;

    if (vkCreateQueryPool(this->_logicalDevice, &queryPoolCreateInfo, nullptr, &this->_queryPool) != VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan timestamp query pool.");
    }
}

TimedRenderer::~TimedRenderer() {
    // vkDestroyQueryPool(this->_logicalDevice, this->_queryPool, nullptr);
    // delete this->_renderer;
}

void TimedRenderer::draw(VkCommandBuffer commandBuffer) {
    vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, this->_queryPool, 0);

    RendererDecorator::draw(commandBuffer);

    vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, this->_queryPool, 1);
}

double TimedRenderer::readTimestamps(VkCommandBuffer commandBuffer) {
    vkCmdResetQueryPool(commandBuffer, this->_queryPool, 0, 2);

    uint64_t timestamps[2];
    VkResult result =
        vkGetQueryPoolResults(this->_logicalDevice, this->_queryPool, 0, 2, sizeof(timestamps), timestamps,
                              sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Error reading timestamps from vulkan query pool.");
    }

    double timestampPeriod = 1e-9;
    double time = (timestamps[1] - timestamps[0]) * timestampPeriod;

    return time;
}

};  // namespace vft