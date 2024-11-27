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
    VkDevice _logicalDevice{nullptr};
    VkQueryPool _queryPool{nullptr};

public:
    TimedRenderer(Renderer *renderer, VkDevice logicalDevice);
    ~TimedRenderer() override;

    void draw(VkCommandBuffer commandBuffer) override;
    double readTimestamps(VkCommandBuffer commandBuffer);
};

}  // namespace vft
