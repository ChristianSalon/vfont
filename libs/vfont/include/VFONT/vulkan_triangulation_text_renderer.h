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

#include "cpu_tessellator.h"
#include "glyph_cache.h"
#include "vulkan_text_renderer.h"

namespace vft {

class VulkanTriangulationTextRenderer : public VulkanTextRenderer {
protected:
    std::unordered_map<GlyphKey, uint32_t, GlyphKeyHash> _offsets;

    std::vector<glm::vec2> _vertices; /**< Vertex buffer */
    std::vector<uint32_t> _indices;   /**< Index buffer */

    VkBuffer _vertexBuffer{nullptr};             /**< Vulkan vertex buffer */
    VkDeviceMemory _vertexBufferMemory{nullptr}; /**< Vulkan vertex buffer memory */
    VkBuffer _indexBuffer{nullptr};              /**< Vulkan index buffer */
    VkDeviceMemory _indexBufferMemory{nullptr};  /**< Vulkan index buffer memory */

    VkPipelineLayout _pipelineLayout{nullptr};
    VkPipeline _pipeline{nullptr};

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
