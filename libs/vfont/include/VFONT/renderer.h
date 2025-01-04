/**
 * @file renderer.h
 * @author Christian Saloň
 */

#pragma once

#include <memory>

#include <vulkan/vulkan.h>

#include "text_block.h"
#include "text_renderer_utils.h"

namespace vft {

/**
 * @class Renderer
 *
 * @brief Base class for all text renderers
 */
class Renderer {
public:
    enum class TessellationStrategy { CPU_ONLY, CPU_AND_GPU, GPU_ONLY };

    virtual ~Renderer() = default;

    virtual void init(TessellationStrategy tessellationStrategy, VulkanContext vulkanContext) = 0;
    virtual void destroy() = 0;
    virtual void add(std::shared_ptr<TextBlock> text) = 0;
    virtual void draw(VkCommandBuffer commandBuffer) = 0;

    virtual void setUniformBuffers(UniformBufferObject ubo) = 0;
    virtual void setViewportSize(unsigned int width, unsigned int height) = 0;

    virtual VulkanContext getVulkanContext() = 0;

    virtual void setCache(GlyphCache &cache) = 0;
    virtual void setCacheSize(unsigned long maxSize) = 0;
};

}  // namespace vft
