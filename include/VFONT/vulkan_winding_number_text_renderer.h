/**
 * @file vulkan_winding_number_text_renderer.h
 * @author Christian Saloň
 */

#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

#include "glyph_cache.h"
#include "vulkan_text_renderer.h"
#include "winding_number_tessellator.h"

namespace vft {

/**
 * @brief Basic implementation of vulkan text renderer where glyphs are filled using the winding number algorithm in the
 * fragment shader
 */
class VulkanWindingNumberTextRenderer : public VulkanTextRenderer {
public:
    /** Index into the array containing index buffer offsets of glyph's bounding boxes */
    static constexpr unsigned int BOUNDING_BOX_OFFSET_BUFFER_INDEX = 0;
    /** Index into the array containing index buffer offsets of glyph's line and curve segments */
    static constexpr unsigned int SEGMENTS_INFO_OFFSET_BUFFER_INDEX = 1;

    /**
     * @brief Represents a glyph's information about line and curve segments stored in ssbo
     */
    struct SegmentsInfo {
        uint32_t lineSegmentsStartIndex;  /**< Index into the ssbo where glyph's line segments start */
        uint32_t lineSegmentsCount;       /**< Number of glyph's line segments */
        uint32_t curveSegmentsStartIndex; /**< Index into the ssbo where glyph's curve segments start */
        uint32_t curveSegmentsCount;      /**< Number of glyph's curve segments */
    };

    /**
     * @brief Represents a line segment
     */
    struct LineSegment {
        glm::vec2 start; /**< Start of line segment */
        glm::vec2 end;   /**< End of line segment */
    };

    /**
     * @brief Represents a quadratic bezier curve segment
     */
    struct CurveSegment {
        glm::vec2 start;   /**< Start of curve segment */
        glm::vec2 control; /**< Control point of curve segment */
        glm::vec2 end;     /**< End of curve segment */
    };

    /**
     * @brief Vulkan push constants
     */
    struct CharacterPushConstants {
        glm::mat4 model;                  /**< Model matrix of character */
        glm::vec4 color;                  /**< Color of character */
        uint32_t lineSegmentsStartIndex;  /**< Index into the ssbo where glyph's line segments start */
        uint32_t lineSegmentsCount;       /**< Number of glyph's line segments */
        uint32_t curveSegmentsStartIndex; /**< Index into the ssbo where glyph's curve segments start */
        uint32_t curveSegmentsCount;      /**< Number of glyph's curve segments */
    };

protected:
    /** Hash map containing glyph offsets into the index buffers (key: glyph key, value: array of offsets into the index
     * buffer)
     */
    std::unordered_map<GlyphKey, std::array<uint32_t, 2>, GlyphKeyHash> _offsets{};

    std::vector<glm::vec2> _vertices{};          /**< Vertex buffer */
    std::vector<uint32_t> _boundingBoxIndices{}; /**< Index buffer containing boundig box indices */
    std::vector<glm::vec2> _segments{};          /**< Contains line and curve segments of all glpyhs */
    std::vector<SegmentsInfo> _segmentsInfo{};   /**< Contains segment info of all glpyhs */

    VkBuffer _vertexBuffer{nullptr};                       /**< Vulkan vertex buffer */
    VkDeviceMemory _vertexBufferMemory{nullptr};           /**< Vulkan vertex buffer memory */
    VkBuffer _boundingBoxIndexBuffer{nullptr};             /**< Vulkan index buffer for bonding boxes */
    VkDeviceMemory _boundingBoxIndexBufferMemory{nullptr}; /**< Vulkan index buffer memory for bonding boxes  */
    VkBuffer _segmentsBuffer{nullptr};                     /**< Vulkan vertex buffer for line and curve segments */
    VkDeviceMemory _segmentsBufferMemory{nullptr}; /**< Vulkan vertex buffer memory for line and curve segments */

    VkPipelineLayout _segmentsPipelineLayout{
        nullptr};                          /**< Vulkan pipeline layout for glpyh's line and curve segments */
    VkPipeline _segmentsPipeline{nullptr}; /**< Vulkan pipeline for glpyh's line and curve segments */

    VkDescriptorSetLayout _segmentsDescriptorSetLayout{nullptr}; /**< Vulkan descriptor set layout for segments info */
    VkDescriptorSet _segmentsDescriptorSet{nullptr};             /**< Vulkan descriptor set for segments info */

    VkBuffer _ssbo{nullptr};             /**< Vulkan buffer containing line and curve segments of all glyphs */
    VkDeviceMemory _ssboMemory{nullptr}; /**< Vulkan memory containing line and curve segments of all glyphs */
    void *_mappedSSBO{nullptr};          /**< Pointer to the mapped memory for line and curve segments of all glyphs */

public:
    VulkanWindingNumberTextRenderer() = default;
    ~VulkanWindingNumberTextRenderer() = default;

    void initialize() override;
    void destroy() override;
    void draw() override;
    void update() override;

protected:
    void _createVertexAndIndexBuffers();
    void _createSsbo();

    void _createDescriptorPool() override;
    void _createSegmentsDescriptorSetLayout();
    void _createSegmentsDescriptorSet();

    void _createSegmentsPipeline();
};

}  // namespace vft
