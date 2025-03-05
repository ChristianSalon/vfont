/**
 * @file vulkan_timed_renderer.h
 * @author Christian Saloň
 */

#pragma once

#include <stdexcept>
#include <vector>

#include <vulkan/vulkan.h>

#include "vulkan_text_renderer.h"
#include "vulkan_text_renderer_decorator.h"

namespace vft {

class VulkanTimedRenderer : public VulkanTextRendererDecorator {
protected:
    double _timestampPeriod{1e-9};

    VkQueryPool _queryPool{nullptr};

public:
    VulkanTimedRenderer(VulkanTextRenderer *renderer);
    ~VulkanTimedRenderer() override;

    void initialize() override;
    void destroy() override;
    void draw() override;

    double readTimestamps();
    void resetQueryPool();

protected:
    double _getTimestampPeriod();
};

}  // namespace vft
