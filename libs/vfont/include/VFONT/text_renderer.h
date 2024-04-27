/**
 * @file text_renderer.h
 * @author Christian Saloň
 */

#pragma once

#include <vector>
#include <array>
#include <memory>
#include <unordered_map>
#include <map>
#include <cstdint>

#include <vulkan/vulkan.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include "glyph.h"
#include "character.h"
#include "text_renderer_utils.h"
#include "text_block.h"

namespace vft {

/**
 * @class TextRenderer
 * 
 * @brief Creates vertex and index buffers for specified characters
 */
class TextRenderer {

protected:

    std::vector<glm::vec2> _vertices;                   /**< Vertex buffer */
    std::vector<uint32_t> _indices;                     /**< Index buffer */
    std::vector<std::shared_ptr<TextBlock>> _blocks;    /**< All text blocks to be rendered */

    VkPhysicalDevice _physicalDevice;                   /**< Vulkan physical device */
    VkDevice _logicalDevice;                            /**< Vulkan logical device */
    VkQueue _graphicsQueue;                             /**< Vulkan graphics queue */
    VkCommandPool _commandPool;                         /**< Vulkan command pool */
    VkPipelineLayout _pipelineLayout;                   /**< Vulkan pipeline layout */
    VkBuffer _vertexBuffer;                             /**< Vulkan vertex buffer */
    VkDeviceMemory _vertexBufferMemory;                 /**< Vulkan vertex buffer memory */
    VkBuffer _indexBuffer;                              /**< Vulkan index buffer */
    VkDeviceMemory _indexBufferMemory;                  /**< Vulkan index buffer memory */

public:

    TextRenderer();
    TextRenderer(
        VkPhysicalDevice physicalDevice,
        VkDevice logicalDevice,
        VkCommandPool commandPool,
        VkQueue graphicsQueue,
        VkPipelineLayout pipelineLayout);
    ~TextRenderer();

    void init(
        VkPhysicalDevice physicalDevice,
        VkDevice logicalDevice,
        VkCommandPool commandPool,
        VkQueue graphicsQueue,
        VkPipelineLayout pipelineLayout);
    void destroy();
    void draw(VkCommandBuffer commandBuffer);
    void add(std::shared_ptr<TextBlock> text);
    void recreateBuffers();

    uint32_t getVertexCount();
    uint32_t getIndexCount();
    VkBuffer getVertexBuffer();
    VkBuffer getIndexBuffer();

protected:

    void _createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    void _copyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize size);
    void _createVertexBuffer();
    void _createIndexBuffer();
    void _destroyBuffer(VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    uint32_t _selectMemoryType(uint32_t memoryType, VkMemoryPropertyFlags properties);

    void _setPhysicalDevice(VkPhysicalDevice physicalDevice);
    void _setLogicalDevice(VkDevice logicalDevice);
    void _setCommandPool(VkCommandPool commandPool);
    void _setGraphicsQueue(VkQueue graphicsQueue);
    void _setPipelineLayout(VkPipelineLayout pipelineLayout);

};

}
