/**
 * @file text_renderer.h
 * @author Christian Saloň
 */

#pragma once

#include <vector>
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

    /**
     * @brief Map containing glyph information.
     * Key: unicode code point, Value: Glyph vertex data and metrics
     */
    std::unordered_map<uint32_t, Glyph> _glyphInfo;
    std::vector<Character> _characters;             /**< Vector of characters to render */

    unsigned int _fontSize;                         /**< Character size used for rendering */
    bool _useKerning;                               /**< Indicates whether to use kerning */
    bool _useWrapping;                              /**< Indicates whether to use word wrapping (line breaking), works only in 2D */
    glm::mat4 _transformMatrix;                     /**< User specified transforms, resets after every add() call */
    int _penX;                                      /**< X coordinate of current pen position (indicates where to render next characters) */
    int _penY;                                      /**< Y coordinate of current pen position (indicates where to render next characters) */

    FT_Library _ft;                                 /**< Freetype library */
    FT_Face _face;                                  /**< Freetype face */
    Glyph _currentGlyph;                            /**< Currently processed glyph */
    uint32_t _vertexId;                             /**< Currently processed glyph's vertex id counter */
    uint32_t _contourStartVertexId;                 /**< Vertex id for the starting vertex of a contour */
    glm::vec2 _lastVertex;                          /**< Last vertex processed */
    bool _isFirstContour;                           /**< Indicates whether currently processed contour is the first one */
    static FT_Outline_MoveToFunc _moveToFunc;
    static FT_Outline_LineToFunc _lineToFunc;
    static FT_Outline_ConicToFunc _conicToFunc;
    static FT_Outline_CubicToFunc _cubicToFunc;

    unsigned int _viewportWidth;                    /**< Target window's width */
    unsigned int _viewportHeight;                   /**< Target window's height */

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
        unsigned int fontSize,
        std::string fontFilePath,
        unsigned int viewportWidth,
        unsigned int viewportHeight,
        VkPhysicalDevice physicalDevice,
        VkDevice logicalDevice,
        VkCommandPool commandPool,
        VkQueue graphicsQueue,
        VkPipelineLayout pipelineLayout,
        bool useKerning = true,
        bool useWrapping = false
    );
    void draw(VkCommandBuffer commandBuffer);
    void destroy();

    void add(std::vector<uint32_t> codePoints);
    void add(std::vector<uint32_t> codePoints, int x, int y);
    void add(std::vector<uint32_t> codePoints, std::vector<glm::mat4> transforms);
    void remove(unsigned int count = 1);

    void useKerning(bool kerning);
    void useWrapping(bool wrapping);
    void scale(float x, float y, float z);
    void translate(float x, float y, float z);
    void rotate(float x, float y, float z);
    void setViewport(unsigned int width, unsigned int height);

    std::vector<Character> getCharacters();
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
        std::map<float, glm::vec2> &vertices
    );
    void _initializeGlyphInfo();

    void _renderGlyph(uint32_t codePoint);
    void _decomposeGlyph(uint32_t codePoint);
    void _triangulate();
    void _recreateBuffers();

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
