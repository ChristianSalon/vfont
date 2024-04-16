/**
 * @file text_renderer_utils.h
 * @author Christian Saloň
 */

#pragma once

#include <vulkan/vulkan.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

namespace vft {

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
class edge_t {

public:

    uint32_t first;     /**< First vertex index */
    uint32_t second;    /**< Second vertex index */

    inline edge_t(uint32_t first, uint32_t second) : first{first}, second{second} {};

    inline bool operator==(const edge_t &edge) { return this->first == edge.first && this->second == edge.second; };

};

/**
 * @brief Push constants used by Vulkan for rendering
 */
typedef struct character_push_constants {
    glm::mat4 model;
    glm::vec3 color;
} character_push_constants_t;

}
