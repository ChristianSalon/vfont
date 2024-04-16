/**
 * @file text_renderer.cpp
 * @author Christian Saloň
 */

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cmath>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include "CDT.h"
#include <glm/ext/matrix_transform.hpp>

#include "text_renderer.h"
#include "text_renderer_utils.h"

namespace vft {

const uint32_t TextRenderer::U_BACKSPACE = 0x00000008;
const uint32_t TextRenderer::U_ENTER = 0x0000000d;
const uint32_t TextRenderer::U_SPACE = 0x00000020;
const uint32_t TextRenderer::U_TAB = 0x00000009;

const unsigned int TextRenderer::DEFAULT_FONT_SIZE = 64;

/**
 * @brief Text renderer constructor
 */
TextRenderer::TextRenderer() {
    this->_vertices = {};
    this->_indices = {};

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
        pThis->_currentGlyph.updateEdge(pThis->_currentGlyph.getEdgeCount() - 1, { pThis->_currentGlyph.getEdges().at(pThis->_currentGlyph.getEdgeCount() - 1).first, pThis->_contourStartVertexId });
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
    pThis->_currentGlyph.addVertex(pThis->_lastVertex);
    pThis->_currentGlyph.addEdge({ pThis->_vertexId, ++(pThis->_vertexId) });

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

    glm::vec2 controlPoint = { static_cast<float>(control->x), static_cast<float>(control->y) };
    glm::vec2 endPoint = { static_cast<float>(to->x), static_cast<float>(to->y) };

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
 * @param fontFilePath File path to .ttf file
 * @param viewportWidth Target window width
 * @param viewportHeight Target window height
 * @param physicalDevice Vulkan physical device
 * @param logicalDevice Vulkan logical device
 * @param commandPool Vulkan command pool
 * @param graphicsQueue Vulkan graphics queue
 * @param pipelineLayout Vulkan pipeline layout
 * @param useKerning Indicates whether to use kerning
 * @param useWrapping Indicates whether to use wrapping
 */
void TextRenderer::init(
    VkPhysicalDevice physicalDevice,
    VkDevice logicalDevice,
    VkCommandPool commandPool,
    VkQueue graphicsQueue,
    VkPipelineLayout pipelineLayout
) {
    this->_setPhysicalDevice(physicalDevice);
    this->_setLogicalDevice(logicalDevice);
    this->_setCommandPool(commandPool);
    this->_setGraphicsQueue(graphicsQueue);
    this->_setPipelineLayout(pipelineLayout);
}

/**
 * @brief Destroys freetype and vulkan buffers
 */
void TextRenderer::destroy() {
    // Destroy vulkan buffers
    this->_destroyBuffer(this->_indexBuffer, this->_indexBufferMemory);
    this->_destroyBuffer(this->_vertexBuffer, this->_vertexBufferMemory);
}

/**
 * @brief Add vulkan draw commands to selected command buffer
 *
 * @param commandBuffer Selected vulkan command buffer
 */
void TextRenderer::draw(VkCommandBuffer commandBuffer) {
    if(this->getVertexCount() > 0) {
        VkBuffer vertexBuffers[] = { this->_vertexBuffer };
        VkDeviceSize offsets[] = { 0 };

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, this->_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    }

    for(int i = 0; i < this->_blocks.size(); i++) {
        for(Character &character : this->_blocks[i]->getCharacters()) {
            if(character.glyph.getVertexCount() > 0) {
                character_push_constants_t pushConstants = {
                    .model = character.getModelMatrix(),
                    .color = this->_blocks[i]->getColor()
                };
                vkCmdPushConstants(commandBuffer, this->_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(character_push_constants_t), &pushConstants);
                vkCmdDrawIndexed(commandBuffer, character.glyph.getIndexCount(), 1, character.getIndexBufferOffset(), 0, 0);
            }
        }
    }
}

void TextRenderer::add(std::shared_ptr<TextBlock> text) {
    this->_blocks.push_back(text);
    this->recreateBuffers();
}

void TextRenderer::renderGlyph(uint32_t codePoint, std::shared_ptr<Font> font) {
    this->_vertexId = 0;
    this->_contourStartVertexId = 0;
    this->_lastVertex = { 0, 0 };
    this->_isFirstContour = true;

    // If code point is TAB, then it is rendered as four SPACE characters
    uint32_t newCodePoint = codePoint == TextRenderer::U_TAB ? TextRenderer::U_SPACE : codePoint;

    if(!font.get()->getAllGlyphInfo().contains(newCodePoint)) {
        this->_decomposeGlyph(newCodePoint, font);
        this->_triangulate();

        // Insert glyph info to map
        font.get()->setGlyphInfo(newCodePoint, this->_currentGlyph);
    }
}

/**
 * @brief Decomposes a glyph into a set of points and edges
 *
 * @param codePoint Unicode code point of glyph
 */
void TextRenderer::_decomposeGlyph(uint32_t codePoint, std::shared_ptr<Font> font) {
    // Get glyph from .ttf file
    if(FT_Load_Char(font.get()->getFace(), codePoint, FT_LOAD_DEFAULT)) {
        throw std::runtime_error("Error loading glyph");
    }
    FT_GlyphSlot slot = font.get()->getFace()->glyph;

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
            { this->_currentGlyph.getEdges().back().first, this->_contourStartVertexId }
        );
    }

    this->_currentGlyph.setAdvanceX(slot->advance.x >> 6);
    this->_currentGlyph.setAdvanceY(slot->advance.y >> 6);
}

/**
 * @brief Performs constrained delaunay triangulation on a decomposed glyph
 */
void TextRenderer::_triangulate() {
    std::vector<glm::vec2> vertices = this->_currentGlyph.getVertices();
    std::vector<vft::edge_t> edges = this->_currentGlyph.getEdges();

    //this->_checkIntersectingEdges(vertices, edges);
    CDT::RemoveDuplicatesAndRemapEdges<float>(
        vertices,
        [](const glm::vec2 &p) { return p.x; },
        [](const glm::vec2 &p) { return p.y; },
        edges.begin(),
        edges.end(),
        [](const vft::edge_t &e) { return e.first; },
        [](const vft::edge_t &e) { return e.second; },
        [](uint32_t i1, uint32_t i2) -> vft::edge_t { return vft::edge_t{i1, i2}; });

    this->_currentGlyph.setVertices(vertices);
    this->_currentGlyph.setEdges(edges);

    // CDT uses Constrained Delaunay Triangulation algorithm
    CDT::Triangulation<float> cdt{
        CDT::VertexInsertionOrder::Auto,
        CDT::IntersectingConstraintEdges::TryResolve,
        1.0e-6};

    cdt.insertVertices(
        this->_currentGlyph.getVertices().begin(),
        this->_currentGlyph.getVertices().end(),
        [](const glm::vec2 &p) { return p.x; },
        [](const glm::vec2 &p) { return p.y; });
    cdt.insertEdges(
        this->_currentGlyph.getEdges().begin(),
        this->_currentGlyph.getEdges().end(),
        [](const vft::edge_t &e) { return e.first; },
        [](const vft::edge_t &e) { return e.second; });
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
void TextRenderer::recreateBuffers() {
    // Destroy vulkan buffers
    this->_destroyBuffer(this->_indexBuffer, this->_indexBufferMemory);
    this->_destroyBuffer(this->_vertexBuffer, this->_vertexBufferMemory);

    // Create vulkan buffers
    // Always call _createVertexBuffer() before _createIndexBuffer()
    this->_createVertexBuffer();
    this->_createIndexBuffer();
}

/**
 * @brief Computes vertices of a quadratic bezier curve with adaptive level of detail
 *
 * @param startPoint Bezier curve starting point
 * @param controlPoint Bezier curve control point
 * @param endPoint Bezier curve ending point
 */
void TextRenderer::_detailBezier(glm::vec2 startPoint, glm::vec2 controlPoint, glm::vec2 endPoint) {
    std::map<float, glm::vec2> vertices{ { 1.f, glm::vec2(endPoint) } };
    this->_subdivide(startPoint, controlPoint, endPoint, 0.5f, 0.5f, vertices);

    for(std::map<float, glm::vec2>::iterator i = vertices.begin(); i != vertices.end(); i++) {
        this->_currentGlyph.addVertex(i->second);
        this->_currentGlyph.addEdge({ this->_vertexId, ++(this->_vertexId) });
    }
}

/**
 * @brief Subdivides a quadratic bezier curve until there is no loss of quality
 *
 * @param startPoint Bezier curve starting point
 * @param controlPoint Bezier curve control point
 * @param endPoint Bezier curve ending point
 * @param t Parameter for the position on the bezier curve (t = <0, 1>)
 * @param delta Indicates distance between current point and computed points in the previous step
 * @param vertices Created vertices by dividing bezier curve into line segments
 */
void TextRenderer::_subdivide(glm::vec2 startPoint, glm::vec2 controlPoint, glm::vec2 endPoint, float t, float delta, std::map<float, glm::vec2> &vertices) {
    // Add current point
    glm::vec2 newVertex{ (1 - t) * (1 - t) * startPoint + 2 * (1 - t) * t * controlPoint + (t * t) * endPoint };
    vertices.insert({ t, newVertex });

    // Compute points around the current point with the given delta
    glm::vec2 left{ (1 - (t - delta)) * (1 - (t - delta)) * startPoint + 2 * (1 - (t - delta)) * (t - delta) * controlPoint + (t - delta) * (t - delta) * endPoint };
    glm::vec2 right{ (1 - (t + delta)) * (1 - (t + delta)) * startPoint + 2 * (1 - (t + delta)) * (t + delta) * controlPoint + (t + delta) * (t + delta) * endPoint };

    // Check if distance between curve(t - delta) and curve(t) is less than one pixel
    // or if points can be connected by a line without losing quality
    if(glm::length(newVertex - left) >= 64.f && newVertex.x != left.x && newVertex.y != left.y) {
        // The segment curve(t - delta) and curve(t) should be subdivided
        this->_subdivide(startPoint, controlPoint, endPoint, t - delta / 2.f, delta / 2.f, vertices);
    }

    // Check if distance between curve(t) and curve(t + delta) is less than one pixel
    // or if points can be connected by a line without losing quality
    if(glm::length(newVertex - right) >= 64.f && newVertex.x != right.x && newVertex.y != right.y) {
        // The segment curve(t) and curve(t + delta) should be subdivided
        this->_subdivide(startPoint, controlPoint, endPoint, t + delta / 2.f, delta / 2.f, vertices);
    }
}

double TextRenderer::_determinant(double a, double b, double c, double d) {
    return (a * d) - (b * c);
}

bool TextRenderer::_intersect(const std::vector<glm::vec2> &vertices, vft::edge_t first, vft::edge_t second, glm::vec2 &intersection) {
    static double epsilon = 1e-6;

    double x1 = vertices.at(first.first).x;
    double y1 = vertices.at(first.first).y;
    double x2 = vertices.at(first.second).x;
    double y2 = vertices.at(first.second).y;
    double x3 = vertices.at(second.first).x;
    double y3 = vertices.at(second.first).y;
    double x4 = vertices.at(second.second).x;
    double y4 = vertices.at(second.second).y;

    double det1 = this->_determinant(x1 - x2, y1 - y2, x3 - x4, y3 - y4);
    double det2 = this->_determinant(x1 - x3, y1 - y3, x3 - x4, y3 - y4);

    // Check if edges are parallel
    if(fabs(det1) < epsilon || fabs(det2) < epsilon) {
        return false;
    }

    intersection.x = this->_determinant(this->_determinant(x1, y1, x2, y2), x1 - x2, this->_determinant(x3, y3, x4, y4), x3 - x4) / det1;
    intersection.y = this->_determinant(this->_determinant(x1, y1, x2, y2), y1 - y2, this->_determinant(x3, y3, x4, y4), y3 - y4) / det1;

    return this->_isPointOnLineSegment(x1, y1, x2, y2, intersection.x, intersection.y) &&
           this->_isPointOnLineSegment(x3, y3, x4, y4, intersection.x, intersection.y);
}

bool TextRenderer::_isPointOnLineSegment(double x1, double y1, double x2, double y2, double x, double y) {
    static double epsilon = 1e-6;

    return fabs(this->_determinant(x - x1, y - y1, x2 - x1, y2 - y1)) < epsilon && (x - x1) * (x - x2) + (y - y1) * (y - y2) <= 0;
}

void TextRenderer::_checkIntersectingEdges(std::vector<glm::vec2> &vertices, std::vector<vft::edge_t> &edges) {
    // Remove duplicate edges
    uint32_t deletedVerticesCount = 0;
    for(uint32_t i = 0; i < vertices.size(); i++) {
        for(uint32_t j = i + 1; j < vertices.size(); j++) {
            if(fabs(vertices.at(i).x - vertices.at(j).x) <= 1.0e-6 && fabs(vertices.at(i).y - vertices.at(j).y) <= 1.0e-6) {
                std::for_each(edges.begin(), edges.end(), [&](vft::edge_t &edge) {
                    if(edge.first == j) {
                        edge.first = i;
                    }
                    if(edge.second == j) {
                        edge.second = i;
                    }
                });
            }
        }
    }

    // Resolve intersecting edges
    for(int i = 0; i < edges.size(); i++) {
        for(int j = i + 1; j < edges.size(); j++) {
            vft::edge_t &firstEdge = edges.at(i);
            vft::edge_t &secondEdge = edges.at(j);

            if(
                firstEdge == secondEdge ||
                firstEdge.second == secondEdge.first ||
                firstEdge.first == secondEdge.second ||
                firstEdge.first == secondEdge.first ||
                firstEdge.second == secondEdge.second) {
                continue;
            }

            glm::vec2 intersection = glm::vec2(0.f, 0.f);
            bool intersect = this->_intersect(vertices, firstEdge, secondEdge, intersection);
            if(intersect) {
                if(intersection == vertices.at(firstEdge.first)) {
                    uint32_t oldSecondEdgeSecondIndex = secondEdge.second;
                    secondEdge.second = firstEdge.first;
                    auto secondEdgeIterator = std::find(edges.begin(), edges.end(), secondEdge);
                    edges.insert(secondEdgeIterator + 1, {firstEdge.first, oldSecondEdgeSecondIndex});
                }
                else if(intersection == vertices.at(firstEdge.second)) {
                    uint32_t oldSecondEdgeSecondIndex = secondEdge.second;
                    secondEdge.second = firstEdge.second;
                    auto secondEdgeIterator = std::find(edges.begin(), edges.end(), secondEdge);
                    edges.insert(secondEdgeIterator + 1, {firstEdge.second, oldSecondEdgeSecondIndex});
                }
                else if(intersection == vertices.at(secondEdge.first)) {
                    uint32_t oldFirstEdgeSecondIndex = firstEdge.second;
                    firstEdge.second = secondEdge.first;
                    auto firstEdgeIterator = std::find(edges.begin(), edges.end(), firstEdge);
                    edges.insert(firstEdgeIterator + 1, {secondEdge.first, oldFirstEdgeSecondIndex});
                }
                else if(intersection == vertices.at(secondEdge.second)) {
                    uint32_t oldFirstEdgeSecondIndex = firstEdge.second;
                    firstEdge.second = secondEdge.second;
                    auto firstEdgeIterator = std::find(edges.begin(), edges.end(), firstEdge);
                    edges.insert(firstEdgeIterator + 1, {secondEdge.second, oldFirstEdgeSecondIndex});
                }
                else {
                    uint32_t intersectionIndex = vertices.size();
                    vertices.push_back(intersection);

                    uint32_t oldFirstEdgeSecondIndex = firstEdge.second;
                    uint32_t oldSecondEdgeSecondIndex = secondEdge.second;
                    firstEdge.second = intersectionIndex;
                    secondEdge.second = intersectionIndex;

                    auto firstEdgeIterator = std::find(edges.begin(), edges.end(), firstEdge);
                    edges.insert(firstEdgeIterator + 1, {intersectionIndex, oldFirstEdgeSecondIndex});

                    secondEdge = edges.at(j + 1);
                    auto secondEdgeIterator = std::find(edges.begin(), edges.end(), secondEdge);
                    edges.insert(secondEdgeIterator + 1, {intersectionIndex, oldSecondEdgeSecondIndex});

                    j++;
                }

                j++;
            }
        }
    }
}

/**
 * @brief Selects the best possible memory to use for vertex buffer
 *
 * @param memoryType Required memory type
 * @param properties Required memory properties
 *
 * @return Index of selected memory
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
    for(int i = 0; i < this->_blocks.size(); i++) {
        for(Character &character : this->_blocks[i]->getCharacters()) {
            if(!usedCodePoints.contains(character.getUnicodeCodePoint())) {
                usedCodePoints.insert({ character.getUnicodeCodePoint(), vertexCount });
                vertexCount += character.glyph.getVertexCount();

                this->_vertices.insert(this->_vertices.end(), character.glyph.getVertices().begin(), character.glyph.getVertices().end());
            }

            character.setVertexBufferOffset(usedCodePoints.at(character.getUnicodeCodePoint()));
        }

        usedCodePoints.clear();
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
    for(int i = 0; i < this->_blocks.size(); i++) {
        for(Character &character : this->_blocks[i]->getCharacters()) {
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

        usedCodePoints.clear();
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
 * @brief Getter for text renderer
 *
 * @return Text renderer object
 */
TextRenderer &TextRenderer::getInstance() {
    static TextRenderer instance;
    return instance;
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

/**
 * @brief Setter for vulkan pipeline layout
 *
 * @param pipelineLayout Vulkan pipeline layout
 */
void TextRenderer::_setPipelineLayout(VkPipelineLayout pipelineLayout) {
    if(pipelineLayout == nullptr)
        throw std::runtime_error("Vulkan pipeline layout is not initialized");

    this->_pipelineLayout = pipelineLayout;
}

}
