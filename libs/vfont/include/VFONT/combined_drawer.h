/**
 * @file combined_drawer.h
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

#include "drawer.h"
#include "glyph_cache.h"
#include "text_block.h"
#include "text_renderer_utils.h"

namespace vft {

class CombinedDrawer : public Drawer {
public:
    static constexpr unsigned int GLYPH_MESH_TRIANGLE_BUFFER_INDEX = 0;
    static constexpr unsigned int GLYPH_MESH_CURVE_BUFFER_INDEX = 1;

    static constexpr unsigned int LINE_OFFSET_BUFFER_INDEX = 0;
    static constexpr unsigned int CURVE_OFFSET_BUFFER_INDEX = 1;

    struct ViewportPushConstants {
        uint32_t viewportWidth;
        uint32_t viewportHeight;
    };

protected:
    std::unordered_map<GlyphKey, std::array<uint32_t, 2>, GlyphKeyHash> _offsets;

    std::vector<glm::vec2> _vertices;            /**< Vertex buffer */
    std::vector<uint32_t> _lineSegmentsIndices;  /**< Index buffer */
    std::vector<uint32_t> _curveSegmentsIndices; /**< Index buffer */

    unsigned int _viewportWidth;
    unsigned int _viewportHeight;

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
    CombinedDrawer(GlyphCache &cache);
    ~CombinedDrawer();

    void init(VkPhysicalDevice physicalDevice,
              VkDevice logicalDevice,
              VkCommandPool commandPool,
              VkQueue graphicsQueue,
              VkRenderPass renderPass) override;
    void draw(std::vector<std::shared_ptr<TextBlock>> textBlocks, VkCommandBuffer commandBuffer) override;
    void recreateBuffers(std::vector<std::shared_ptr<TextBlock>> textBlocks) override;
    
    void setViewportSize(unsigned int width, unsigned int height);

protected:
    void _createVertexAndIndexBuffers(std::vector<std::shared_ptr<TextBlock>> &textBlocks);

    void _createLineSegmentsPipeline();
    void _createCurveSegmentsPipeline();
};

}  // namespace vft
