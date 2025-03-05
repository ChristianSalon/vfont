/**
 * @file vulkan_text_renderer.h
 * @author Christian Saloň
 */

#pragma once

#include <fstream>
#include <memory>
#include <stdexcept>

#include <vulkan/vulkan.h>

#include "i_vulkan_text_renderer.h"
#include "text_renderer_utils.h"

namespace vft {

class VulkanTextRenderer : public IVulkanTextRenderer {
protected:
    VkPhysicalDevice _physicalDevice{nullptr};  /// Vulkan physical device
    VkDevice _logicalDevice{nullptr};           /// Vulkan logical device
    VkQueue _graphicsQueue{nullptr};            /// Vulkan graphics queue
    VkCommandPool _commandPool{nullptr};        /// Vulkan command pool
    VkRenderPass _renderPass{nullptr};          /// Vulkan render pass
    VkCommandBuffer _commandBuffer{nullptr};    /// Vulkan command buffer used in draw calls

    VkDescriptorPool _descriptorPool{nullptr};
    VkDescriptorSetLayout _uboDescriptorSetLayout{nullptr};
    VkDescriptorSet _uboDescriptorSet{nullptr};

    VkBuffer _uboBuffer{nullptr};
    VkDeviceMemory _uboMemory{nullptr};
    void *_mappedUbo{nullptr};

public:
    VulkanTextRenderer() = default;
    ~VulkanTextRenderer() = default;

    void initialize() override;
    void destroy() override;

    void setUniformBuffers(UniformBufferObject ubo) override;

    void setPhysicalDevice(VkPhysicalDevice physicalDevice) override;
    void setLogicalDevice(VkDevice logicalDevice) override;
    void setCommandPool(VkCommandPool commandPool) override;
    void setGraphicsQueue(VkQueue graphicsQueue) override;
    void setRenderPass(VkRenderPass renderPass) override;
    void setCommandBuffer(VkCommandBuffer commandBuffer) override;

    VkPhysicalDevice getPhysicalDevice() override;
    VkDevice getLogicalDevice() override;
    VkCommandPool getCommandPool() override;
    VkQueue getGraphicsQueue() override;
    VkRenderPass getRenderPass() override;
    VkCommandBuffer getCommandBuffer() override;

protected:
    virtual void _createDescriptorPool();

    void _createUbo();
    void _createUboDescriptorSetLayout();
    void _createUboDescriptorSet();

    std::vector<char> _readFile(std::string fileName);
    VkShaderModule _createShaderModule(const std::vector<char> &shaderCode);

    void _stageAndCreateVulkanBuffer(void *data,
                                     VkDeviceSize size,
                                     VkBufferUsageFlags destinationUsage,
                                     VkBuffer &destinationBuffer,
                                     VkDeviceMemory &destinationMemory);
    void _createBuffer(VkDeviceSize size,
                       VkBufferUsageFlags usage,
                       VkMemoryPropertyFlags properties,
                       VkBuffer &buffer,
                       VkDeviceMemory &bufferMemory);
    void _copyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize size);
    void _destroyBuffer(VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    uint32_t _selectMemoryType(uint32_t memoryType, VkMemoryPropertyFlags properties);
};

}  // namespace vft
