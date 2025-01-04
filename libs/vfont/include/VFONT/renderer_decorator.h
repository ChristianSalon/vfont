/**
 * @file renderer_decorator.h
 * @author Christian Saloň
 */

#pragma once

#include <vulkan/vulkan.h>

#include "renderer.h"
#include "text_renderer_utils.h"

namespace vft {

/**
 * @class RendererDecorator
 *
 * @brief Base class for all text renderer decorators
 */
class RendererDecorator : public Renderer {
protected:
    Renderer *_renderer;

public:
    RendererDecorator(Renderer *renderer);
    virtual ~RendererDecorator();

    void init(TessellationStrategy tessellationStrategy, VulkanContext vulkanContext) override;
    void destroy() override;
    void add(std::shared_ptr<TextBlock> text) override;
    void draw(VkCommandBuffer commandBuffer) override;

    void setUniformBuffers(UniformBufferObject ubo) override;
    void setViewportSize(unsigned int width, unsigned int height) override;

    VulkanContext getVulkanContext() override;

    void setCache(GlyphCache &cache) override;
    void setCacheSize(unsigned long maxSize) override;
};

}  // namespace vft
