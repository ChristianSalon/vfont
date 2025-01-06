/**
 * @file gpu_drawer.h
 * @author Christian Saloň
 */

#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "drawer.h"
#include "glyph_cache.h"
#include "text_block.h"
#include "text_renderer_utils.h"

namespace vft {

class GpuDrawer : public Drawer {
public:
    static constexpr unsigned int GLYPH_MESH_BOUNDING_BOX_BUFFER_INDEX = 0;
    static constexpr unsigned int GLYPH_MESH_CURVE_BUFFER_INDEX = 1;
    static constexpr unsigned int GLYPH_MESH_LINE_BUFFER_INDEX = 2;

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
    std::unordered_map<GlyphKey, std::array<uint32_t, 2>, GlyphKeyHash> _offsets;

    std::vector<glm::vec2> _vertices;          /**< Vertex buffer */
    std::vector<uint32_t> _boundingBoxIndices; /**< Index buffer */
    std::vector<glm::vec2> _segments;
    std::vector<SegmentsInfo> _segmentsInfo;

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
    VkDescriptorSet _segmentsDescriptorSet;

    VkBuffer _ssbo;
    VkDeviceMemory _ssboMemory;
    void *_mappedSSBO;

public:
    GpuDrawer(GlyphCache &cache);
    ~GpuDrawer();

    void init(VulkanContext vulkanContext) override;
    void draw(std::vector<std::shared_ptr<TextBlock>> textBlocks, VkCommandBuffer commandBuffer) override;
    void recreateBuffers(std::vector<std::shared_ptr<TextBlock>> textBlocks) override;

    void setViewportSize(unsigned int width, unsigned int height);

protected:
    void _createVertexAndIndexBuffers(std::vector<std::shared_ptr<TextBlock>> &textBlocks);
    void _createSsbo();

    void _createDescriptorPool() override;
    void _createSegmentsDescriptorSetLayout();
    void _createSegmentsDescriptorSet();

    void _createSegmentsPipeline();
};

}  // namespace vft
