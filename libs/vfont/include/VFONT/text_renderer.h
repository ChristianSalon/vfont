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
#include "renderer.h"
#include "tessellator.h"
#include "text_block.h"
#include "text_renderer_utils.h"

namespace vft {

/**
 * @class TextRenderer
 *
 * @brief Creates vertex and index buffers for specified characters
 */
class TextRenderer : public Renderer {
protected:
    TessellationStrategy _tessellationStrategy;

    std::vector<std::shared_ptr<TextBlock>> _blocks; /**< All text blocks to be rendered */
    std::shared_ptr<Tessellator> _tessellator;
    std::shared_ptr<Drawer> _drawer;
    GlyphCache _cache;

    VulkanContext _vulkanContext;

public:
    TextRenderer();
    ~TextRenderer() override;

    void init(TessellationStrategy tessellationStrategy, VulkanContext vulkanContext) override;
    void destroy() override;
    void add(std::shared_ptr<TextBlock> text) override;
    void draw(VkCommandBuffer commandBuffer) override;

    void setUniformBuffers(vft::UniformBufferObject ubo) override;
    void setViewportSize(unsigned int width, unsigned int height) override;

    void setCache(GlyphCache &cache) override;
    void setCacheSize(unsigned long maxSize) override;

    VulkanContext getVulkanContext();

protected:
    void _setPhysicalDevice(VkPhysicalDevice physicalDevice);
    void _setLogicalDevice(VkDevice logicalDevice);
    void _setCommandPool(VkCommandPool commandPool);
    void _setGraphicsQueue(VkQueue graphicsQueue);
    void _setRenderPass(VkRenderPass renderPass);
};

}  // namespace vft
