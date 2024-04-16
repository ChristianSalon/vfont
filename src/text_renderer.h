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

public:

    static const uint32_t U_BACKSPACE;
    static const uint32_t U_ENTER;
    static const uint32_t U_SPACE;
    static const uint32_t U_TAB;

    static const unsigned int DEFAULT_FONT_SIZE;    /**< Default font size */

private:

    std::vector<glm::vec2> _vertices;               /**< Vertex buffer */
    std::vector<uint32_t> _indices;                 /**< Index buffer */

    std::vector<std::shared_ptr<TextBlock>> _blocks;

    Glyph _currentGlyph;                            /**< Currently processed glyph */
    uint32_t _vertexId;                             /**< Currently processed glyph's vertex id counter */
    uint32_t _contourStartVertexId;                 /**< Vertex id for the starting vertex of a contour */
    glm::vec2 _lastVertex;                          /**< Last vertex processed */
    bool _isFirstContour;                           /**< Indicates whether currently processed contour is the first one */
    static FT_Outline_MoveToFunc _moveToFunc;
    static FT_Outline_LineToFunc _lineToFunc;
    static FT_Outline_ConicToFunc _conicToFunc;
    static FT_Outline_CubicToFunc _cubicToFunc;

    VkPhysicalDevice _physicalDevice;               /**< Vulkan physical device */
    VkDevice _logicalDevice;                        /**< Vulkan logical device */
    VkBuffer _vertexBuffer;                         /**< Vulkan vertex buffer */
    VkDeviceMemory _vertexBufferMemory;             /**< Vulkan vertex buffer memory */
    VkBuffer _indexBuffer;                          /**< Vulkan index buffer */
    VkDeviceMemory _indexBufferMemory;              /**< Vulkan index buffer memory */
    VkQueue _graphicsQueue;                         /**< Vulkan graphics queue */
    VkCommandPool _commandPool;                     /**< Vulkan command pool */
    VkPipelineLayout _pipelineLayout;

public:

    static TextRenderer &getInstance();
    void init(
        VkPhysicalDevice physicalDevice,
        VkDevice logicalDevice,
        VkCommandPool commandPool,
        VkQueue graphicsQueue,
        VkPipelineLayout pipelineLayout);
    void draw(VkCommandBuffer commandBuffer);
    void destroy();

    void add(std::shared_ptr<TextBlock> text);
    void renderGlyph(uint32_t codePoint, std::shared_ptr<Font> font);
    void recreateBuffers();

    uint32_t getVertexCount();
    uint32_t getIndexCount();
    VkBuffer getVertexBuffer();
    VkBuffer getIndexBuffer();


private:

    TextRenderer();
    ~TextRenderer();

    void _detailBezier(glm::vec2 startPoint, glm::vec2 controlPoint, glm::vec2 endPoint);
    void _subdivide(
        glm::vec2 startPoint,
        glm::vec2 controlPoint,
        glm::vec2 endPoint,
        float t,
        float delta,
        std::map<float, glm::vec2> &vertices);
    double _determinant(double a, double b, double c, double d);
    bool _isPointOnLineSegment(double x1, double y1, double x2, double y2, double x, double y);
    bool _intersect(
        const std::vector<glm::vec2> &vertices,
        vft::edge_t first,
        vft::edge_t second,
        glm::vec2 &intersection);
    void _checkIntersectingEdges(std::vector<glm::vec2> &vertices, std::vector<vft::edge_t> &edges);

    void _decomposeGlyph(uint32_t codePoint, std::shared_ptr<Font> font);
    void _triangulate();

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

    TextRenderer(const TextRenderer &) = delete;
    TextRenderer &operator=(const TextRenderer &) = delete;
};

}
