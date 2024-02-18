/**
 * @file text_renderer.cpp
 * @author Christian Saloň
 */

#include <iostream>
#include <stdexcept>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include "CDT.h"

#include "text_renderer.h"
#include "text_renderer_utils.h"

const uint32_t TextRenderer::U_BACKSPACE = 0x00000008;
const uint32_t TextRenderer::U_ENTER = 0x0000000d;
const uint32_t TextRenderer::U_SPACE = 0x00000020;
const uint32_t TextRenderer::U_TAB = 0x00000009;

const unsigned int TextRenderer::LOD = 32;
const unsigned int TextRenderer::DEFAULT_FONT_SIZE = 64;

/**
 * @brief Text renderer constructor
 */
TextRenderer::TextRenderer() {
    this->_fontSize = TextRenderer::DEFAULT_FONT_SIZE;
    this->_useKerning = false;
    this->_useWrapping = false;
    this->_penX = 0;
    this->_penY = TextRenderer::DEFAULT_FONT_SIZE;

    this->_vertices = {};
    this->_indices = {};
    this->_glyphInfo = {};

    this->_vertexId = 0;
    this->_contourStartVertexId = 0;
    this->_lastVertex = { 0, 0 };
    this->_isFirstContour = true;

    this->_physicalDevice = nullptr;
    this->_logicalDevice = nullptr;
    this->_vertexBuffer = nullptr;
    this->_vertexBufferMemory = nullptr;
    this->_indexBuffer = nullptr;
    this->_indexBufferMemory = nullptr;
}

/**
 * @brief Text renderer destructor
 */
TextRenderer::~TextRenderer() {
    destroy();
}

/**
 * @brief Freetype outline decomposition move_to function.
 * Used when new contour is detected.
 *
 * @param to Contour starting vertex
 * @param user User defined data
 *
 * @return Exit code
 */
FT_Outline_MoveToFunc TextRenderer::_moveToFunc = [](const FT_Vector *to, void *user) {
    TextRenderer *pThis = reinterpret_cast<TextRenderer *>(user);

    pThis->_lastVertex = { static_cast<float>(to->x), static_cast<float>(to->y) };
    if(!pThis->_isFirstContour) {
        // Makes the contour closed
        pThis->_currentGlyph.getEdges().back().secondVertexIndex = pThis->_contourStartVertexId;
    }
    else {
        pThis->_isFirstContour = false;
    }
    pThis->_contourStartVertexId = pThis->_vertexId;

    return 0;
};

/**
 * @brief Freetype outline decomposition line_to function.
 * Used when between two points is a line segment.
 *
 * @param to Line segment ending point
 * @param user User defined data
 *
 * @return Exit code
 */
FT_Outline_LineToFunc TextRenderer::_lineToFunc = [](const FT_Vector *to, void *user) {
    TextRenderer *pThis = reinterpret_cast<TextRenderer *>(user);

    pThis->_lastVertex = { static_cast<float>(to->x), static_cast<float>(to->y) };
    pThis->_currentGlyph.getVertices().push_back(pThis->_lastVertex);
    pThis->_currentGlyph.getEdges().push_back({ pThis->_vertexId, ++(pThis->_vertexId) });

    return 0;
};

/**
 * @brief Freetype outline decomposition conic_to function.
 * Used when there is a quadratic bezier curve.
 *
 * @param control Bezier curve control point
 * @param to Bezier curve ending point
 * @param user User defined data
 *
 * @return Exit code
 */
FT_Outline_ConicToFunc TextRenderer::_conicToFunc = [](const FT_Vector *control, const FT_Vector *to, void *user) {
    TextRenderer *pThis = reinterpret_cast<TextRenderer *>(user);

    tr::vertex_t controlPoint = { static_cast<float>(control->x), static_cast<float>(control->y) };
    tr::vertex_t endPoint = { static_cast<float>(to->x), static_cast<float>(to->y) };

    pThis->_detailBezier(pThis->_lastVertex, controlPoint, endPoint);
    pThis->_lastVertex = endPoint;

    return 0;
};

/**
 * @brief Freetype outline decomposition cubic_to function.
 * Used when there is a cubic bezier curve.
 *
 * @param control1 First bezier curve control point
 * @param control2 Second bezier curve control point
 * @param to Bezier curve ending point
 * @param user User defined data
 *
 * @return Exit code
 */
FT_Outline_CubicToFunc TextRenderer::_cubicToFunc = [](const FT_Vector *control1, const FT_Vector *control2, const FT_Vector *to, void *user) {
    throw std::runtime_error("Fonts with cubic bezier curves are not supported");
    return 0;
};

/**
 * @brief Initializes text renderer
 *
 * @param fontSize Rendered text size in pixels
 * @param useKerning Indicates whether to use kerning
 * @param useWrapping Indicates whether to use wrapping
 * @param fontFilePath File path to .ttf file
 * @param windowWidth Target window width
 * @param windowHeight Target window height
 * @param physicalDevice Vulkan physical device
 * @param logicalDevice Vulkan logical device
 * @param commandPool Vulkan command pool
 * @param graphicsQueue Vulkan graphics queue
 */
void TextRenderer::init(unsigned int fontSize, bool useKerning, bool useWrapping, std::string fontFilePath, unsigned int windowWidth, unsigned int windowHeight, VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue) {
    this->_useKerning = useKerning;
    this->_useWrapping = useWrapping;
    this->_fontSize = fontSize;
    this->_penX = 0;
    this->_penY = this->_fontSize;
    this->_initializeGlyphInfo();

    this->setWindowDimensions(windowWidth, windowHeight);

    this->_setPhysicalDevice(physicalDevice);
    this->_setLogicalDevice(logicalDevice);
    this->_setCommandPool(commandPool);
    this->_setGraphicsQueue(graphicsQueue);

    if(fontFilePath.empty()) {
        throw std::runtime_error("Path to .ttf file was not entered");
    }

    if(FT_Init_FreeType(&(this->_ft))) {
        throw std::runtime_error("Error initializing freetype");
    }

    if(FT_New_Face(this->_ft, fontFilePath.c_str(), 0, &(this->_face))) {
        throw std::runtime_error("Error loading font face, check path to .ttf file");
    }

    FT_Set_Pixel_Sizes(this->_face, this->_fontSize, 0);

    // Check if .ttf file supports freetype kerning
    if(!FT_HAS_KERNING(this->_face) && this->_useKerning == true) {
        std::cout << "Kerning is not supported" << std::endl;
        this->_useKerning = false;
    }
}

/**
 * @brief Destroys freetype and vulkan buffers
 */
void TextRenderer::destroy() {
    // Destroy freetype objects
    if(this->_face != nullptr)
        FT_Done_Face(this->_face);

    if(this->_ft != nullptr)
        FT_Done_FreeType(this->_ft);

    this->_ft = nullptr;
    this->_face = nullptr;

    // Destroy vulkan buffers
    this->_destroyBuffer(this->_indexBuffer, this->_indexBufferMemory);
    this->_destroyBuffer(this->_vertexBuffer, this->_vertexBufferMemory);
}

/**
 * @brief Creates vertex and index buffer for rendering using Freetype and CDT
 *
 * @param codePoint Unicode code point
 */
void TextRenderer::_renderGlyph(uint32_t codePoint) {
    this->_vertexId = 0;
    this->_contourStartVertexId = 0;
    this->_lastVertex = { 0, 0 };
    this->_isFirstContour = true;

    // If code point is TAB, then it is rendered as four SPACE characters
    uint32_t newCodePoint = codePoint == TextRenderer::U_TAB ? TextRenderer::U_SPACE : codePoint;
    if(!this->_glyphInfo.contains(newCodePoint)) {
        this->_decomposeGlyph(newCodePoint);
        this->_triangulate();

        // Insert glyph info to map
        this->_glyphInfo.insert({ newCodePoint, this->_currentGlyph });
    }

    if(codePoint == TextRenderer::U_ENTER) {
        this->_penX = 0;
        this->_penY += this->_fontSize;

        this->_characters.push_back(Character(codePoint, this->_glyphInfo.at(codePoint), this->_penX, this->_penY));
    }
    else if(codePoint == TextRenderer::U_TAB) {
        this->_characters.push_back(Character(codePoint, this->_glyphInfo.at(TextRenderer::U_SPACE), this->_penX, this->_penY));

        for(int i = 0; i < 4; i++) {
            this->_penX += this->_characters.back().glyph.getAdvanceX();
            this->_penY += this->_characters.back().glyph.getAdvanceY();
        }
    }
    else {
        // Apply wrapping if specified
        bool wasWrapped = false;
        if(this->_useWrapping) {
            if(this->_penX + this->_glyphInfo.at(codePoint).getAdvanceX() > this->_windowWidth) {
                // Render character on new line by adding enter character
                wasWrapped = true;
                this->_penX = 0;
                this->_penY += this->_fontSize;
            }
        }

        // Apply kerning if specified
        if(!wasWrapped && this->_useKerning && this->_characters.size() > 0) {
            uint32_t leftCharIndex = FT_Get_Char_Index(this->_face, this->_characters.back().getUnicodeCodePoint());
            uint32_t rightCharIndex = FT_Get_Char_Index(this->_face, codePoint);

            FT_Vector delta;
            FT_Get_Kerning(this->_face, leftCharIndex, rightCharIndex, FT_KERNING_DEFAULT, &delta);
            this->_penX += delta.x >> 6;
        }

        this->_characters.push_back(Character(codePoint, this->_glyphInfo.at(codePoint), this->_penX, this->_penY));

        this->_penX += this->_characters.back().glyph.getAdvanceX();
        this->_penY += this->_characters.back().glyph.getAdvanceY();
    }
}

/**
 * @brief Renders a single character at current pen position
 * 
 * @param codePoint Unicode code point of character
 */
void TextRenderer::renderCharacter(uint32_t codePoint) {
    this->_renderGlyph(codePoint);
    this->_recreateBuffers();
}

/**
 * @brief Renders characters at the origin pen position
 *
 * @param codePoints Vector of unicode code points
 */
void TextRenderer::renderText(std::vector<uint32_t> codePoints) {
    this->_characters.clear();
    this->renderText(codePoints, 0, this->_fontSize);
}

/**
 * @brief Renders characters starting at the specified pen position
 *
 * @param codePoints Vector of unicode code points
 * @param x X coordinate of starting pen position
 * @param y Y coordinate of starting pen position
 */
void TextRenderer::renderText(std::vector<uint32_t> codePoints, int x, int y) {
    this->_penX = x;
    this->_penY = y;

    for(uint32_t codePoint : codePoints) {
        this->_renderGlyph(codePoint);
    }

    this->_recreateBuffers();
}

/**
 * @brief Deletes last character and recreates vertex and index buffers used for rendering
 */
void TextRenderer::deleteCharacter() {
    if(this->_characters.size() == 0) {
        return;
    }
    else if(this->_characters.size() == 1) {
        this->_characters.pop_back();
        this->_penX = 0;
        this->_penY = this->_fontSize;

        return;
    }

    if(this->_characters.back().getUnicodeCodePoint() == TextRenderer::U_ENTER) {
        this->_characters.pop_back();

        this->_penX = this->_characters.back().getX() + this->_characters.back().glyph.getAdvanceX();
        this->_penY -= this->_fontSize;
    }
    else {
        this->_characters.pop_back();

        if(this->_characters.back().getUnicodeCodePoint() == TextRenderer::U_TAB) {
            this->_penX = this->_characters.back().getX();
            this->_penY = this->_characters.back().getY();

            for(int i = 0; i < 4; i++) {
                this->_penX += this->_characters.back().glyph.getAdvanceX();
            }
        }
        else {
            this->_penX = this->_characters.back().getX() + this->_characters.back().glyph.getAdvanceX();
            this->_penY = this->_characters.back().getY();
        }
    }

    this->_recreateBuffers();
}

/**
 * @brief Decomposes a glyph into a set of points and edges
 *
 * @param codePoint Unicode code point of glyph
 */
void TextRenderer::_decomposeGlyph(uint32_t codePoint) {
    // Get glyph from .ttf file
    if(FT_Load_Char(this->_face, codePoint, FT_LOAD_DEFAULT)) {
        throw std::runtime_error("Error loading glyph");
    }
    FT_GlyphSlot slot = this->_face->glyph;

    // Decompose outlines to vertices and vertex indices
    this->_currentGlyph = Glyph();
    FT_Outline_Funcs outlineFunctions = {
        .move_to = this->_moveToFunc,
        .line_to = this->_lineToFunc,
        .conic_to = this->_conicToFunc,
        .cubic_to = this->_cubicToFunc,
        .shift = 0,
        .delta = 0
    };
    FT_Outline_Decompose(&(slot->outline), &outlineFunctions, this);
    if(this->_currentGlyph.getVertexCount() > 0) {
        this->_currentGlyph.updateEdge(
            this->_currentGlyph.getEdgeCount() - 1,
            { this->_currentGlyph.getEdges().back().firstVertexIndex, this->_contourStartVertexId }
        );
    }

    this->_currentGlyph.setAdvanceX(slot->advance.x >> 6);
    this->_currentGlyph.setAdvanceY(slot->advance.y >> 6);
}

/**
 * @brief Performs constrained delaunay triangulation on a decomposed glyph
 */
void TextRenderer::_triangulate() {
    // CDT uses Constrained Delaunay Triangulation algorithm
    CDT::Triangulation<float> cdt(
        CDT::VertexInsertionOrder::Auto,
        CDT::IntersectingConstraintEdges::TryResolve,
        0.001
    );

    /* CDT::RemoveDuplicatesAndRemapEdges(
        this->_vertices,
        [](const tr::vertex_t &p) -> float { return p.x; },
        [](const tr::vertex_t &p) -> float { return p.y; },
        this->_edges.begin(),
        this->_edges.end(),
        [](const tr::edge_t &e) -> CDT::VertInd { return e.firstVertexIndex; },
        [](const tr::edge_t &e) -> CDT::VertInd { return e.secondVertexIndex; },
        [](uint16_t i1, uint16_t i2) -> tr::edge_t { return { i1, i2 }; }
    ); */

    cdt.insertVertices(
        this->_currentGlyph.getVertices().begin(),
        this->_currentGlyph.getVertices().end(),
        [](const tr::vertex_t &p) { return p.x; },
        [](const tr::vertex_t &p) { return p.y; }
    );
    cdt.insertEdges(
        this->_currentGlyph.getEdges().begin(),
        this->_currentGlyph.getEdges().end(),
        [](const tr::edge_t &e) { return e.firstVertexIndex; },
        [](const tr::edge_t &e) { return e.secondVertexIndex; }
    );
    cdt.eraseOuterTrianglesAndHoles();

    // Create index buffer from triangulation
    CDT::TriangleVec cdtTriangles = cdt.triangles;
    for(int i = 0; i < cdtTriangles.size(); i++) {
        this->_currentGlyph.addIndex(cdtTriangles.at(i).vertices.at(0));
        this->_currentGlyph.addIndex(cdtTriangles.at(i).vertices.at(1));
        this->_currentGlyph.addIndex(cdtTriangles.at(i).vertices.at(2));
    }
}

/**
 * @brief Destroys old vertex and index buffers and creates new ones used for rendering
 */
void TextRenderer::_recreateBuffers() {
    // Destroy vulkan buffers
    this->_destroyBuffer(this->_indexBuffer, this->_indexBufferMemory);
    this->_destroyBuffer(this->_vertexBuffer, this->_vertexBufferMemory);

    // Create vulkan buffers
    // Always call _createVertexBuffer() before _createIndexBuffer()
    this->_createVertexBuffer();
    this->_createIndexBuffer();
}

/**
 * @brief Computes vertices of a quadratic bezier curve with specified level of detail
 *
 * @param startPoint Bezier curve starting point
 * @param controlPoint Bezier curve control point
 * @param endPoint Bezier curve ending point
 */
void TextRenderer::_detailBezier(tr::vertex_t startPoint, tr::vertex_t controlPoint, tr::vertex_t endPoint) {
    for(int i = 1; i <= LOD; i++) {
        double t = i * (1.0 / LOD);

        float x = (1 - t) * (1 - t) * startPoint.x + 2 * (1 - t) * t * controlPoint.x + (t * t) * endPoint.x;
        float y = (1 - t) * (1 - t) * startPoint.y + 2 * (1 - t) * t * controlPoint.y + (t * t) * endPoint.y;

        this->_currentGlyph.getVertices().push_back({ x, y });
        this->_currentGlyph.getEdges().push_back({ this->_vertexId, ++(this->_vertexId) });
    }
}

/**
 * @brief Selects the best possible memory to use for vertex buffer
 *
 * @param memoryType Required memory type
 * @param properties Required memory properties
 */
uint32_t TextRenderer::_selectMemoryType(uint32_t memoryType, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(this->_physicalDevice, &memoryProperties);

    for(uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if((memoryType & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Error selecting memory for vulkan vertex buffer");
}

/**
 * @brief Creates and allocates memory for vulkan buffer
 *
 * @param size Size of buffer
 * @param usage Usage of buffer
 * @param properties Memory properties
 * @param buffer Vulkan buffer
 * @param bufferMemory Vulkan buffer memory
 */
void TextRenderer::_createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateBuffer(this->_logicalDevice, &bufferCreateInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan buffer");
    }

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(this->_logicalDevice, buffer, &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo{};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = _selectMemoryType(memoryRequirements.memoryTypeBits, properties);

    if(vkAllocateMemory(this->_logicalDevice, &memoryAllocateInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Error allocating vulkan buffer memory");
    }

    vkBindBufferMemory(this->_logicalDevice, buffer, bufferMemory, 0);
}

/**
 * @brief Copies vulkan buffer to another vulkan buffer
 *
 * @param sourceBuffer Source vulkan buffer
 * @param destinationBuffer Destination vulkan buffer
 * @param bufferSize Vulkan buffer size
 */
void TextRenderer::_copyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize bufferSize) {
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandPool = this->_commandPool;
    commandBufferAllocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(this->_logicalDevice, &commandBufferAllocateInfo, &commandBuffer);

    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = bufferSize;
    vkCmdCopyBuffer(commandBuffer, sourceBuffer, destinationBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(this->_graphicsQueue, 1, &submitInfo, nullptr);
    vkQueueWaitIdle(this->_graphicsQueue);

    vkFreeCommandBuffers(this->_logicalDevice, this->_commandPool, 1, &commandBuffer);
}

/**
 * @brief Creates and allocates memory for vertex buffer
 */
void TextRenderer::_createVertexBuffer() {
    this->_vertices.clear();

    uint32_t vertexCount = 0;
    // Key: unicode code point, Value: vertex buffer offset
    std::unordered_map<uint32_t, uint32_t> usedCodePoints;
    for(Character &character : this->_characters) {
        if(!usedCodePoints.contains(character.getUnicodeCodePoint())) {
            usedCodePoints.insert({ character.getUnicodeCodePoint(), vertexCount });
            vertexCount += character.glyph.getVertexCount();

            this->_vertices.insert(this->_vertices.end(), character.glyph.getVertices().begin(), character.glyph.getVertices().end());
        }

        character.setVertexBufferOffset(usedCodePoints.at(character.getUnicodeCodePoint()));
    }

    if(vertexCount == 0) {
        return;
    }

    VkDeviceSize bufferSize = sizeof(this->_vertices.at(0)) * this->_vertices.size();

    // Staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    this->_createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(this->_logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, this->_vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(this->_logicalDevice, stagingBufferMemory);

    // Vertex buffer
    this->_createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->_vertexBuffer, this->_vertexBufferMemory);
    this->_copyBuffer(stagingBuffer, this->_vertexBuffer, bufferSize);

    vkDestroyBuffer(this->_logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(this->_logicalDevice, stagingBufferMemory, nullptr);
}

/**
 * @brief Creates and allocates memory for index buffer
 */
void TextRenderer::_createIndexBuffer() {
    this->_indices.clear();

    uint32_t indexCount = 0;
    // Key: unicode code point, Value: index buffer offset
    std::unordered_map<uint32_t, uint32_t> usedCodePoints;
    for(Character &character : this->_characters) {
        if(!usedCodePoints.contains(character.getUnicodeCodePoint())) {
            usedCodePoints.insert({ character.getUnicodeCodePoint(), indexCount });
            this->_indices.insert(this->_indices.end(), character.glyph.getIndices().begin(), character.glyph.getIndices().end());

            for(int i = indexCount; i < this->_indices.size(); i++) {
                this->_indices.at(i) += character.getVertexBufferOffset();
            }

            indexCount += character.glyph.getIndexCount();
        }

        character.setIndexBufferOffset(usedCodePoints.at(character.getUnicodeCodePoint()));
    }

    if(indexCount == 0) {
        return;
    }

    VkDeviceSize bufferSize = sizeof(this->_indices.at(0)) * this->_indices.size();

    // Staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    this->_createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(this->_logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, this->_indices.data(), (size_t) bufferSize);
    vkUnmapMemory(this->_logicalDevice, stagingBufferMemory);

    // Index buffer
    this->_createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->_indexBuffer, this->_indexBufferMemory);
    this->_copyBuffer(stagingBuffer, this->_indexBuffer, bufferSize);

    vkDestroyBuffer(this->_logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(this->_logicalDevice, stagingBufferMemory, nullptr);
}

/**
 * @brief Destroys and deallocates memory for vulkan buffer
 *
 * @param buffer Vulkan buffer
 * @param bufferMemory Vulkan buffer memory
 */
void TextRenderer::_destroyBuffer(VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
    if(buffer == nullptr)
        return;

    vkDeviceWaitIdle(this->_logicalDevice);

    vkDestroyBuffer(this->_logicalDevice, buffer, nullptr);
    vkFreeMemory(this->_logicalDevice, bufferMemory, nullptr);

    buffer = nullptr;
    bufferMemory = nullptr;
}

/**
 * @brief Initializes glyph metrics
 */
void TextRenderer::_initializeGlyphInfo() {
    this->_glyphInfo.clear();

    this->_glyphInfo.insert({ TextRenderer::U_ENTER, Glyph() });
    this->_glyphInfo.at(TextRenderer::U_ENTER).setAdvanceX(0);
    this->_glyphInfo.at(TextRenderer::U_ENTER).setAdvanceY(this->_fontSize);
}

/**
 * @brief Updates the window dimensions. If wrapping is used, rerenders text with respect to new window dimensions
 * 
 * @param windowWidth New window width
 * @param windowHeight New window height
 */
void TextRenderer::setWindowDimensions(unsigned int windowWidth, unsigned int windowHeight) {
    this->_windowWidth = windowWidth;
    this->_windowHeight = windowHeight;

    // Recalculate character positions if wrapping is enabled
    if(this->_useWrapping) {
        std::vector<uint32_t> codePoints;
        for(Character &character : this->_characters) {
            codePoints.push_back(character.getUnicodeCodePoint());
        }

        this->renderText(codePoints);
    }
}

/**
 * @brief Getter for text renderer
 *
 * @return Text renderer object
 */
TextRenderer &TextRenderer::getInstance() {
    static TextRenderer instance;
    return instance;
}

/**
 * @brief Get a vector of characters, which are to be rendered
 *
 * @return Vector of renderable characters
 */
std::vector<Character> TextRenderer::getCharacters() {
    return this->_characters;
}

/**
 * @brief Get vertex count in vertex buffer
 *
 * @return Vertex count
 */
uint32_t TextRenderer::getVertexCount() {
    return this->_vertices.size();
}

/**
 * @brief Get index count in index buffer
 *
 * @return Index count
 */
uint32_t TextRenderer::getIndexCount() {
    return this->_indices.size();
}

/**
 * @brief Getter for vulkan vertex buffer
 *
 * @return Vertex buffer
 */
VkBuffer TextRenderer::getVertexBuffer() {
    return this->_vertexBuffer;
}

/**
 * @brief Getter for vulkan index buffer
 *
 * @return Index buffer
 */
VkBuffer TextRenderer::getIndexBuffer() {
    return this->_indexBuffer;
}

/**
 * @brief Setter for vulkan physical device
 *
 * @param physicalDevice Vulkan physical device
 */
void TextRenderer::_setPhysicalDevice(VkPhysicalDevice physicalDevice) {
    if(physicalDevice == nullptr)
        throw std::runtime_error("Vulkan physical device is not initialized");

    this->_physicalDevice = physicalDevice;
}

/**
 * @brief Setter for vulkan logical device
 *
 * @param logicalDevice Vulkan logical device
 */
void TextRenderer::_setLogicalDevice(VkDevice logicalDevice) {
    if(logicalDevice == nullptr)
        throw std::runtime_error("Vulkan logical device is not initialized");

    this->_logicalDevice = logicalDevice;
}

/**
 * @brief Setter for vulkan command pool
 *
 * @param commandPool Vulkan command pool
 */
void TextRenderer::_setCommandPool(VkCommandPool commandPool) {
    if(commandPool == nullptr)
        throw std::runtime_error("Vulkan command pool is not initialized");

    this->_commandPool = commandPool;
}

/**
 * @brief Setter for vulkan graphics queue
 *
 * @param graphicsQueue Vulkan graphics queue
 */
void TextRenderer::_setGraphicsQueue(VkQueue graphicsQueue) {
    if(graphicsQueue == nullptr)
        throw std::runtime_error("Vulkan graphics queue is not initialized");

    this->_graphicsQueue = graphicsQueue;
}
