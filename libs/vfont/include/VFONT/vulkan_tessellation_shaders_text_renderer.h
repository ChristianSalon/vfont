/**
 * @file vulkan_tessellation_shaders_text_renderer.h
 * @author Christian Saloň
 */

#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.h>

#include "combined_tessellator.h"
#include "glyph_cache.h"
#include "vulkan_text_renderer.h"

namespace vft {

class VulkanTessellationShadersTextRenderer : public VulkanTextRenderer {
public:
    static constexpr unsigned int LINE_OFFSET_BUFFER_INDEX = 0;
    static constexpr unsigned int CURVE_OFFSET_BUFFER_INDEX = 1;

    struct ViewportPushConstants {
        uint32_t viewportWidth;
        uint32_t viewportHeight;
    };

protected:
    std::unordered_map<GlyphKey, std::array<uint32_t, 2>, GlyphKeyHash> _offsets{};

    std::vector<glm::vec2> _vertices{};            /**< Vertex buffer */
    std::vector<uint32_t> _lineSegmentsIndices{};  /**< Index buffer */
    std::vector<uint32_t> _curveSegmentsIndices{}; /**< Index buffer */

    unsigned int _viewportWidth{0};
    unsigned int _viewportHeight{0};

    VkBuffer _vertexBuffer{nullptr};                         /**< Vulkan vertex buffer */
    VkDeviceMemory _vertexBufferMemory{nullptr};             /**< Vulkan vertex buffer memory */
    VkBuffer _lineSegmentsIndexBuffer{nullptr};              /**< Vulkan index buffer */
    VkDeviceMemory _lineSegmentsIndexBufferMemory{nullptr};  /**< Vulkan index buffer memory */
    VkBuffer _curveSegmentsIndexBuffer{nullptr};             /**< Vulkan index buffer */
    VkDeviceMemory _curveSegmentsIndexBufferMemory{nullptr}; /**< Vulkan index buffer memory */

    VkPipelineLayout _lineSegmentsPipelineLayout{nullptr};
    VkPipeline _lineSegmentsPipeline{nullptr};
    VkPipelineLayout _curveSegmentsPipelineLayout{nullptr};
    VkPipeline _curveSegmentsPipeline{nullptr};

public:
    VulkanTessellationShadersTextRenderer() = default;
    ~VulkanTessellationShadersTextRenderer() = default;

    void initialize() override;
    void destroy() override;
    void draw() override;
    void update() override;

protected:
    void _createVertexAndIndexBuffers();

    void _createLineSegmentsPipeline();
    void _createCurveSegmentsPipeline();
};

}  // namespace vft
