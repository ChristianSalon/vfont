/**
 * @file text_renderer.h
 * @author Christian Saloň
 */

#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

#include <vulkan/vulkan.h>

#include "combined_drawer.h"
#include "combined_tessellator.h"
#include "cpu_drawer.h"
#include "cpu_tessellator.h"
#include "drawer.h"
#include "glyph_cache.h"
#include "gpu_drawer.h"
#include "gpu_tessellator.h"
#include "tessellator.h"
#include "text_block.h"
#include "text_renderer_utils.h"

namespace vft {

/**
 * @class TextRenderer
 *
 * @brief Creates vertex and index buffers for specified characters
 */
class TextRenderer {
public:
    enum class TessellationStrategy { CPU_ONLY, CPU_AND_GPU, GPU_ONLY };

protected:
    TessellationStrategy _tessellationStrategy;

    std::vector<std::shared_ptr<TextBlock>> _blocks; /**< All text blocks to be rendered */
    std::shared_ptr<Tessellator> _tessellator;
    std::shared_ptr<Drawer> _drawer;
    GlyphCache _cache;

    VkPhysicalDevice _physicalDevice{nullptr}; /**< Vulkan physical device */
    VkDevice _logicalDevice{nullptr};          /**< Vulkan logical device */
    VkQueue _graphicsQueue{nullptr};           /**< Vulkan graphics queue */
    VkCommandPool _commandPool{nullptr};       /**< Vulkan command pool */
    VkRenderPass _renderPass{nullptr};

public:
    TextRenderer();
    ~TextRenderer();

    void init(TessellationStrategy tessellationStrategy,
              VkPhysicalDevice physicalDevice,
              VkDevice logicalDevice,
              VkCommandPool commandPool,
              VkQueue graphicsQueue,
              VkRenderPass renderPass);
    void destroy();
    void draw(VkCommandBuffer commandBuffer);
    void add(std::shared_ptr<TextBlock> text);

    void setUniformBuffers(vft::UniformBufferObject ubo);
    void setViewportSize(unsigned int width, unsigned int height);

protected:
    void _setPhysicalDevice(VkPhysicalDevice physicalDevice);
    void _setLogicalDevice(VkDevice logicalDevice);
    void _setCommandPool(VkCommandPool commandPool);
    void _setGraphicsQueue(VkQueue graphicsQueue);
    void _setRenderPass(VkRenderPass renderPass);
};

}  // namespace vft
