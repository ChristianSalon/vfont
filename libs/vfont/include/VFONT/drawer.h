/**
 * @file drawer.h
 * @author Christian Saloň
 */

#pragma once

#include <array>
#include <cstdint>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

#include "glyph_cache.h"
#include "text_block.h"
#include "text_renderer_utils.h"

namespace vft {

class Drawer {
protected:
    GlyphCache &_cache;

    VkPhysicalDevice _physicalDevice{nullptr}; /**< Vulkan physical device */
    VkDevice _logicalDevice{nullptr};          /**< Vulkan logical device */
    VkCommandPool _commandPool{nullptr};       /**< Vulkan command pool */
    VkQueue _graphicsQueue{nullptr};           /**< Vulkan graphics queue */
    VkRenderPass _renderPass{nullptr};

    VkDescriptorPool _descriptorPool{nullptr};

    VkDescriptorSetLayout _uboDescriptorSetLayout{nullptr};
    VkDescriptorSet _uboDescriptorSet;

    VkBuffer _ubo;
    VkDeviceMemory _uboMemory;
    void *_mappedUbo;

public:
    Drawer(GlyphCache &cache);
    ~Drawer();

    virtual void init(VkPhysicalDevice physicalDevice,
                      VkDevice logicalDevice,
                      VkCommandPool commandPool,
                      VkQueue graphicsQueue,
                      VkRenderPass renderPass);
    void setUniformBuffers(vft::UniformBufferObject ubo);

    virtual void draw(std::vector<std::shared_ptr<TextBlock>> textBlocks, VkCommandBuffer commandBuffer) = 0;
    virtual void recreateBuffers(std::vector<std::shared_ptr<TextBlock>> textBlocks) = 0;

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
