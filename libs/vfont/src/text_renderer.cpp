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

/**
 * @brief TextRenderer constructor
 */
TextRenderer::TextRenderer() {
    this->_physicalDevice = nullptr;
    this->_logicalDevice = nullptr;
    this->_commandPool = nullptr;
    this->_graphicsQueue = nullptr;
    this->_pipelineLayout = nullptr;
    this->_vertexBuffer = nullptr;
    this->_vertexBufferMemory = nullptr;
    this->_indexBuffer = nullptr;
    this->_indexBufferMemory = nullptr;
}

/**
 * @brief Text renderer constructor
 * 
 * @param physicalDevice Vulkan physical device
 * @param logicalDevice Vulkan logical device
 * @param commandPool Vulkan command pool
 * @param graphicsQueue Vulkan graphics queue
 * @param pipelineLayout Vulkan pipeline layout
 */
TextRenderer::TextRenderer(
    VkPhysicalDevice physicalDevice,
    VkDevice logicalDevice,
    VkCommandPool commandPool,
    VkQueue graphicsQueue,
    VkPipelineLayout pipelineLayout
) {
    this->init(physicalDevice, logicalDevice, commandPool, graphicsQueue, pipelineLayout);
}

/**
 * @brief Text renderer destructor
 */
TextRenderer::~TextRenderer() {
    this->destroy();
}

/**
 * @brief Initializes text renderer
 *
 * @param physicalDevice Vulkan physical device
 * @param logicalDevice Vulkan logical device
 * @param commandPool Vulkan command pool
 * @param graphicsQueue Vulkan graphics queue
 * @param pipelineLayout Vulkan pipeline layout
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
 * @brief Must be called to destroy vulkan buffers
 */
void TextRenderer::destroy() {
    // Destroy vulkan buffers
    if(this->_indexBuffer != nullptr)
        this->_destroyBuffer(this->_indexBuffer, this->_indexBufferMemory);

    if(this->_vertexBuffer != nullptr)
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
                vft::CharacterPushConstants pushConstants{character.getModelMatrix(), this->_blocks[i]->getColor()};
                vkCmdPushConstants(commandBuffer, this->_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vft::CharacterPushConstants), &pushConstants);
                vkCmdDrawIndexed(commandBuffer, character.glyph.getIndexCount(), 1, character.getIndexBufferOffset(), 0, 0);
            }
        }
    }
}

/**
 * @brief Add text block for rendering
 * 
 * @param text Text block to render
 */
void TextRenderer::add(std::shared_ptr<TextBlock> text) {
    this->_blocks.push_back(text);
    text->onTextChange = [this]() {
        this->recreateBuffers();
    };

    this->recreateBuffers();
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
