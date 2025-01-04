/**
 * @file timed_renderer.h
 * @author Christian Saloň
 */

#pragma once

#include <stdexcept>
#include <vector>

#include <vulkan/vulkan.h>

#include "renderer.h"
#include "renderer_decorator.h"

namespace vft {

class TimedRenderer : public RendererDecorator {
protected:
    double _timestampPeriod{1e-9};

    VkQueryPool _queryPool{nullptr};

public:
    TimedRenderer(Renderer *renderer);
    ~TimedRenderer() override;

    void init(TessellationStrategy tessellationStrategy, VulkanContext vulkanContext) override;
    void destroy() override;
    void draw(VkCommandBuffer commandBuffer) override;

    double readTimestamps(VkCommandBuffer commandBuffer);
    void resetQueryPool(VkCommandBuffer commandBuffer);

protected:
    double _getTimestampPeriod();
};

}  // namespace vft
