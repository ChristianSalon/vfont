/**
 * @file text_renderer_utils.h
 * @author Christian Saloň
 */

#pragma once

#include <cstdint>

#include <vulkan/vulkan.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

#include "glyph.h"

namespace vft {

const uint32_t U_BACKSPACE = 0x00000008;
const uint32_t U_ENTER = 0x0000000d;
const uint32_t U_SPACE = 0x00000020;
const uint32_t U_TAB = 0x00000009;

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
class Edge {

public:

    uint32_t first;     /**< First vertex index */
    uint32_t second;    /**< Second vertex index */

    Edge(uint32_t first, uint32_t second) : first{first}, second{second} {};

    inline bool operator==(const Edge &edge) { return this->first == edge.first && this->second == edge.second; };
    inline bool isInverse(const Edge &edge) { return this->first == edge.second && this->second == edge.first; };

};

/**
 * @brief Push constants used by Vulkan for rendering
 */
class CharacterPushConstants {

public:

    glm::mat4 model;    /**< Model matrix of character */
    glm::vec3 color;    /**< Color of character */

    CharacterPushConstants(glm::mat4 model, glm::vec3 color) : model{model}, color{color} {};

};

class ComposedGlyphData {

public:
    
    uint32_t vertexId;              /**< Currently processed glyph's vertex id counter */
    uint32_t contourStartVertexId;  /**< Vertex id for the starting vertex of a contour */
    glm::vec2 lastVertex;           /**< Last vertex processed */
    int contourCount;               /**< Number of processed contours */

    ComposedGlyphData() :
        vertexId{0},
        contourStartVertexId{0},
        lastVertex{0,0},
        contourCount{0} {}

};

}
