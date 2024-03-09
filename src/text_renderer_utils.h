/**
 * @file text_renderer_utils.h
 * @author Christian Saloň
 */

#pragma once

#include <vulkan/vulkan.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

namespace tr {

static VkVertexInputBindingDescription getVertexInutBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(glm::vec2);
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

/**
 * @brief Represents an edge between two vertices
 */
typedef struct edge {
    uint32_t first;     /**< First vertex index */
    uint32_t second;    /**< Second vertex index */
} edge_t;

/**
 * @brief Push constants used by Vulkan for rendering
 */
typedef struct character_push_constants {
    glm::mat4 model;
} character_push_constants_t;

}
