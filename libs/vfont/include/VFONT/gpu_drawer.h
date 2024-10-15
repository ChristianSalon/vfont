/**
 * @file gpu_drawer.h
 * @author Christian Saloň
 */

#pragma once

#include <vector>
#include <array>
#include <memory>
#include <unordered_map>
#include <cstdint>
#include <stdexcept>

#include <vulkan/vulkan.h>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

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
    static constexpr unsigned int CURVE_OFFSET_BUFFER_INDEX = 1;
    static constexpr unsigned int LINE_SEGMENTS_INFO_OFFSET_BUFFER_INDEX = 2;

    struct LineSegmentsInfo {
        uint32_t startIndex;
        uint32_t count;
    };

    struct LineSegment {
        glm::vec2 start;
        glm::vec2 end;
    };

    struct PushConstants {
        glm::mat4 model;
        glm::vec4 color;
        uint32_t lineSegmentsStartIndex;
        uint32_t lineSegmentsCount;
    };

protected:

    std::unordered_map<GlyphKey, std::array<uint32_t, 3>, GlyphKeyHash> _offsets;

    std::vector<glm::vec2> _vertices;                   /**< Vertex buffer */
    std::vector<uint32_t> _boundingBoxIndices;         /**< Index buffer */
    std::vector<uint32_t> _curveSegmentsIndices;        /**< Index buffer */
    std::vector<LineSegment> _lineSegments;
    std::vector<LineSegmentsInfo> _lineSegmentsInfo;

    VkBuffer _vertexBuffer{ nullptr };                             /**< Vulkan vertex buffer */
    VkDeviceMemory _vertexBufferMemory{ nullptr };                 /**< Vulkan vertex buffer memory */
    VkBuffer _boundingBoxIndexBuffer{ nullptr };                  /**< Vulkan index buffer */
    VkDeviceMemory _boundingBoxIndexBufferMemory{ nullptr };      /**< Vulkan index buffer memory */
    VkBuffer _curveSegmentsIndexBuffer{ nullptr };                 /**< Vulkan index buffer */
    VkDeviceMemory _curveSegmentsIndexBufferMemory{ nullptr };     /**< Vulkan index buffer memory */
    VkBuffer _lineSegmentsBuffer{ nullptr };                             /**< Vulkan vertex buffer */
    VkDeviceMemory _lineSegmentsBufferMemory{ nullptr };                 /**< Vulkan vertex buffer memory */

    VkPipelineLayout _lineSegmentsPipelineLayout{ nullptr };
    VkPipeline _lineSegmentsPipeline{ nullptr };
    VkPipelineLayout _curveSegmentsPipelineLayout{ nullptr };
    VkPipeline _curveSegmentsPipeline{ nullptr };

    VkDescriptorSetLayout _lineSegmentsDescriptorSetLayout{ nullptr };
    std::vector<VkDescriptorSet> _lineSegmentsDescriptorSets;

    std::vector<VkBuffer> _ssbo;
    std::vector<VkDeviceMemory> _ssboMemory;
    std::vector<void*> _mappedSSBO;

public:

    GpuDrawer(GlyphCache& cache);
    ~GpuDrawer();

    void init(
        VkPhysicalDevice physicalDevice,
        VkDevice logicalDevice,
        VkCommandPool commandPool,
        VkQueue graphicsQueue,
        VkRenderPass renderPass) override;
    void draw(std::vector<std::shared_ptr<TextBlock>> textBlocks, VkCommandBuffer commandBuffer) override;
    void recreateBuffers(std::vector<std::shared_ptr<TextBlock>> textBlocks) override;

protected:

    void _createVertexAndIndexBuffers(std::vector<std::shared_ptr<TextBlock>>& textBlocks);
    void _createSsbo();

    void _createDescriptorPool() override;
    void _createLineSegmentsDescriptorSetLayout();
    void _createLineSegmentsDescriptorSets();

    void _createLineSegmentsPipeline();
    void _createCurveSegmentsPipeline();

};

}
