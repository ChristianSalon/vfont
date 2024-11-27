/**
 * @file renderer_decorator.h
 * @author Christian Saloň
 */

#pragma once

#include <vulkan/vulkan.h>

#include "renderer.h"

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
    virtual ~RendererDecorator() = default;

    void init(TessellationStrategy tessellationStrategy,
              VkPhysicalDevice physicalDevice,
              VkDevice logicalDevice,
              VkCommandPool commandPool,
              VkQueue graphicsQueue,
              VkRenderPass renderPass) override;
    void destroy() override;
    void add(std::shared_ptr<TextBlock> text) override;
    void draw(VkCommandBuffer commandBuffer) override;

    void setUniformBuffers(UniformBufferObject ubo) override;
    void setViewportSize(unsigned int width, unsigned int height) override;
};

}  // namespace vft
