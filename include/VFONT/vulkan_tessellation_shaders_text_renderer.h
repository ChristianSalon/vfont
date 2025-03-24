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

#include "glyph_cache.h"
#include "tessellation_shaders_tessellator.h"
#include "vulkan_text_renderer.h"

namespace vft {

/**
 * @brief Basic implementation of vulkan text renderer where outer triangles are tessellated using shaders and inner
 * triangles are triangulized on the cpu
 */
class VulkanTessellationShadersTextRenderer : public VulkanTextRenderer {
public:
    /** Index into the array containing index buffer offsets of glyph's triangles */
    static constexpr unsigned int LINE_OFFSET_BUFFER_INDEX = 0;
    /** Index into the array containing index buffer offsets of glyph's curve segments */
    static constexpr unsigned int CURVE_OFFSET_BUFFER_INDEX = 1;

    /**
     * @brief Push constants for the viewport size
     */
    struct ViewportPushConstants {
        uint32_t viewportWidth;  /**< Viewport width */
        uint32_t viewportHeight; /**< Viewport height */
    };

protected:
    /** Hash map containing glyph offsets into the index buffers (key: glyph key, value: array of offsets into the index
     * buffer)
     */
    std::unordered_map<GlyphKey, std::array<uint32_t, 2>, GlyphKeyHash> _offsets{};

    std::vector<glm::vec2> _vertices{};            /**< Vertex buffer */
    std::vector<uint32_t> _lineSegmentsIndices{};  /**< Index buffer */
    std::vector<uint32_t> _curveSegmentsIndices{}; /**< Index buffer */

    VkBuffer _vertexBuffer{nullptr};             /**< Vulkan vertex buffer */
    VkDeviceMemory _vertexBufferMemory{nullptr}; /**< Vulkan vertex buffer memory */
    VkBuffer _lineSegmentsIndexBuffer{nullptr};  /**< Vulkan index buffer for line segments forming inner triangles */
    VkDeviceMemory _lineSegmentsIndexBufferMemory{
        nullptr}; /**< Vulkan index buffer memory for line segments forming inner triangles */
    VkBuffer _curveSegmentsIndexBuffer{nullptr};             /**< Vulkan index buffer for curve segments */
    VkDeviceMemory _curveSegmentsIndexBufferMemory{nullptr}; /**< Vulkan index buffer memory for curve segments */

    VkPipelineLayout _lineSegmentsPipelineLayout{nullptr};  /**< Vulkan pipeline layout for glpyh's triangles */
    VkPipeline _lineSegmentsPipeline{nullptr};              /**< Vulkan pipeline for glpyh's triangles */
    VkPipelineLayout _curveSegmentsPipelineLayout{nullptr}; /**< Vulkan pipeline layout for glpyh's curve segments */
    VkPipeline _curveSegmentsPipeline{nullptr};             /**< Vulkan pipeline for glpyh's curve segments */

public:
    VulkanTessellationShadersTextRenderer(VkPhysicalDevice physicalDevice,
                                          VkDevice logicalDevice,
                                          VkQueue graphicsQueue,
                                          VkCommandPool commandPool,
                                          VkRenderPass renderPass,
                                          VkCommandBuffer commandBuffer = nullptr);
    virtual ~VulkanTessellationShadersTextRenderer();

    void draw() override;
    void update() override;

protected:
    void _createVertexAndIndexBuffers();

    void _createLineSegmentsPipeline();
    void _createCurveSegmentsPipeline();
};

}  // namespace vft
