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
#include "window.h"

TextRenderer::vertex_t lastVertex = { 0, 0 };
uint16_t vertexId = 0;
uint16_t startVertexId = 0;

/**
 * @brief Text renderer constructor
 */
TextRenderer::TextRenderer() {
    this->_physicalDevice = nullptr;
    this->_logicalDevice = nullptr;
    this->_vertexBuffer = nullptr;
    this->_vertexBufferMemory = nullptr;
    this->_indexBuffer = nullptr;
    this->_indexBufferMemory = nullptr;
    this->_vertices = {};
    this->_edges = {};
    this->_indices = {};
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
    
    lastVertex = pThis->_normalizeVertex(to->x, to->y);

    if(vertexId != 0) {
        // Makes the contour closed
        pThis->_edges.at(pThis->_edges.size() - 1).secondVertexIndex = startVertexId;
    }
    startVertexId = vertexId;

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

    lastVertex = pThis->_normalizeVertex(to->x, to->y);
    pThis->_vertices.push_back(lastVertex);
    pThis->_edges.push_back({ vertexId, ++vertexId });

    return 0;
};

/**
 * @brief Freetype outline decomposition conic_to function.
 * Used when there is a quadratic bezier curve.
 * 
 * @param control Bezier curve control point
 * @param to  Bezier curve ending point
 * @param user User defined data
 * 
 * @return Exit code
 */
FT_Outline_ConicToFunc TextRenderer::_conicToFunc = [](const FT_Vector *control, const FT_Vector *to, void *user) {
    TextRenderer *pThis = reinterpret_cast<TextRenderer *>(user);

    vertex_t toNormalized = pThis->_normalizeVertex(to->x, to->y);
    vertex_t controlNormalized = pThis->_normalizeVertex(control->x, control->y);
    
    pThis->_detailBezier(
        lastVertex,
        controlNormalized,
        toNormalized
    );

    lastVertex = toNormalized;

    return 0;
};

/**
 * @brief Freetype outline decomposition cubic_to function.
 * Used when there is a cubic bezier curve.
 * 
 * @param control1 First bezier curve control point
 * @param control2 Second bezier curve control point
 * @param to  Bezier curve ending point
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
 * @param fontFilePath File path to .ttf file
 * @param window Window where to render
 * @param physicalDevice Vulkan physical device
 * @param logicalDevice Vulkan logical device
 * @param commandPool Vulkan command pool
 * @param graphicsQueue Vulkan graphics queue
 */
void TextRenderer::init(std::string fontFilePath, std::shared_ptr<MainWindow> window, VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue) {
    this->_setWindow(window);
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

    FT_Set_Pixel_Sizes(this->_face, 256, 0);
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
void TextRenderer::renderGlyph(uint32_t codePoint) {
    this->_indices.clear();
    this->_vertices.clear();
    this->_edges.clear();
    lastVertex = { 0, 0 };
    vertexId = 0;
    startVertexId = 0;
    
    // Get glyph from .ttf file
    FT_UInt glyph_index = FT_Get_Char_Index(this->_face, codePoint);
    if(FT_Load_Glyph(this->_face, glyph_index, FT_LOAD_DEFAULT)) {
        throw std::runtime_error("Error loading glyph");
    }

    FT_GlyphSlot slot = this->_face->glyph;
    if(slot->outline.n_points == 0)
        return;

    for (int i = 0; i < slot->outline.n_points; i++) {
        float scale = 64;
        slot->outline.points[i].x /= scale;
        slot->outline.points[i].y /= scale;
    }

    // Decompose outlines to vertices and vertex indices
    FT_Outline_Funcs outlineFunctions = {
        .move_to = this->_moveToFunc,
        .line_to = this->_lineToFunc,
        .conic_to = this->_conicToFunc,
        .cubic_to = this->_cubicToFunc,
        .shift = 0,
        .delta = 0
    };
    FT_Outline_Decompose(&(slot->outline), &outlineFunctions, this);
    this->_edges.at(this->_edges.size() - 1).secondVertexIndex = startVertexId;

    // Reposition vertices to center of window
    for (int i = 0; i < this->_vertices.size(); i++) {
        this->_vertices.at(i).x += 1 - (this->_face->glyph->metrics.width / 64.f / this->_window.get()->getWidth()) - (this->_face->glyph->metrics.horiBearingX / 64.f / this->_window.get()->getWidth());
        this->_vertices.at(i).y -= 1 - (this->_face->glyph->metrics.height / 64.f / this->_window.get()->getHeight());
    }

    // Triangulate glyph vertices
    // CDT uses Constrained Delaunay Triangulation algorithm
    CDT::Triangulation<float> cdt;
    cdt.insertVertices(
        this->_vertices.begin(),
        this->_vertices.end(),
        [](const vertex_t &p) { return p.x; },
        [](const vertex_t &p) { return p.y; }
    );
    cdt.insertEdges(
        this->_edges.begin(),
        this->_edges.end(),
        [](const edge_t &e) { return e.firstVertexIndex; },
        [](const edge_t &e) { return e.secondVertexIndex; }
    );
    cdt.eraseOuterTrianglesAndHoles();

    // Create index buffer from triangulation
    CDT::TriangleVec cdtTriangles = cdt.triangles;
    for(int i = 0; i < cdtTriangles.size(); i++) {
        this->_indices.push_back(cdtTriangles.at(i).vertices.at(0));
        this->_indices.push_back(cdtTriangles.at(i).vertices.at(1));
        this->_indices.push_back(cdtTriangles.at(i).vertices.at(2));
    }
    
    // Create vulkan buffers
    this->_destroyBuffer(this->_indexBuffer, this->_indexBufferMemory);
    this->_destroyBuffer(this->_vertexBuffer, this->_vertexBufferMemory);
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
void TextRenderer::_detailBezier(vertex_t startPoint, vertex_t controlPoint, vertex_t endPoint) {
    for(int i = 1; i <= LOD; i++) {
        double t = i * (1.0 / LOD);

        float x = (1 - t) * (1 - t) * startPoint.x + 2 * (1 - t) * t * controlPoint.x + (t * t) * endPoint.x;
        float y = (1 - t) * (1 - t) * startPoint.y + 2 * (1 - t) * t * controlPoint.y + (t * t) * endPoint.y;

        this->_vertices.push_back({x, y});
        this->_edges.push_back({ vertexId, ++vertexId });
    }
}

/**
 * @brief Normalizes vertex to vulkan NDC
 * 
 * @param x X coordinate of vertex
 * @param y Y coordinate of vertex
 * 
 * @return Normalized vertex
 */
TextRenderer::vertex_t TextRenderer::_normalizeVertex(float x, float y) {
    x = 2.0f * (static_cast<float>(x) / this->_window.get()->getWidth()) - 1.0f;
    y = 1.0f - 2.0f * (static_cast<float>(y) / this->_window.get()->getHeight());

    return { x, y };
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

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
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
 * @brief Setter for window object
 * 
 * @param window Window object
 */
void TextRenderer::_setWindow(std::shared_ptr<MainWindow> window) {
    if(window == nullptr)
        throw std::runtime_error("Window is not initialized");

    this->_window = window;
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
