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

#include "glyph_cache.h"
#include "vulkan_text_renderer.h"
#include "winding_number_tessellator.h"

namespace vft {

class VulkanWindingNumberTextRenderer : public VulkanTextRenderer {
public:
    static constexpr unsigned int BOUNDING_BOX_OFFSET_BUFFER_INDEX = 0;
    static constexpr unsigned int SEGMENTS_INFO_OFFSET_BUFFER_INDEX = 1;

    struct SegmentsInfo {
        uint32_t lineSegmentsStartIndex;
        uint32_t lineSegmentsCount;
        uint32_t curveSegmentsStartIndex;
        uint32_t curveSegmentsCount;
    };

    struct LineSegment {
        glm::vec2 start;
        glm::vec2 end;
    };

    struct CurveSegment {
        glm::vec2 start;
        glm::vec2 control;
        glm::vec2 end;
    };

    struct PushConstants {
        glm::mat4 model;
        glm::vec4 color;
        uint32_t lineSegmentsStartIndex;
        uint32_t lineSegmentsCount;
        uint32_t curveSegmentsStartIndex;
        uint32_t curveSegmentsCount;
    };

    struct ViewportPushConstants {
        uint32_t viewportWidth;
        uint32_t viewportHeight;
    };

protected:
    std::unordered_map<GlyphKey, std::array<uint32_t, 2>, GlyphKeyHash> _offsets{};

    std::vector<glm::vec2> _vertices{};          /**< Vertex buffer */
    std::vector<uint32_t> _boundingBoxIndices{}; /**< Index buffer */
    std::vector<glm::vec2> _segments{};
    std::vector<SegmentsInfo> _segmentsInfo{};

    unsigned int _viewportWidth{0};
    unsigned int _viewportHeight{0};

    VkBuffer _vertexBuffer{nullptr};                       /**< Vulkan vertex buffer */
    VkDeviceMemory _vertexBufferMemory{nullptr};           /**< Vulkan vertex buffer memory */
    VkBuffer _boundingBoxIndexBuffer{nullptr};             /**< Vulkan index buffer */
    VkDeviceMemory _boundingBoxIndexBufferMemory{nullptr}; /**< Vulkan index buffer memory */
    VkBuffer _segmentsBuffer{nullptr};                     /**< Vulkan vertex buffer */
    VkDeviceMemory _segmentsBufferMemory{nullptr};         /**< Vulkan vertex buffer memory */

    VkPipelineLayout _segmentsPipelineLayout{nullptr};
    VkPipeline _segmentsPipeline{nullptr};

    VkDescriptorSetLayout _segmentsDescriptorSetLayout{nullptr};
    VkDescriptorSet _segmentsDescriptorSet{nullptr};

    VkBuffer _ssbo{nullptr};
    VkDeviceMemory _ssboMemory{nullptr};
    void *_mappedSSBO{nullptr};

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
