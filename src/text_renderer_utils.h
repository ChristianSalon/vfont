/**
 * @file text_renderer_utils.h
 * @author Christian Saloň
 */

#pragma once

#include <vulkan/vulkan.h>

namespace tr {

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
    uint32_t firstVertexIndex;  /**< First vertex index */
    uint32_t secondVertexIndex; /**< Second vertex index */
} edge_t;

/**
 * @brief Push constants used by Vulkan for rendering
 */
typedef struct character_push_constants {
    int x;              /**< X coordinate of character */
    int y;              /**< Y coordinate of character */
    int windowWidth;    /**< Target window's width */
    int windowHeight;   /**< Target window's height */
} character_push_constants_t;

}
