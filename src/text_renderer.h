/**
 * @file text_renderer.h
 * @author Christian Saloň
 */

#pragma once

#include <vector>
#include <memory>

#include <vulkan/vulkan.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "window.h"

/**
 * @class TextRenderer
 * 
 * @brief Creates vulkan vertex and index buffer for specified character
 */
class TextRenderer {

public:

    static const int LOD = 32; /**< Bezier curve level of detail */

    /**
     * @brief Represents a 2D vertex
     */
    typedef struct vertex {
        float x;    /**< X coordinate of vertex */
        float y;    /**< Y coordinate of vertex */

        static VkVertexInputBindingDescription getVertexInutBindingDescription() {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static VkVertexInputAttributeDescription getVertexInputAttributeDescription() {
            VkVertexInputAttributeDescription attributeDescriptions{};
            attributeDescriptions.binding = 0;
            attributeDescriptions.location = 0;
            attributeDescriptions.format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions.offset = 0;

            return attributeDescriptions;
        }
    } vertex_t;

    /**
     * @brief Represents an edge between two vertices
     */
    typedef struct edge {
        uint16_t firstVertexIndex;  /**< First vertex index */
        uint16_t secondVertexIndex; /**< Second vertex index */
    } edge_t;

private:

    std::vector<vertex_t> _vertices;                /**< Character vertices */
    std::vector<edge_t> _edges;                     /**< Character edges */
    std::vector<uint16_t> _indices;                 /**< Character vertex indices */

    FT_Library _ft;                                 /**< Freetype library */
    FT_Face _face;                                  /**< Freetype face */
    static FT_Outline_MoveToFunc _moveToFunc;
    static FT_Outline_LineToFunc _lineToFunc;
    static FT_Outline_ConicToFunc _conicToFunc;
    static FT_Outline_CubicToFunc _cubicToFunc;

    std::shared_ptr<MainWindow> _window;            /**< Window object */

    VkPhysicalDevice _physicalDevice;               /**< Vulkan physical device */
    VkDevice _logicalDevice;                        /**< Vulkan logical device */
    VkBuffer _vertexBuffer;                         /**< Vulkan vertex buffer */
    VkDeviceMemory _vertexBufferMemory;             /**< Vulkan vertex buffer memory */
    VkBuffer _indexBuffer;                          /**< Vulkan index buffer */
    VkDeviceMemory _indexBufferMemory;              /**< Vulkan index buffer memory */
    VkQueue _graphicsQueue;                         /**< Vulkan graphics queue */
    VkCommandPool _commandPool;                     /**< Vulkan command pool */

public:

    static TextRenderer &getInstance();
    void init(std::string fontFilePath, std::shared_ptr<MainWindow> window, VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
    void destroy();

    void renderGlyph(uint32_t codePoint);

    uint32_t getVertexCount();
    uint32_t getIndexCount();
    VkBuffer getVertexBuffer();
    VkBuffer getIndexBuffer();

    void setWindow(std::shared_ptr<MainWindow> window);
    void setPhysicalDevice(VkPhysicalDevice physicalDevice);
    void setLogicalDevice(VkDevice logicalDevice);
    void setCommandPool(VkCommandPool commandPool);
    void setGraphicsQueue(VkQueue graphicsQueue);

private:

    TextRenderer();
    ~TextRenderer();

    void _detailBezier(vertex_t startPoint, vertex_t controlPoint, vertex_t endPoint);
    vertex_t _normalizeVertex(float x, float y);

    void _createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    void _copyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize size);
    void _createVertexBuffer();
    void _createIndexBuffer();
    void _destroyBuffer(VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    uint32_t _selectMemoryType(uint32_t memoryType, VkMemoryPropertyFlags properties);

    TextRenderer(const TextRenderer &) = delete;
    TextRenderer &operator=(const TextRenderer &) = delete;
};
