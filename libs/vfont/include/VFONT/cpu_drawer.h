/**
 * @file cpu_drawer.h
 * @author Christian Saloň
 */

#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <cstdint>
#include <stdexcept>

#include <vulkan/vulkan.h>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include "drawer.h"
#include "glyph_cache.h"
#include "text_block.h"
#include "text_renderer_utils.h"

namespace vft {

class CpuDrawer : public Drawer {

protected:

    std::unordered_map<GlyphKey, uint32_t, GlyphKeyHash> _offsets;

    std::vector<glm::vec2> _vertices;                   /**< Vertex buffer */
    std::vector<uint32_t> _indices;         /**< Index buffer */

    VkBuffer _vertexBuffer{ nullptr };                             /**< Vulkan vertex buffer */
    VkDeviceMemory _vertexBufferMemory{ nullptr };                 /**< Vulkan vertex buffer memory */
    VkBuffer _indexBuffer{ nullptr };                  /**< Vulkan index buffer */
    VkDeviceMemory _indexBufferMemory{ nullptr };      /**< Vulkan index buffer memory */

    VkPipelineLayout _pipelineLayout{ nullptr };
    VkPipeline _pipeline{ nullptr };

public:

    CpuDrawer(GlyphCache& cache);
    ~CpuDrawer();

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

    void _createPipeline();

};

}
