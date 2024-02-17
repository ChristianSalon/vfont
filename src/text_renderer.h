/**
 * @file text_renderer.h
 * @author Christian Saloň
 */

#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include <vulkan/vulkan.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "window.h"
#include "glyph.h"
#include "character.h"
#include "text_renderer_utils.h"

/**
 * @class TextRenderer
 * 
 * @brief Creates vulkan vertex and index buffer for specified character
 */
class TextRenderer {

public:

    static const uint32_t U_BACKSPACE = 0x00000008;
    static const uint32_t U_ENTER = 0x0000000d;
    static const uint32_t U_SPACE = 0x00000020;

    static const unsigned int LOD = 32; /**< Bezier curve level of detail */
    static const unsigned int DEFAULT_FONT_SIZE = 64;

private:

    std::vector<tr::vertex_t> _vertices;            /**< Character vertices */
    std::vector<tr::edge_t> _edges;                 /**< Character edges */
    std::vector<uint32_t> _indices;                 /**< Character vertex indices */

    std::unordered_map<uint32_t, Glyph> _glyphInfo;
    std::vector<Character> _characters;

    unsigned int _fontSize;
    bool _useKerning;
    int _penX;
    int _penY;

    FT_Library _ft;                                 /**< Freetype library */
    FT_Face _face;                                  /**< Freetype face */
    Glyph _currentGlyph;
    uint32_t _vertexId;
    uint32_t _contourStartVertexId;
    tr::vertex_t _lastVertex;
    bool _isFirstContour;
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
    void init(
        unsigned int fontSize,
        bool useKerning,
        std::string fontFilePath,
        std::shared_ptr<MainWindow> window,
        VkPhysicalDevice physicalDevice,
        VkDevice logicalDevice,
        VkCommandPool commandPool,
        VkQueue graphicsQueue
    );
    void destroy();

    void renderGlyph(uint32_t codePoint);
    void renderText(std::vector<uint32_t> codePoints);
    void deleteGlyph();

    std::vector<Character> getCharacters();
    uint32_t getVertexCount();
    uint32_t getIndexCount();
    VkBuffer getVertexBuffer();
    VkBuffer getIndexBuffer();

private:

    TextRenderer();
    ~TextRenderer();

    void _detailBezier(tr::vertex_t startPoint, tr::vertex_t controlPoint, tr::vertex_t endPoint);
    void _initializeGlyphInfo(int fontSize);

    void _decomposeGlyph(uint32_t codePoint);
    void _triangulate();
    void _recreateBuffers();

    void _createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    void _copyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize size);
    void _createVertexBuffer();
    void _createIndexBuffer();
    void _destroyBuffer(VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    uint32_t _selectMemoryType(uint32_t memoryType, VkMemoryPropertyFlags properties);

    void _setWindow(std::shared_ptr<MainWindow> window);
    void _setPhysicalDevice(VkPhysicalDevice physicalDevice);
    void _setLogicalDevice(VkDevice logicalDevice);
    void _setCommandPool(VkCommandPool commandPool);
    void _setGraphicsQueue(VkQueue graphicsQueue);

    TextRenderer(const TextRenderer &) = delete;
    TextRenderer &operator=(const TextRenderer &) = delete;
};
