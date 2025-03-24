/**
 * @file vulkan_triangulation_text_renderer.h
 * @author Christian Saloň
 */

#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.h>
#include <glm/vec2.hpp>

#include "glyph_cache.h"
#include "triangulation_tessellator.h"
#include "vulkan_text_renderer.h"

namespace vft {

/**
 * @brief Basic implementation of vulkan text renderer where glyphs are tessellated and triangulized on the cpu
 */
class VulkanTriangulationTextRenderer : public VulkanTextRenderer {
protected:
    /** Hash map containing glyph offsets into the index buffer (key: glyph key, value: offset into the index buffer) */
    std::unordered_map<GlyphKey, uint32_t, GlyphKeyHash> _offsets{};

    std::vector<glm::vec2> _vertices{}; /**< Vertex buffer */
    std::vector<uint32_t> _indices{};   /**< Index buffer */

    VkBuffer _vertexBuffer{nullptr};             /**< Vulkan vertex buffer */
    VkDeviceMemory _vertexBufferMemory{nullptr}; /**< Vulkan vertex buffer memory */
    VkBuffer _indexBuffer{nullptr};              /**< Vulkan index buffer for glyph's triangles */
    VkDeviceMemory _indexBufferMemory{nullptr};  /**< Vulkan index buffer memory for glyph's triangles */

    VkPipelineLayout _pipelineLayout{nullptr}; /**< Vulkan pipeline layout for glpyh's triangles */
    VkPipeline _pipeline{nullptr};             /**< Vulkan pipeline for glpyh's triangles */

public:
    VulkanTriangulationTextRenderer() = default;
    ~VulkanTriangulationTextRenderer() = default;

    void initialize() override;
    void destroy() override;
    void draw() override;
    void update() override;

protected:
    void _createPipeline();
    void _createVertexAndIndexBuffers();
};

}  // namespace vft
