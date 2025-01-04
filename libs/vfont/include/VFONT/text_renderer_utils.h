/**
 * @file text_renderer_utils.h
 * @author Christian Saloň
 */

#pragma once

#include <cstdint>

#include <vulkan/vulkan.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

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
    uint32_t first;  /**< First vertex index */
    uint32_t second; /**< Second vertex index */

    Edge(uint32_t first, uint32_t second) : first{first}, second{second} {};

    bool operator==(const Edge &edge) const { return this->first == edge.first && this->second == edge.second; };
    inline bool isInverse(const Edge &edge) { return this->first == edge.second && this->second == edge.first; };
};

class Curve {
public:
    uint32_t start;
    uint32_t control;
    uint32_t end;

    Curve(uint32_t start, uint32_t control, uint32_t end) : start{start}, control{control}, end{end} {};

    bool operator==(const Curve &curve) const {
        return this->start == curve.start && this->control == curve.control && this->end == curve.end;
    };
};

/**
 * @brief Push constants used by Vulkan for rendering
 */
class CharacterPushConstants {
public:
    glm::mat4 model; /**< Model matrix of character */
    glm::vec4 color; /**< Color of character */

    CharacterPushConstants(glm::mat4 model, glm::vec4 color) : model{model}, color{color} {};
};

class ComposedGlyphData {
public:
    uint32_t vertexId;             /**< Currently processed glyph's vertex id counter */
    uint32_t contourStartVertexId; /**< Vertex id for the starting vertex of a contour */
    glm::vec2 lastVertex;          /**< Last vertex processed */
    int contourCount;              /**< Number of processed contours */

    ComposedGlyphData() : vertexId{0}, contourStartVertexId{0}, lastVertex{0, 0}, contourCount{0} {}
};

/**
 * @brief Uniform buffer object
 */
class UniformBufferObject {
public:
    glm::mat4 view;
    glm::mat4 projection;

    UniformBufferObject(glm::mat4 view, glm::mat4 projection) : view{view}, projection{projection} {}
    UniformBufferObject() : view{glm::mat4(1.f)}, projection{glm::mat4(1.f)} {}
};

class VulkanContext {
public:
    VkPhysicalDevice physicalDevice{nullptr};  /// Vulkan physical device
    VkDevice logicalDevice{nullptr};           /// Vulkan logical device
    VkQueue graphicsQueue{nullptr};            /// Vulkan graphics queue
    VkCommandPool commandPool{nullptr};        /// Vulkan command pool
    VkRenderPass renderPass{nullptr};          /// Vulkan render pass

    VulkanContext(VkPhysicalDevice physicalDevice,
                  VkDevice logicalDevice,
                  VkQueue graphicsQueue,
                  VkCommandPool commandPool,
                  VkRenderPass renderPass)
        : physicalDevice{physicalDevice},
          logicalDevice{logicalDevice},
          graphicsQueue{graphicsQueue},
          commandPool{commandPool},
          renderPass{renderPass} {}
    VulkanContext() {}
};

}  // namespace vft
